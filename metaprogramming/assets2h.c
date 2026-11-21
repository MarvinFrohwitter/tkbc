#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>

#include <libgen.h>
#include <pthread.h>

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <unistd.h>

#define TKBC_UTILS_IMPLEMENTATION
#include "../src/global/tkbc-utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb/stb_image.h"

// Compact alpha-mask format shared with tkbc-asset-handler.c.
//
//   'T' 'M' 'S' 'K' | version | kind | width | height | x0 | y0 | x1 | y1 | RLE
//   (all multi-byte fields are u32 little endian)
//
// kind 1: one bit per pixel (alpha is only 0 or 255), bits packed LSB first.
// kind 2: one byte per pixel with the original alpha value.
// x0..y1 is the inclusive bounding box of all opaque pixels, precomputed here
// so the colorizer never has to scan 20 million pixels to find it at runtime.
// An empty mask has x1 < x0.
// The RLE stream is a sequence of (count as ULEB128, value byte) runs over the
// packed representation. The colorizer only ever tests alpha > 0, so a binary
// mask reproduces its behaviour exactly while using 1/32 of the RGBA memory.
#define TKBC_MASK_HEADER_SIZE 30
#define TKBC_MASK_KIND_BINARY 1
#define TKBC_MASK_KIND_ALPHA8 2

static void tkbc_mask_push_uleb128(Content *out, uint64_t value) {
    do {
        unsigned char byte = (unsigned char) (value & 0x7f);
        value >>= 7;
        if (value != 0) {
            byte |= 0x80;
        }
        tkbc_dap(out, byte);
    } while (value != 0);
}

static bool tkbc_build_panel_mask(const char *image_file_path, int width, int height, Content *out) {
    int w = 0;
    int h = 0;
    int comp = 0;
    unsigned char *pixels = stbi_load(image_file_path, &w, &h, &comp, 4);
    if (pixels == NULL || w != width || h != height) {
        stbi_image_free(pixels);
        return false;
    }

    const size_t pixel_count = (size_t) w * (size_t) h;
    bool binary = true;
    int x0 = w;
    int y0 = h;
    int x1 = -1;
    int y1 = -1;
    for (size_t i = 0; i < pixel_count; ++i) {
        unsigned char alpha = pixels[i * 4 + 3];
        if (alpha != 0 && alpha != 255) {
            binary = false;
        }
        if (alpha > 0) {
            int x = (int) (i % (size_t) w);
            int y = (int) (i / (size_t) w);
            if (x < x0) x0 = x;
            if (x > x1) x1 = x;
            if (y < y0) y0 = y;
            if (y > y1) y1 = y;
        }
    }

    tkbc_dap(out, 'T');
    tkbc_dap(out, 'M');
    tkbc_dap(out, 'S');
    tkbc_dap(out, 'K');
    tkbc_dap(out, 1);  // version
    tkbc_dap(out, binary ? TKBC_MASK_KIND_BINARY : TKBC_MASK_KIND_ALPHA8);
    uint32_t header_values[6] = {(uint32_t) w, (uint32_t) h, (uint32_t) x0, (uint32_t) y0, (uint32_t) x1,
                                 (uint32_t) y1};
    for (int i = 0; i < 6; ++i) {
        for (int byte = 0; byte < 4; ++byte) {
            tkbc_dap(out, (unsigned char) (header_values[i] >> (byte * 8)));
        }
    }

    size_t i = 0;
    while (i < pixel_count) {
        unsigned char value = binary ? (pixels[i * 4 + 3] != 0 ? 1 : 0) : pixels[i * 4 + 3];
        uint64_t run = 1;
        while (i + run < pixel_count && run < UINT64_MAX) {
            unsigned char next = binary ? (pixels[(i + run) * 4 + 3] != 0 ? 1 : 0) : pixels[(i + run) * 4 + 3];
            if (next != value) {
                break;
            }
            ++run;
        }
        tkbc_mask_push_uleb128(out, run);
        tkbc_dap(out, value);
        i += (size_t) run;
    }

    stbi_image_free(pixels);
    return true;
}

static void tkbc_append_mask_byte_array(Content *data, const char *name_lower, const Content *mask) {
    tkbc_dapf(data, "\nstatic const unsigned char asset_mask_%s[%zu] = {\n", name_lower, mask->count);
    for (size_t i = 0; i < mask->count; ++i) {
        if (i == 0) {
            tkbc_dapf(data, "    ");
        } else if (i % 15 == 0) {
            tkbc_dapf(data, "\n    ");
        }
        tkbc_dapf(data, "0x%02X,", (unsigned char) mask->elements[i]);
    }
    tkbc_dapf(data, "\n};\n");
}

