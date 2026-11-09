#ifndef TKBC_SCRIPT_CONVERTER_H
#define TKBC_SCRIPT_CONVERTER_H

#include "../../external/orbit/orbit.h"
#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
#include <stdio.h>

int tkbc_export_script_to_dot_kite_file_from_mem(Script *script, const char *filepath);
int tkbc_export_all_scripts_to_dot_kite_file_from_mem(Env *env, const char *path);

#endif  // TKBC_SCRIPT_CONVERTER_H
