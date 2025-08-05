#ifndef TKBC_SCRIPT_CONVERTER_H
#define TKBC_SCRIPT_CONVERTER_H

#include "../global/tkbc-types.h"
#include <stdio.h>

void tkbc_print_kites(FILE *file, Kite_Ids ids);
int tkbc_export_script_to_dot_kite_file_from_mem(Script *script,
                                                 const char *filename);
int tkbc_export_all_scripts_to_dot_kite_file_from_mem(Env *env);

#endif // TKBC_SCRIPT_CONVERTER_H
