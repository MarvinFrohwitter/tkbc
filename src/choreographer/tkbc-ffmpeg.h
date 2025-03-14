#ifndef TKBC_FFMPEG_H_
#define TKBC_FFMPEG_H_

#include "../global/tkbc-types.h"

void tkbc_ffmpeg_handler(Env *env, const char *output_file_path);
bool tkbc_ffmpeg_create_proc(Env *env, const char *output_file_path);
bool tkbc_ffmpeg_end(Env *env, bool is_kill_foreced);
bool tkbc_ffmpeg_wait(Process process, bool is_kill_foreced);
int tkbc_ffmpeg_write_image(Env *env);

#endif // TKBC_FFMPEG_H_
