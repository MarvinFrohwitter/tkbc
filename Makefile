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

all: options clean build tkbc server client

options:
	@echo tbkc build options:
	@echo "CFLAGS = ${CFLAGS}"
	@echo "LIBS   = ${LIBS}"
	@echo "CC     = ${CC}"


server: build first.o
	${CC} ${INCLUDE} ${CFLAGS} -static -o build/server src/network/tkbc-server.c src/network/tkbc-server-client-handler.c src/network/tkbc-network-common.c ${CHOREOGRAPHER_FILES} ${LIBS}

client: build first.o
	${CC} ${INCLUDE} ${CFLAGS} -o build/client src/network/tkbc-client.c src/network/tkbc-network-common.c src/global/tkbc-popup.c ${CHOREOGRAPHER_FILES} ${LIBS}


tkbc: build first.o
	${CC} ${INCLUDE} ${CFLAGS} -o build/tkbc ${CHOREOGRAPHER} ${LIBS}

first.o: build
	${CC} ${INCLUDE} ${CFLAGS} -c tkbc_scripts/first.c -o build/first.o

build:
	mkdir -p build

clean:
	rm -r build

test: options
	${CC} ${INCLUDE} ${CFLAGS} -o build/test_geometrics src/tests/tkbc_test_geometrics.c ${CHOREOGRAPHER_FILES} ${LIBS}
	./build/test_geometrics

test-short: options
	${CC} ${INCLUDE} ${CFLAGS} -DSHORT_LOG -o build/test_geometrics src/tests/tkbc_test_geometrics.c ${CHOREOGRAPHER_FILES} ${LIBS}
	./build/test_geometrics

.PHONY: all clean options tkbc tkbc.o build server client test