bool transpile_png_to_h(const char *image_file_path, const char *name) {
    bool ok = true;
    Content data = {0};
    Content file_data = {0};
    Content mask = {0};
    fprintf(stderr, "INFO: .................................\n");
    fprintf(stderr, "INFO: Converting %s ...\n", image_file_path);

    // Only the dimensions are needed at build time. The actual pixel data is
    // kept in its compressed PNG form and decoded by raylib at runtime with
    // LoadImageFromMemory. That keeps the generated headers (and therefore
    // the binaries) small.
    int x, y, comp;
    if (!stbi_info(image_file_path, &x, &y, &comp)) {
        fprintf(stderr, "ERROR: could not read image information from file!\n");
        fprintf(stderr, "INFO: .................................\n");
        check_return(false);
    }

    if (tkbc_read_entire_file(image_file_path, &file_data) < 0) {
        fprintf(stderr, "ERROR: could not read image file!\n");
        fprintf(stderr, "INFO: .................................\n");
        check_return(false);
    }

    char *dot = (char *) strrchr(name, '.');
    if (dot == NULL) {
        fprintf(stderr, "ERROR: no valid file extension!\n");
        fprintf(stderr, "INFO: .................................\n");
        check_return(false);
    }
    *dot = '\0';

    char *name_upper = tkbc_strtoupper(strdup(name));
    char *name_lower = tkbc_strtolower(strdup(name));
    tkbc_dapf(&data, "#define IMAGE_%s_WIDTH %d\n", name_upper, x);
    tkbc_dapf(&data, "#define IMAGE_%s_HEIGHT %d\n", name_upper, y);
    // tkbc_dapf(&data, "#define IMAGE_%s_FORMAT %s\n", name_upper, STR(PIXELFORMAT_UNCOMPRESSED_R8G8B8A8));
    tkbc_dapf(&data, "#define IMAGE_%s_FILE_TYPE \".png\"\n", name_upper);

    tkbc_dapf(&data, "\nstatic const unsigned char asset_image_%s[%zu] = {\n", name_lower,
              (size_t) file_data.count);

    int line_comp = 15;
    for (size_t i = 0; i < (size_t) file_data.count; ++i) {
        if (i == 0) {
            tkbc_dapf(&data, "    ");
        } else if (i % line_comp == 0) {
            tkbc_dapf(&data, "\n    ");
        }
        tkbc_dapf(&data, "0x%02X,", (unsigned char) file_data.elements[i]);
    }

    if (data.elements[data.count - 1] == ',') {
        data.count -= 1;
    } else if (data.elements[data.count - 1] == '\n') {
        data.count -= 2;
    }
    tkbc_dapf(&data, "\n};\n");

    // Emit a compact alpha mask as well. The colorizer only needs to know
    // which pixels of a panel are opaque; keeping full RGBA panels in memory
    // cost several gigabytes while the mask is only a few hundred kilobytes
    // compressed for all panels together.
    if (!tkbc_build_panel_mask(image_file_path, x, y, &mask)) {
        fprintf(stderr, "ERROR: could not build the alpha mask for %s!\n", image_file_path);
        free(name_upper);
        free(name_lower);
        *dot = '.';
        check_return(false);
    }
    tkbc_append_mask_byte_array(&data, name_lower, &mask);

    free(name_upper);
    free(name_lower);
    *dot = '.';

    dot = (char *) strrchr(image_file_path, '.');
    char save[3];
    save[0] = *(dot + 0);
    save[1] = *(dot + 1);
    save[2] = *(dot + 2);

    *(dot + 0) = '.';
    *(dot + 1) = 'h';
    *(dot + 2) = '\0';

    int res = tkbc_write_file(image_file_path, data.elements, data.count);
    if (res < 0) {
        fprintf(stderr, "ERROR: could not write image data to file!\n");
        fprintf(stderr, "INFO: .................................\n");
        *(dot + 0) = save[0];
        *(dot + 1) = save[1];
        *(dot + 2) = save[2];
        check_return(false);
    } else {
        fprintf(stderr, "Successfully converted: %s\n", image_file_path);
        fprintf(stderr, "INFO: .................................\n");
        *(dot + 0) = save[0];
        *(dot + 1) = save[1];
        *(dot + 2) = save[2];
    }

check:
    free(file_data.elements);
    free(mask.elements);
    free(data.elements);
    return ok;
}

// ===========================================================================
// Parallel asset conversion
// ===========================================================================

typedef struct {
    const Dir_Entry *entry;
    bool ok;
} Transpile_Task;

typedef struct {
    Transpile_Task *tasks;
    size_t task_count;
    atomic_size_t next_task;

    // Completion barrier: counts down for every finished task. The main thread
    // waits here until all file handling is done.
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    size_t pending_tasks;
} Transpile_Pool;

typedef struct {
    Transpile_Pool *pool;
} Transpile_Worker_Args;

static void transpile_pool_init(Transpile_Pool *pool, Transpile_Task *tasks, size_t task_count) {
    pool->tasks = tasks;
    pool->task_count = task_count;
    atomic_init(&pool->next_task, 0);
    pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->cond, NULL);
    pool->pending_tasks = task_count;
}

static void transpile_pool_destroy(Transpile_Pool *pool) {
    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->cond);
}

