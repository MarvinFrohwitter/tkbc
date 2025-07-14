#ifndef TKBC_SCRIPT_CONVERTER_H
#define TKBC_SCRIPT_CONVERTER_H

#include "../global/tkbc-types.h"
#include <stdio.h>

void tkbc_print_kites(FILE *file, Kite_Ids ids);
int tkbc_write_script_kite_from_mem(Script *script, const char *filename);

#endif // TKBC_SCRIPT_CONVERTER_H
