// Video capturing via the FFmpeg libraries (libavformat/libavcodec/
// libswscale/libswresample/libavutil) linked directly into this application.
// No separate ffmpeg process is spawned and no pipe is used, so the
// dependency can be linked statically.
//
// Output: MP4 with H.264 video (libx264 when available, otherwise any H.264
// or MPEG-4 encoder) in yuv420p at the window framerate, plus the selected
// sound file transcoded to AAC when env->sound_file_name is set.

#include "raylib.h"

#include <float.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/channel_layout.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>

struct Process {
    AVFormatContext *oc;
    AVStream *video_st;
    AVCodecContext *video_enc;
    struct SwsContext *sws;
    AVFrame *frame;
    int64_t frame_count;
    int width;   // source width (window)
    int height;  // source height (window)
    int enc_width;   // encoder width (even aligned)
    int enc_height;  // encoder height (even aligned)
    int fps;
    char *output_path;

    // Audio (optional, transcoded at tkbc_ffmpeg_end()).
    bool has_audio;
    char *audio_path;
    AVFormatContext *audio_ic;
    AVCodecContext *audio_dec;
    int audio_stream_idx;
    AVStream *audio_st;
    AVCodecContext *audio_enc;
    struct SwrContext *swr;
    int64_t audio_next_pts;
};

#include "../global/tkbc-utils.h"
#include "tkbc-ffmpeg.h"
#include "tkbc-keymaps.h"

#define TKBC_VIDEO_BIT_RATE (2500 * 1000)
#define TKBC_AUDIO_BIT_RATE (192 * 1000)

static void tkbc_ffmpeg_free_process(Process *p) {
    if (!p) return;
    if (p->frame) av_frame_free(&p->frame);
    if (p->sws) sws_freeContext(p->sws);
    if (p->video_enc) avcodec_free_context(&p->video_enc);
    if (p->swr) swr_free(&p->swr);
    if (p->audio_enc) avcodec_free_context(&p->audio_enc);
    if (p->audio_dec) avcodec_free_context(&p->audio_dec);
    if (p->audio_ic) avformat_close_input(&p->audio_ic);
    // NOTE: oc (incl. its streams) is freed by the caller via
    // avformat_free_context() after av_write_trailer()/avio_closep().
    free(p->output_path);
    free(p->audio_path);
    free(p);
}

static const AVCodec *tkbc_ffmpeg_find_video_encoder(void) {
    const AVCodec *c = avcodec_find_encoder_by_name("libx264");
    if (c) return c;
    c = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (c) return c;
    return avcodec_find_encoder(AV_CODEC_ID_MPEG4);
}

static bool tkbc_ffmpeg_write_packet(AVFormatContext *oc, AVCodecContext *enc, AVStream *st, AVPacket *pkt) {
    av_packet_rescale_ts(pkt, enc->time_base, st->time_base);
    pkt->stream_index = st->index;
    if (av_interleaved_write_frame(oc, pkt) < 0) {
        tkbc_fprintf(stderr, "ERROR", "Could not write frame to output.\n");
        return false;
    }
    return true;
}

