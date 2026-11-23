/**
 * @brief Supplies TCC with its runtime archive without a system tcc install.
 *
 * libtcc (the JIT compiler) is linked statically into the executables, but
 * libtcc1.a (the compiler runtime) is only ever opened by TCC as a file. The
 * archive is embedded at build time (see metaprogramming/libtcc1_2h.c), written
 * into a private temporary directory on first use and that directory is added
 * to the library paths of every new TCCState.
 */

#include "tkbc-libtcc-runtime.h"

#include "skystep-libtcc1-header.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#include <process.h>
#define TKBC_TCC_PATH_SEP '\\'
#define TKBC_TCC_MKDIR(path) _mkdir(path)
#define TKBC_TCC_RMDIR(path) _rmdir(path)
#else
#include <sys/stat.h>
#include <unistd.h>
#define TKBC_TCC_PATH_SEP '/'
#define TKBC_TCC_MKDIR(path) mkdir((path), 0700)
#define TKBC_TCC_RMDIR(path) rmdir(path)
#endif

#define TKBC_TCC_RUNTIME_NAME "libtcc1.a"
#define TKBC_TCC_PATH_MAX 512

static char runtime_dir[TKBC_TCC_PATH_MAX] = {0};
static char runtime_file[TKBC_TCC_PATH_MAX] = {0};
static bool runtime_ready = false;
static bool runtime_tried = false;

static void tkbc_tcc_runtime_cleanup(void) {
    if (runtime_file[0]) {
        remove(runtime_file);
    }
    if (runtime_dir[0]) {
        TKBC_TCC_RMDIR(runtime_dir);
    }
}

static const char *tkbc_tcc_temp_base(void) {
#ifdef _WIN32
    const char *base = getenv("TEMP");
    if (!base || !*base) {
        base = getenv("TMP");
    }
    return (base && *base) ? base : ".";
#else
    const char *base = getenv("TMPDIR");
    return (base && *base) ? base : "/tmp";
#endif
}

static bool tkbc_tcc_create_runtime(void) {
    const char *base = tkbc_tcc_temp_base();

    // The pid and the attempt counter keep concurrent processes from using
    // the same directory.
    for (int attempt = 0; attempt < 100; ++attempt) {
        int n = snprintf(runtime_dir, sizeof(runtime_dir), "%s%ctkbc-tcc-%ld-%d", base, TKBC_TCC_PATH_SEP,
                         (long)getpid(), attempt);
        if (n < 0 || (size_t)n >= sizeof(runtime_dir)) {
            fprintf(stderr, "[TCC] Temporary path for the embedded runtime is too long.\n");
            return false;
        }
        if (TKBC_TCC_MKDIR(runtime_dir) == 0) {
            break;
        }
        if (errno != EEXIST || attempt == 99) {
            fprintf(stderr, "[TCC] Could not create temporary directory %s: %s\n", runtime_dir, strerror(errno));
            return false;
        }
    }

    int n = snprintf(runtime_file, sizeof(runtime_file), "%s%c%s", runtime_dir, TKBC_TCC_PATH_SEP,
                     TKBC_TCC_RUNTIME_NAME);
    if (n < 0 || (size_t)n >= sizeof(runtime_file)) {
        fprintf(stderr, "[TCC] Temporary path for the embedded runtime is too long.\n");
        tkbc_tcc_runtime_cleanup();
        return false;
    }

    FILE *file = fopen(runtime_file, "wb");
    if (file == NULL) {
        fprintf(stderr, "[TCC] Could not create %s: %s\n", runtime_file, strerror(errno));
        tkbc_tcc_runtime_cleanup();
        return false;
    }

    size_t size = sizeof(tkbc_embedded_libtcc1);
    bool ok = fwrite(tkbc_embedded_libtcc1, 1, size, file) == size;
    if (fclose(file) != 0) {
        ok = false;
    }
    if (!ok) {
        fprintf(stderr, "[TCC] Could not write %s: %s\n", runtime_file, strerror(errno));
        tkbc_tcc_runtime_cleanup();
        return false;
    }

    atexit(tkbc_tcc_runtime_cleanup);
    return true;
}

void tkbc_tcc_add_embedded_runtime(TCCState *tcc) {
    if (!runtime_tried) {
        runtime_tried = true;
        runtime_ready = tkbc_tcc_create_runtime();
    }
    if (!runtime_ready || tcc == NULL) {
        return;
    }
    if (tcc_add_library_path(tcc, runtime_dir) == -1) {
        fprintf(stderr, "[TCC] Could not add the embedded runtime path %s.\n", runtime_dir);
    }
}
