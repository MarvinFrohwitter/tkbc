#ifndef TKBC_LIBTCC_RUNTIME_H_
#define TKBC_LIBTCC_RUNTIME_H_

#include "libtcc.h"

/**
 * @brief Makes the libtcc1.a embedded in this executable available to TCC.
 *
 * TCC resolves libtcc1.a through the file system while relocating JIT code.
 * The runtime is extracted once into a private temporary directory and that
 * directory is added to the library search paths of @p tcc.
 *
 * @param tcc The TCC state that is about to compile/relocate a script.
 */
void tkbc_tcc_add_embedded_runtime(TCCState *tcc);

#endif // TKBC_LIBTCC_RUNTIME_H_