static bool tkbc_ffmpeg_drain_video_encoder(Process *p) {
    if (avcodec_send_frame(p->video_enc, NULL) < 0) {
        tkbc_fprintf(stderr, "ERROR", "Could not flush video encoder.\n");
        return false;
    }
    AVPacket *pkt = av_packet_alloc();
    if (!pkt) return false;
    bool ok = true;
    for (;;) {
        int ret = avcodec_receive_packet(p->video_enc, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
        if (ret < 0) {
            tkbc_fprintf(stderr, "ERROR", "Error while flushing video encoder.\n");
            ok = false;
            break;
        }
        if (!tkbc_ffmpeg_write_packet(p->oc, p->video_enc, p->video_st, pkt)) {
            ok = false;
            av_packet_unref(pkt);
            break;
        }
        av_packet_unref(pkt);
    }
    av_packet_free(&pkt);
    return ok;
}

// ---------------------------------------------------------------------------
// Audio: the input file is opened and the decoder is prepared before the
// output context exists. The output audio stream/encoder is only created
// once decoding works, so a failure never leaves a half-added stream behind.
// The actual transcode happens in tkbc_ffmpeg_end() where the video duration
// is known and audio can be truncated to it).
// ---------------------------------------------------------------------------
static void tkbc_ffmpeg_close_audio_input(Process *p) {
    if (p->audio_dec) avcodec_free_context(&p->audio_dec);
    if (p->audio_ic) avformat_close_input(&p->audio_ic);
    p->audio_stream_idx = -1;
}

// Phase 1 (no output context needed): open the audio file and its decoder.
// Returns true when decoding can proceed.
static bool tkbc_ffmpeg_open_audio_input(Process *p, const char *audio_path) {
    p->audio_stream_idx = -1;
    if (avformat_open_input(&p->audio_ic, audio_path, NULL, NULL) < 0) {
        tkbc_fprintf(stderr, "WARNING", "Could not open audio file %s, recording without sound.\n", audio_path);
        return false;
    }
    if (avformat_find_stream_info(p->audio_ic, NULL) < 0) {
        tkbc_fprintf(stderr, "WARNING", "Could not find stream info for %s, recording without sound.\n", audio_path);
        avformat_close_input(&p->audio_ic);
        return false;
    }
    int idx = av_find_best_stream(p->audio_ic, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (idx < 0) {
        tkbc_fprintf(stderr, "WARNING", "No audio stream in %s, recording without sound.\n", audio_path);
        avformat_close_input(&p->audio_ic);
        return false;
    }
    AVStream *in_st = p->audio_ic->streams[idx];
    const AVCodec *dec = avcodec_find_decoder(in_st->codecpar->codec_id);
    if (!dec) {
        tkbc_fprintf(stderr, "WARNING", "No decoder for audio file %s, recording without sound.\n", audio_path);
        avformat_close_input(&p->audio_ic);
        return false;
    }
    p->audio_dec = avcodec_alloc_context3(dec);
    if (!p->audio_dec) {
        avformat_close_input(&p->audio_ic);
        return false;
    }
    if (avcodec_parameters_to_context(p->audio_dec, in_st->codecpar) < 0 ||
        avcodec_open2(p->audio_dec, dec, NULL) < 0) {
        tkbc_fprintf(stderr, "WARNING", "Could not open audio decoder, recording without sound.\n");
        tkbc_ffmpeg_close_audio_input(p);
        return false;
    }
    p->audio_stream_idx = idx;
    return true;
}

// Phase 2: create the AAC encoder plus resampler (no output stream yet, so
// a failure leaves nothing behind). The caller must guarantee that phase 1
// succeeded.
static bool tkbc_ffmpeg_setup_audio_encoder(Process *p) {
    const AVCodec *enc = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!enc) {
        tkbc_fprintf(stderr, "WARNING", "No AAC encoder available, recording without sound.\n");
        return false;
    }
    p->audio_enc = avcodec_alloc_context3(enc);
    if (!p->audio_enc) {
        return false;
    }
    p->audio_enc->sample_rate = p->audio_dec->sample_rate ? p->audio_dec->sample_rate : 44100;
    // Some files (e.g. mono WAV) carry an "unknown" channel layout that the
    // AAC encoder rejects, so normalize to a default layout with the same
    // channel count.
    int nb_channels = p->audio_dec->ch_layout.nb_channels;
    if (nb_channels <= 0) nb_channels = 2;
    av_channel_layout_default(&p->audio_enc->ch_layout, nb_channels);
    p->audio_enc->sample_fmt = AV_SAMPLE_FMT_FLTP;
    p->audio_enc->bit_rate = TKBC_AUDIO_BIT_RATE;
    p->audio_enc->time_base = (AVRational){1, p->audio_enc->sample_rate};
    if (p->oc->oformat->flags & AVFMT_GLOBALHEADER) {
        p->audio_enc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    if (avcodec_open2(p->audio_enc, enc, NULL) < 0) {
        tkbc_fprintf(stderr, "WARNING", "Could not open AAC encoder, recording without sound.\n");
        av_channel_layout_uninit(&p->audio_enc->ch_layout);
        avcodec_free_context(&p->audio_enc);
        return false;
    }

    if (swr_alloc_set_opts2(&p->swr, &p->audio_enc->ch_layout, p->audio_enc->sample_fmt,
                             p->audio_enc->sample_rate, &p->audio_dec->ch_layout,
                             p->audio_dec->sample_fmt, p->audio_dec->sample_rate, 0, NULL) < 0 ||
        swr_init(p->swr) < 0) {
        tkbc_fprintf(stderr, "WARNING", "Could not init audio resampler, recording without sound.\n");
        swr_free(&p->swr);
        avcodec_free_context(&p->audio_enc);
        return false;
    }
    p->audio_next_pts = 0;
    return true;
}

// Phase 3: expose the prepared encoder as an output stream. Only fails on
// OOM, in which case the whole recording is aborted by the caller.
static bool tkbc_ffmpeg_add_audio_stream(Process *p) {
    p->audio_st = avformat_new_stream(p->oc, NULL);
    if (!p->audio_st) {
        return false;
    }
    if (avcodec_parameters_from_context(p->audio_st->codecpar, p->audio_enc) < 0) {
        return false;
    }
    p->audio_st->time_base = p->audio_enc->time_base;
    p->audio_st->avg_frame_rate = (AVRational){0, 0};
    return true;
}

static void tkbc_ffmpeg_close_audio(Process *p) {
    if (p->swr) swr_free(&p->swr);
    if (p->audio_enc) avcodec_free_context(&p->audio_enc);
    tkbc_ffmpeg_close_audio_input(p);
}

static bool tkbc_ffmpeg_encode_audio_frame(Process *p, AVFrame *frame, double max_sec, bool *truncated) {
    if (frame) {
        double t = (double) frame->pts / (double) p->audio_enc->sample_rate;
        if (t >= max_sec) {
            if (truncated) *truncated = true;
            return true;  // drop: beyond video duration
        }
    }
    if (avcodec_send_frame(p->audio_enc, frame) < 0) {
        tkbc_fprintf(stderr, "ERROR", "Could not send audio frame to encoder.\n");
        return false;
    }
    AVPacket *pkt = av_packet_alloc();
    if (!pkt) return false;
    bool ok = true;
    for (;;) {
        int ret = avcodec_receive_packet(p->audio_enc, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
        if (ret < 0) {
            tkbc_fprintf(stderr, "ERROR", "Error while encoding audio.\n");
            ok = false;
            break;
        }
        if (!tkbc_ffmpeg_write_packet(p->oc, p->audio_enc, p->audio_st, pkt)) {
            ok = false;
            av_packet_unref(pkt);
            break;
        }
        av_packet_unref(pkt);
    }
    av_packet_free(&pkt);
    return ok;
}

// Transcode the whole audio file, truncated to the video duration, using a
// fifo because decoder and encoder frame sizes may differ.
static bool tkbc_ffmpeg_transcode_audio(Process *p, double max_sec) {
    if (max_sec <= 0) return true;
    AVAudioFifo *fifo = av_audio_fifo_alloc(p->audio_enc->sample_fmt,
                                            p->audio_enc->ch_layout.nb_channels, 1024);
    if (!fifo) return false;
    AVPacket *pkt = av_packet_alloc();
    AVFrame *dframe = av_frame_alloc();
    AVFrame *conv = av_frame_alloc();
    if (!pkt || !dframe || !conv) {
        av_packet_free(&pkt);
        av_frame_free(&dframe);
        av_frame_free(&conv);
        av_audio_fifo_free(fifo);
        return false;
    }
    // Sample count and layout are set per conversion below.
    bool ok = true;
    bool truncated = false;

    int enc_frame_size = p->audio_enc->frame_size ? p->audio_enc->frame_size : 1024;
    AVFrame *eframe = av_frame_alloc();
    if (!eframe) ok = false;
    if (ok) {
        eframe->format = p->audio_enc->sample_fmt;
        eframe->sample_rate = p->audio_enc->sample_rate;
        if (av_channel_layout_copy(&eframe->ch_layout, &p->audio_enc->ch_layout) < 0) ok = false;
        eframe->nb_samples = enc_frame_size;
        if (ok && av_frame_get_buffer(eframe, 0) < 0) ok = false;
    }

    while (ok && !truncated && av_read_frame(p->audio_ic, pkt) >= 0) {
        if (pkt->stream_index != p->audio_stream_idx) {
            av_packet_unref(pkt);
            continue;
        }
        if (avcodec_send_packet(p->audio_dec, pkt) < 0) {
            av_packet_unref(pkt);
            ok = false;
            break;
        }
        av_packet_unref(pkt);
        for (;;) {
            if (avcodec_receive_frame(p->audio_dec, dframe) != 0) break;
            // Resample decoded frame into fifo.
            int dst_nb = swr_get_out_samples(p->swr, dframe->nb_samples);
            conv->format = p->audio_enc->sample_fmt;
            av_channel_layout_uninit(&conv->ch_layout);
            if (av_channel_layout_copy(&conv->ch_layout, &p->audio_enc->ch_layout) < 0) {
                ok = false;
                break;
            }
            conv->sample_rate = p->audio_enc->sample_rate;
            conv->nb_samples = dst_nb;
            if (av_frame_get_buffer(conv, 0) < 0) {
                ok = false;
                break;
            }
            int converted = swr_convert(p->swr, conv->data, dst_nb,
                                        (const uint8_t **) dframe->data, dframe->nb_samples);
            av_frame_unref(dframe);
            if (converted < 0) {
                av_frame_unref(conv);
                ok = false;
                break;
            }
            conv->nb_samples = converted;
            if (av_audio_fifo_write(fifo, (void **) conv->data, converted) < converted) {
                av_frame_unref(conv);
                ok = false;
                break;
            }
            av_frame_unref(conv);
            while (ok && !truncated && av_audio_fifo_size(fifo) >= enc_frame_size) {
                if (av_frame_make_writable(eframe) < 0) {
                    ok = false;
                    break;
                }
                if (av_audio_fifo_read(fifo, (void **) eframe->data, enc_frame_size) < enc_frame_size) {
                    ok = false;
                    break;
                }
                eframe->pts = p->audio_next_pts;
                p->audio_next_pts += enc_frame_size;
                if (!tkbc_ffmpeg_encode_audio_frame(p, eframe, max_sec, &truncated)) ok = false;
            }
            if (!ok || truncated) break;
        }
    }
    // Flush decoder.
    if (ok && !truncated) {
        avcodec_send_packet(p->audio_dec, NULL);
        while (avcodec_receive_frame(p->audio_dec, dframe) == 0) {
            int dst_nb = swr_get_out_samples(p->swr, dframe->nb_samples);
            conv->format = p->audio_enc->sample_fmt;
            av_channel_layout_uninit(&conv->ch_layout);
            if (av_channel_layout_copy(&conv->ch_layout, &p->audio_enc->ch_layout) < 0) {
                ok = false;
                break;
            }
            conv->sample_rate = p->audio_enc->sample_rate;
            conv->nb_samples = dst_nb;
            if (av_frame_get_buffer(conv, 0) < 0) {
                ok = false;
                break;
            }
            int converted = swr_convert(p->swr, conv->data, dst_nb,
                                        (const uint8_t **) dframe->data, dframe->nb_samples);
            av_frame_unref(dframe);
            if (converted < 0) {
                av_frame_unref(conv);
                ok = false;
                break;
            }
            conv->nb_samples = converted;
            if (av_audio_fifo_write(fifo, (void **) conv->data, converted) < converted) {
                av_frame_unref(conv);
                ok = false;
                break;
            }
            av_frame_unref(conv);
            while (ok && !truncated && av_audio_fifo_size(fifo) >= enc_frame_size) {
                if (av_frame_make_writable(eframe) < 0) {
                    ok = false;
                    break;
                }
                if (av_audio_fifo_read(fifo, (void **) eframe->data, enc_frame_size) < enc_frame_size) {
                    ok = false;
                    break;
                }
                eframe->pts = p->audio_next_pts;
                p->audio_next_pts += enc_frame_size;
                if (!tkbc_ffmpeg_encode_audio_frame(p, eframe, max_sec, &truncated)) ok = false;
            }
            if (!ok || truncated) break;
        }
        // Drain fifo remainder as final (possibly partial) frame.
        while (ok && !truncated && av_audio_fifo_size(fifo) > 0) {
            int n = av_audio_fifo_size(fifo);
            if (n > enc_frame_size) n = enc_frame_size;
            if (av_frame_make_writable(eframe) < 0) {
                ok = false;
                break;
            }
            // Pad remainder of the encoder frame with silence.
            int bytes_per_sample = av_get_bytes_per_sample(p->audio_enc->sample_fmt);
            int ch = p->audio_enc->ch_layout.nb_channels;
            if (av_audio_fifo_read(fifo, (void **) eframe->data, n) < n) {
                ok = false;
                break;
            }
            if (n < enc_frame_size && bytes_per_sample > 0) {
                for (int c = 0; c < ch; ++c) {
                    memset((uint8_t *) eframe->data[c] + n * bytes_per_sample, 0,
                           (size_t)(enc_frame_size - n) * (size_t) bytes_per_sample);
                }
            }
            eframe->nb_samples = enc_frame_size;
            eframe->pts = p->audio_next_pts;
            p->audio_next_pts += enc_frame_size;
            if (!tkbc_ffmpeg_encode_audio_frame(p, eframe, max_sec, &truncated)) ok = false;
        }
    }
    if (truncated) {
        // Drop anything that was read past the video duration, then flush
        // the encoder so already-fed frames (including its delayed output)
        // are still written.
        av_audio_fifo_drain(fifo, av_audio_fifo_size(fifo));
        tkbc_fprintf(stderr, "INFO", "Audio truncated to video duration.\n");
    }
    if (ok) {
        bool dummy = false;
        if (!tkbc_ffmpeg_encode_audio_frame(p, NULL, DBL_MAX, &dummy)) ok = false;
    }

    av_frame_free(&eframe);
    av_packet_free(&pkt);
    av_frame_free(&dframe);
    av_frame_free(&conv);
    av_audio_fifo_free(fifo);
    return ok;
}

/**
 * @brief The function captures the current screen into an image and exports
 * it to the given file path.
 *
 * @param path The file path where the screenshot should be saved.
 */
void tkbc_take_screenshot(const char *path) {
    if (IsWindowMinimized() || IsWindowHidden()) {
        tkbc_fprintf(stderr, "WARNING", "SYSTEM: Cannot take screenshot while window is minimized or hidden");
        return;
    }
    Image image = LoadImageFromScreen();
    ExportImage(image, path);  // WARNING: Module required: rtextures
    UnloadImage(image);

    if (FileExists(path))
        tkbc_fprintf(stderr, "WARNING", "SYSTEM: [%s] Screenshot taken successfully\n", path);
    else
        tkbc_fprintf(stderr, "WARNING", "SYSTEM: [%s] Screenshot could not be saved\n", path);
}

/**
 * @brief The function controls the keyboard input of the start and stop video
 * capturing.
 *
 * @param env The global state of the application.
 */
void tkbc_ffmpeg_handler(Env *env) {
    // KEY_B
    if (tkbc_check_keymaps_full(env->keymaps, KMH_TAKE_SCREENSHOT, KEY_MAP_CHECK_KEY_PRESSED)) {

        tkbc_make_dir_recursive_if_not_existis(env->tkbc_dir);
        const char *prefix = space_tprintf("%s%s", env->tkbc_dir, "Choreo Picture - ");
        if (!prefix) {
            goto err_screenshot;
        }

        char *file_name = tkbc_generate_name_with_time_stamp(prefix, ".png");
        space_reset_tspace();
        if (file_name == NULL) {
        err_screenshot:
            tkbc_fprintf(stderr, "ERROR", "No file name for screenshot can be allocated. Screenshot abort.\n");
        } else {

            tkbc_fprintf(stderr, "INFO", "File: %s\n", file_name);
            tkbc_take_screenshot(file_name);
            free(file_name);
        }
    }

    //
    // The handler has to be carefully checked because the same key is used
    // multiple times and that can cause problems, with reinitializing the
    // encoder where the old one is still running.
    //

    // KEY_V && KEY_LEFT_SHIFT && KEY_RIGHT_SHIFT
    if (tkbc_check_keymaps_full(env->keymaps, KMH_END_RECORDING, KEY_MAP_CHECK_KEY_PRESSED_MOD_DOWN)) {
        tkbc_ffmpeg_end(env, false);

    } else if (tkbc_check_keymaps_full(env->keymaps, KMH_BEGIN_RECORDING, KEY_MAP_CHECK_KEY_PRESSED)) {
        if (!env->rendering) {

            tkbc_make_dir_recursive_if_not_existis(env->tkbc_dir);
            const char *prefix = space_tprintf("%s%s", env->tkbc_dir, "Choreo Video - ");
            if (!prefix) {
                goto err_video;
            }
            char *output_file_path = tkbc_generate_name_with_time_stamp(prefix, ".mp4");
            space_reset_tspace();

            tkbc_fprintf(stderr, "INFO", "File: %s\n", output_file_path);
            if (output_file_path == NULL) {
            err_video:
                tkbc_fprintf(stderr, "ERROR", "No file name for screencast can be allocated. Screencast abort.\n");
                return;
            }

            if (!tkbc_ffmpeg_create_proc(env, output_file_path)) {
                env->recording = false;
                env->rendering = false;
            }
            free(output_file_path);
            // This ensures that the frame buffer can be setup and remaining
            // changes can take effect. For example disabling the fps display.
            // Otherwise the fps display would be visible in the final video for
            // just one frame. As a result tkbc_ffmpeg_write_image function is
            // delayed one frame.
            return;
        }
    }

    if (env->rendering) {
        int err = tkbc_ffmpeg_write_image(env);
        if (err == -1) {
            tkbc_ffmpeg_end(env, true);
        }
    }
}

/**
 * @brief Create the in-process video encoder for capturing the current
 * display. With env->sound_file_name set, that audio file is transcoded to
 * AAC and muxed (truncated to the video duration) when the recording ends.
 *
 * @param env The global state of the application.
 * @param output_file_path The file path of the output video.
 * @return True on success, otherwise false.
 */
bool tkbc_ffmpeg_create_proc(Env *env, const char *output_file_path) {
    if (!env || !output_file_path) return false;
    if (env->rendering && env->ffmpeg) {
        tkbc_fprintf(stderr, "WARNING", "Recording already in progress, ignoring new request.\n");
        return false;
    }
    int width = (int) env->window_width;
    int height = (int) env->window_height;
    int fps = env->fps > 0 ? env->fps : 60;
    if (width <= 0 || height <= 0) {
        tkbc_fprintf(stderr, "ERROR", "Invalid window size %dx%d for recording.\n", width, height);
        return false;
    }

    Process *p = calloc(1, sizeof(*p));
    if (!p) {
        tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
        return false;
    }
    p->audio_stream_idx = -1;
    p->width = width;
    p->height = height;
    // yuv420p needs even dimensions; scale down by one pixel if odd.
    p->enc_width = width & ~1;
    p->enc_height = height & ~1;
    if (p->enc_width <= 0) p->enc_width = 2;
    if (p->enc_height <= 0) p->enc_height = 2;
    p->fps = fps;
    p->output_path = strdup(output_file_path);
    if (!p->output_path) {
        free(p);
        return false;
    }

    if (avformat_alloc_output_context2(&p->oc, NULL, "mp4", output_file_path) < 0 || !p->oc) {
        tkbc_fprintf(stderr, "ERROR", "Could not allocate mp4 output context.\n");
        tkbc_ffmpeg_free_process(p);
        return false;
    }

    const AVCodec *vcodec = tkbc_ffmpeg_find_video_encoder();
    if (!vcodec) {
        tkbc_fprintf(stderr, "ERROR", "No H.264/MPEG-4 video encoder available in the linked FFmpeg libraries.\n");
        avformat_free_context(p->oc);
        p->oc = NULL;
        tkbc_ffmpeg_free_process(p);
        return false;
    }

    p->video_st = avformat_new_stream(p->oc, NULL);
    if (!p->video_st) {
        tkbc_fprintf(stderr, "ERROR", "Could not allocate video stream.\n");
        avformat_free_context(p->oc);
        p->oc = NULL;
        tkbc_ffmpeg_free_process(p);
        return false;
    }
    p->video_enc = avcodec_alloc_context3(vcodec);
    if (!p->video_enc) {
        tkbc_fprintf(stderr, "ERROR", "Could not allocate video encoder context.\n");
        avformat_free_context(p->oc);
        p->oc = NULL;
        tkbc_ffmpeg_free_process(p);
        return false;
    }
    p->video_enc->width = p->enc_width;
    p->video_enc->height = p->enc_height;
    p->video_enc->pix_fmt = AV_PIX_FMT_YUV420P;
    p->video_enc->time_base = (AVRational){1, fps};
    p->video_enc->framerate = (AVRational){fps, 1};
    p->video_enc->bit_rate = TKBC_VIDEO_BIT_RATE;
    p->video_enc->gop_size = 12;
    p->video_enc->max_b_frames = 2;
    if (p->oc->oformat->flags & AVFMT_GLOBALHEADER) {
        p->video_enc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    if (vcodec->id == AV_CODEC_ID_H264) {
        // Fast, widely compatible settings (equivalent to the old
        // `-c:v libx264` invocation).
        av_opt_set(p->video_enc->priv_data, "preset", "medium", 0);
    }
    if (avcodec_open2(p->video_enc, vcodec, NULL) < 0) {
        tkbc_fprintf(stderr, "ERROR", "Could not open video encoder %s.\n", vcodec->name);
        avformat_free_context(p->oc);
        p->oc = NULL;
        tkbc_ffmpeg_free_process(p);
        return false;
    }
    if (avcodec_parameters_from_context(p->video_st->codecpar, p->video_enc) < 0) {
        tkbc_fprintf(stderr, "ERROR", "Could not copy video params.\n");
        avformat_free_context(p->oc);
        p->oc = NULL;
        tkbc_ffmpeg_free_process(p);
        return false;
    }
    p->video_st->time_base = p->video_enc->time_base;
    p->video_st->avg_frame_rate = (AVRational){fps, 1};
    p->video_st->r_frame_rate = (AVRational){fps, 1};

    if (env->sound_file_name) {
        p->audio_path = strdup(env->sound_file_name);
        if (p->audio_path && tkbc_ffmpeg_open_audio_input(p, p->audio_path) &&
            tkbc_ffmpeg_setup_audio_encoder(p) && tkbc_ffmpeg_add_audio_stream(p)) {
            p->has_audio = true;
            tkbc_fprintf(stderr, "INFO", "Muxing audio from %s\n", p->audio_path);
        } else {
            tkbc_ffmpeg_close_audio(p);
            free(p->audio_path);
            p->audio_path = NULL;
            p->has_audio = false;
        }
    }

    p->sws = sws_getContext(width, height, AV_PIX_FMT_RGBA, p->enc_width, p->enc_height,
                             AV_PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);
    if (!p->sws) {
        tkbc_fprintf(stderr, "ERROR", "Could not create color converter.\n");
        AVFormatContext *oc = p->oc;
        p->oc = NULL;
        avformat_free_context(oc);
        tkbc_ffmpeg_free_process(p);
        return false;
    }
    p->frame = av_frame_alloc();
    if (!p->frame) {
        AVFormatContext *oc = p->oc;
        p->oc = NULL;
        avformat_free_context(oc);
        tkbc_ffmpeg_free_process(p);
        return false;
    }
    p->frame->format = AV_PIX_FMT_YUV420P;
    p->frame->width = p->enc_width;
    p->frame->height = p->enc_height;
    if (av_frame_get_buffer(p->frame, 0) < 0) {
        tkbc_fprintf(stderr, "ERROR", "Could not allocate video frame.\n");
        AVFormatContext *oc = p->oc;
        p->oc = NULL;
        avformat_free_context(oc);
        tkbc_ffmpeg_free_process(p);
        return false;
    }

    if (!(p->oc->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&p->oc->pb, output_file_path, AVIO_FLAG_WRITE) < 0) {
            tkbc_fprintf(stderr, "ERROR", "Could not open output file %s.\n", output_file_path);
            AVFormatContext *oc = p->oc;
            p->oc = NULL;
            avformat_free_context(oc);
            tkbc_ffmpeg_free_process(p);
            return false;
        }
    }
    if (avformat_write_header(p->oc, NULL) < 0) {
        tkbc_fprintf(stderr, "ERROR", "Could not write header to %s.\n", output_file_path);
        if (!(p->oc->oformat->flags & AVFMT_NOFILE)) avio_closep(&p->oc->pb);
        AVFormatContext *oc = p->oc;
        p->oc = NULL;
        avformat_free_context(oc);
        tkbc_ffmpeg_free_process(p);
        return false;
    }

    tkbc_fprintf(stderr, "INFO", "Recording to %s (%dx%d@%d, %s%s)\n", output_file_path, p->enc_width,
                 p->enc_height, fps, vcodec->name, p->has_audio ? " + audio" : "");
    env->ffmpeg = p;
    env->recording = true;
    env->rendering = true;
    return true;
}

/**
 * @brief Finish the recording: flush the encoders, mux pending audio,
 * write the trailer and free the encoder.
 *
 * @param env The global state of the application.
 * @param is_kill_foreced Kept for compatibility; the file is always
 * finalized so it stays playable. True only adds a warning log.
 * @return True on success, otherwise false.
 */
bool tkbc_ffmpeg_end(Env *env, bool is_kill_foreced) {
    if (!env || !env->rendering || !env->ffmpeg) {
        return true;
    }
    if (is_kill_foreced) {
        tkbc_fprintf(stderr, "WARNING", "Recording aborted, finalizing partial file.\n");
    }
    Process *p = env->ffmpeg;
    bool ok = true;
    if (!tkbc_ffmpeg_drain_video_encoder(p)) ok = false;

    if (p->has_audio && p->fps > 0) {
        double max_sec = (double) p->frame_count / (double) p->fps;
        if (!tkbc_ffmpeg_transcode_audio(p, max_sec)) {
            tkbc_fprintf(stderr, "WARNING", "Audio muxing failed, video is kept.\n");
        }
    }

    if (av_write_trailer(p->oc) < 0) {
        tkbc_fprintf(stderr, "ERROR", "Could not write trailer.\n");
        ok = false;
    }
    if (!(p->oc->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&p->oc->pb);
    }
    AVFormatContext *oc = p->oc;
    p->oc = NULL;
    avformat_free_context(oc);
    tkbc_fprintf(stderr, "INFO", "Recording finished: %s (%lld frames)\n", p->output_path,
                 (long long) p->frame_count);
    tkbc_ffmpeg_free_process(p);
    env->ffmpeg = NULL;
    env->recording = false;
    env->rendering = false;
    return ok;
}

/**
 * @brief Encode one raw RGBA frame.
 *
 * @param env The global state holding the encoder.
 * @param rgba Packed 8-bit RGBA, top-down rows.
 * @param width Frame width in pixels.
 * @param height Frame height in pixels.
 * @return True on success, otherwise false.
 */
bool tkbc_ffmpeg_write_rgba(Env *env, const unsigned char *rgba, int width, int height) {
    if (!env || !env->rendering || !env->ffmpeg || !rgba) return false;
    Process *p = env->ffmpeg;
    if (width != p->width || height != p->height) {
        tkbc_fprintf(stderr, "ERROR", "Frame size %dx%d does not match recording size %dx%d.\n", width,
                     height, p->width, p->height);
        return false;
    }
    if (av_frame_make_writable(p->frame) < 0) {
        tkbc_fprintf(stderr, "ERROR", "Could not make video frame writable.\n");
        return false;
    }
    const uint8_t *src[4] = {rgba, NULL, NULL, NULL};
    int src_ls[4] = {width * 4, 0, 0, 0};
    sws_scale(p->sws, src, src_ls, 0, height, p->frame->data, p->frame->linesize);
    p->frame->pts = p->frame_count;

    if (avcodec_send_frame(p->video_enc, p->frame) < 0) {
        tkbc_fprintf(stderr, "ERROR", "Could not send video frame to encoder.\n");
        return false;
    }
    AVPacket *pkt = av_packet_alloc();
    if (!pkt) return false;
    bool ok = true;
    for (;;) {
        int ret = avcodec_receive_packet(p->video_enc, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
        if (ret < 0) {
            tkbc_fprintf(stderr, "ERROR", "Error while encoding video frame.\n");
            ok = false;
            break;
        }
        if (!tkbc_ffmpeg_write_packet(p->oc, p->video_enc, p->video_st, pkt)) {
            ok = false;
            av_packet_unref(pkt);
            break;
        }
        av_packet_unref(pkt);
    }
    av_packet_free(&pkt);
    if (ok) p->frame_count++;
    return ok;
}

/**
 * @brief Capture the current window content and encode it.
 *
 * @param env The global state of the application.
 * @return 1 if nothing was encoded, 0 on success, -1 on error.
 */
int tkbc_ffmpeg_write_image(Env *env) {
    if (!env || !env->rendering || !env->ffmpeg) {
        return 1;
    }
    if (IsWindowMinimized() || IsWindowHidden()) {
        return 1;
    }
    int ok = 0;
    Image image = LoadImageFromScreen();
    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    if (image.width != (int) env->ffmpeg->width || image.height != (int) env->ffmpeg->height) {
        tkbc_fprintf(stderr, "WARNING", "Window size changed during recording (%dx%d), stopping.\n", image.width,
                     image.height);
        UnloadImage(image);
        return -1;
    }
    if (!tkbc_ffmpeg_write_rgba(env, image.data, image.width, image.height)) {
        ok = -1;
    }
    UnloadImage(image);
    return ok;
}