static void transpile_pool_wait(Transpile_Pool *pool) {
    pthread_mutex_lock(&pool->mutex);
    while (pool->pending_tasks > 0) {
        pthread_cond_wait(&pool->cond, &pool->mutex);
    }
    pthread_mutex_unlock(&pool->mutex);
}

static void *transpile_worker(void *arg) {
    Transpile_Pool *pool = ((Transpile_Worker_Args *) arg)->pool;
    for (;;) {
        size_t i = atomic_fetch_add(&pool->next_task, 1);
        if (i >= pool->task_count) {
            break;
        }

        Transpile_Task *task = &pool->tasks[i];
        task->ok = transpile_png_to_h(task->entry->full_path, task->entry->name);

        pthread_mutex_lock(&pool->mutex);
        pool->pending_tasks--;
        if (pool->pending_tasks == 0) {
            pthread_cond_signal(&pool->cond);
        }
        pthread_mutex_unlock(&pool->mutex);
    }
    return NULL;
}

static size_t transpile_thread_count(size_t task_count) {
    size_t thread_count = 0;
    long nproc = sysconf(_SC_NPROCESSORS_ONLN);
    if (nproc > 0) {
        thread_count = (size_t) nproc;
    }
    if (thread_count == 0) {
        thread_count = 4;
    }
    if (thread_count > task_count) {
        thread_count = task_count;
    }
    return thread_count;
}

#define BUILD_PATH "build/"

int get_file_type(const char *path) {
    struct stat statbuf;
    if (lstat(path, &statbuf) < 0) {
        fprintf(stderr, "Error: could not get stat for file %s: %s\n", path, strerror(errno));
        return -1;
    }
    return statbuf.st_mode;
}

int main(int argc, char **argv) {
    int ok = 0;
    const char *program_name = shift(argv, argc);
    if (argc == 0) {
        fprintf(stderr, "Error: Incorrect argument count given\n");
        fprintf(stderr, "Usage: %s <path> [path..]\n", program_name);
        return 1;
    }
    while (argc > 0) {
        char *path = shift(argv, argc);
        if (strcmp(path, "") == 0) {
            continue;
        }

        int file_type = get_file_type(path);
        if (file_type < 0) {
            continue;
        }
        Dir_Entries entries = {0};
        if (0) {
        } else if (S_ISREG(file_type)) {
            tkbc_dap(&entries, ((Dir_Entry){
                                   .name = strdup(basename(path)),
                                   .full_path = strdup(path),
                                   .type = DT_REG,
                               }));
        } else if (S_ISDIR(file_type)) {
            bool ok = read_dir_recursive(path, &entries);
            if (!ok) {
                free_dir_entrys(entries);
                continue;
            }
        } else {
            continue;
        }

        size_t png_count = 0;
        for (size_t i = 0; i < entries.count; ++i) {
            const char *name = entries.elements[i].name;
            if (strstr(name, ".png") && *name != '.') {
                png_count++;
            }
        }

        if (png_count > 0) {
            Transpile_Task *tasks = calloc(png_count, sizeof(*tasks));
            if (tasks == NULL) {
                fprintf(stderr, "ERROR: could not allocate the asset tasks!\n");
                free_dir_entrys(entries);
                continue;
            }

            size_t task_count = 0;
            for (size_t i = 0; i < entries.count; ++i) {
                const char *name = entries.elements[i].name;
                if (strstr(name, ".png") && *name != '.') {
                    tasks[task_count].entry = &entries.elements[i];
                    tasks[task_count].ok = false;
                    task_count++;
                }
            }

            Transpile_Pool pool = {0};
            transpile_pool_init(&pool, tasks, task_count);

            size_t thread_count = transpile_thread_count(task_count);
            pthread_t *threads = calloc(thread_count, sizeof(*threads));
            Transpile_Worker_Args *worker_args = calloc(thread_count, sizeof(*worker_args));
            size_t threads_started = 0;
            if (threads != NULL && worker_args != NULL) {
                for (size_t i = 0; i < thread_count; ++i) {
                    worker_args[i].pool = &pool;
                    if (pthread_create(&threads[i], NULL, transpile_worker, &worker_args[i]) != 0) {
                        fprintf(stderr, "WARNING: could only start %zu worker threads!\n", threads_started);
                        break;
                    }
                    threads_started++;
                }
            }

            // The main thread helps with the remaining tasks (and does all of
            // the work if no worker thread could be started).
            Transpile_Worker_Args main_args = {.pool = &pool};
            transpile_worker(&main_args);

            // Barrier: every worker has to finish its file handling before the
            // worker threads are joined.
            transpile_pool_wait(&pool);

            for (size_t i = 0; i < threads_started; ++i) {
                pthread_join(threads[i], NULL);
            }

            free(worker_args);
            free(threads);
            transpile_pool_destroy(&pool);
            free(tasks);
        }

        free_dir_entrys(entries);
    }
    return ok;
}
