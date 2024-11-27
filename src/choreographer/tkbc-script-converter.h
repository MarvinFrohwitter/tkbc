#ifndef TKBC_SCRIPT_CONVERTER_H
#define TKBC_SCRIPT_CONVERTER_H

#include "../global/tkbc-types.h"
#include <stdio.h>

bool tkbc_print_kites(FILE *file, Kite_Ids ids);
bool tkbc_write_script_kite_from_mem(Block_Frame *block_frame,
                                     const char *filename);

#endif // TKBC_SCRIPT_CONVERTER_H
