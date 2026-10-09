# Vendored FFmpeg 9.0.2 (static, minimal feature set)

Source: https://ffmpeg.org/releases/ffmpeg-9.0.2.tar.xz
Upstream license: LGPL version 2.1 or later (no GPL components enabled:
no libx264, no other --enable-gpl libraries).

Built from a minimal configuration so the archives stay small and carry no
external dependencies beyond the C library:

```
./configure --disable-shared --enable-static --enable-pic \
    --disable-programs --disable-doc \
    --disable-avdevice --disable-avfilter \
    --disable-network --disable-bzlib --disable-lzma --disable-zlib \
    --disable-iconv --disable-hwaccels \
    --disable-libdrm --disable-vaapi --disable-vdpau --disable-vulkan \
    --disable-d3d11va --disable-d3d12va --disable-videotoolbox \
    --disable-everything \
    --enable-muxer=mp4 \
    --enable-demuxer=wav,mp3,ogg,flac,mov,aac \
    --enable-decoder=pcm_s16le,pcm_s24le,pcm_s32le,pcm_f32le,pcm_f64le,pcm_u8,pcm_s16be,mp3,vorbis,opus,flac,aac \
    --enable-encoder=mpeg4,aac \
    --enable-parser=mpegaudio,vorbis,opus,flac,aac \
    --enable-protocol=file \
    --enable-bsf=aac_adtstoasc \
    --enable-swscale --enable-swresample
```

Linux: native gcc build. Windows: `--target-os=mingw32
--cross-prefix=x86_64-w64-mingw32- --arch=x86_64 --pkg-config=false`.

Result: MP4 output with the native MPEG-4 video encoder and AAC audio.
H.264 encoding needs an external encoder (e.g. libx264, GPL) and is
intentionally not included; the application falls back to MPEG-4
automatically.

Link order: -lavformat -lavcodec -lswscale -lswresample -lavutil -lm
(-latomic -pthread on Linux).
