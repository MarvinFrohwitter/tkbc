# clang bug with size_t, variadics, optimization and compiler caching
CC = gcc
RAYLIBPATH = external/raylib-5.5_linux_amd64
INCLUDE = -I src/choreographer/ -I src/global/ -I src/network/
INCLUDE += -I tkbc_scripts/ -I build/
INCLUDE += -I ${RAYLIBPATH}/include/
LIBS = -L ${RAYLIBPATH}/lib/
LIBS += -l:libraylib.a
LIBS += -lm
CFLAGS = -x c -O0 -fPIC -pedantic -Wall -Wextra -ggdb
CHOREOGRAPHERPATH = src/choreographer
CHOREOGRAPHER = ${CHOREOGRAPHERPATH}/main.c
CHOREOGRAPHER_FILES = ${shell find ${CHOREOGRAPHERPATH}/ ! -name "main.c" -name "*.c"}
CHOREOGRAPHER += ${CHOREOGRAPHER_FILES}

all: options build tkbc client tkbc-win64 poll-server poll-server-win64 client-win64

options:
	@echo tbkc build options:
	@echo "CFLAGS = ${CFLAGS}"
	@echo "LIBS   = ${LIBS}"
	@echo "CC     = ${CC}"

poll-server: build
	${CC} ${INCLUDE} ${CFLAGS} -o build/poll-server src/network/poll-server.c src/network/tkbc-network-common.c ${CHOREOGRAPHER_FILES} ${LIBS}
	cp -v build/poll-server build/server

poll-server-win64: build first.o
	x86_64-w64-mingw32-gcc -Wall -Wextra -O3 -static  -mwindows -I ./external/raylib-5.5_win64_mingw-w64/include/ -o build/poll-server-win64 src/network/poll-server.c src/network/tkbc-network-common.c ${CHOREOGRAPHER_FILES} -L ./external/raylib-5.5_win64_mingw-w64/lib/ -lraylib -lws2_32 -lwinmm

client: build first.o
	${CC} ${INCLUDE} ${CFLAGS} -o build/client src/network/tkbc-client.c src/network/tkbc-network-common.c src/global/tkbc-popup.c ${CHOREOGRAPHER_FILES} ${LIBS}


tkbc: build first.o
	${CC} ${INCLUDE} ${CFLAGS} -o build/tkbc ${CHOREOGRAPHER} ${LIBS}

first.o: build
	${CC} ${INCLUDE} ${CFLAGS} -c tkbc_scripts/first.c -o build/first.o


tkbc-win64: build
	x86_64-w64-mingw32-gcc -Wall -Wextra -O3 -static -mwindows -I ./external/raylib-5.5_win64_mingw-w64/include/ -o ./build/tkbc-win64 ./src/choreographer/*.c  -L ./external/raylib-5.5_win64_mingw-w64/lib/ -lraylib -lwinmm -lgdi32

client-win64: build first.o
	x86_64-w64-mingw32-gcc -Wall -Wextra -O3 -static -mwindows -I ./external/raylib-5.5_win64_mingw-w64/include/ -o build/client-win64 src/network/tkbc-client.c src/network/tkbc-network-common.c src/global/tkbc-popup.c ${CHOREOGRAPHER_FILES} -L ./external/raylib-5.5_win64_mingw-w64/lib/ -lraylib -lwinmm -lgdi32 -lws2_32

build:
	mkdir -p build

clean:
	rm -r build

test: options
	${CC} ${INCLUDE} ${CFLAGS} -o build/tests src/tests/tkbc_tests.c ${CHOREOGRAPHER_FILES} ${LIBS}
	./build/tests

test-verbose: options
	${CC} ${INCLUDE} ${CFLAGS} -DPRINT_OPERATION_AND_DESCRIPTION -o build/tests src/tests/tkbc_tests.c ${CHOREOGRAPHER_FILES} ${LIBS}
	./build/tests

test-short: options
	${CC} ${INCLUDE} ${CFLAGS} -DSHORT_LOG -o build/tests src/tests/tkbc_tests.c ${CHOREOGRAPHER_FILES} ${LIBS}
	./build/tests

.PHONY: all clean options tkbc tkbc.o build client test
