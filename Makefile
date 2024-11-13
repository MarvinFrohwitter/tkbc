# clang bug with size_t, variadics, optimization and compiler caching
CC = gcc
RAYLIBPATH = external/raylib-5.0/src
INCLUDE = -I src/choreographer/ -I src/global/ -I src/network/
INCLUDE += -I ${RAYLIBPATH} -I tkbc_scripts/ -I build/
LIBS = -L ${RAYLIBPATH}
LIBS += -l:libraylib.a
LIBS += -lm
CFLAGS = -x c -O0 -pedantic -Wall -Wextra -ggdb
CHOREOGRAPHERPATH = src/choreographer
CHOREOGRAPHER = ${CHOREOGRAPHERPATH}/main.c ${CHOREOGRAPHERPATH}/tkbc.c ${CHOREOGRAPHERPATH}/tkbc-ffmpeg.c ${CHOREOGRAPHERPATH}/tkbc-input-handler.c ${CHOREOGRAPHERPATH}/tkbc-script-api.c ${CHOREOGRAPHERPATH}/tkbc-script-handler.c ${CHOREOGRAPHERPATH}/tkbc-sound-handler.c ${CHOREOGRAPHERPATH}/tkbc-team-figures-api.c ${CHOREOGRAPHERPATH}/tkbc-ui.c ${CHOREOGRAPHERPATH}/tkbc-parser.c

all: options tkbc raylib build server client

options:
	@echo tbkc build options:
	@echo "CFLAGS = ${CFLAGS}"
	@echo "LIBS   = ${LIBS}"
	@echo "CC     = ${CC}"


server:
	${CC} ${INCLUDE} ${CFLAGS} -static -o build/server src/network/tkbc-server.c src/network/tkbc-server-client-handler.c src/network/tkbc-network-common.c src/choreographer/tkbc.c src/choreographer/tkbc-script-handler.c ${LIBS}

client:
	${CC} ${INCLUDE} ${CFLAGS} -o build/client src/network/tkbc-client.c src/network/tkbc-network-common.c src/choreographer/tkbc.c src/choreographer/tkbc-script-handler.c ${LIBS}


tkbc: build raylib first.o
	${CC} ${INCLUDE} ${CFLAGS} -o build/tkbc ${CHOREOGRAPHER} ${LIBS}

first.o: build raylib
	${CC} ${INCLUDE} ${CFLAGS} -c tkbc_scripts/first.c -o build/first.o

raylib:
	$(MAKE) -C ${RAYLIBPATH}

build:
	mkdir -p build

clean:
	rm -r build

.PHONY: all clean options tkbc tkbc.o build raylib server client

