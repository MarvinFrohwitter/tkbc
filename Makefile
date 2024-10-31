# clang bug with size_t, variadics, optimization and compiler caching
CC = gcc
RAYLIBPATH = external/raylib-5.0/src
INCLUDE = -I src/
INCLUDE += -I ${RAYLIBPATH} -I tkbc_scripts/ -I build/
LIBS = -L ${RAYLIBPATH}
LIBS += -l:libraylib.a
LIBS += -lm
CFLAGS = -x c -O3 -pedantic -Wall -Wextra -ggdb

all: options tkbc raylib build server client

options:
	@echo tbkc build options:
	@echo "CFLAGS = ${CFLAGS}"
	@echo "LIBS   = ${LIBS}"
	@echo "CC     = ${CC}"


server:
	${CC} ${INCLUDE} ${CFLAGS} -static -o build/server src/network/tkbc-server.c src/network/tkbc-server-client-handler.c src/network/tkbc-network-common.c

client:
	${CC} ${INCLUDE} ${CFLAGS} -static -o build/client src/network/tkbc-client.c src/network/tkbc-network-common.c

tkbc: build raylib first.o
	${CC} ${INCLUDE} ${CFLAGS} -o build/tkbc src/main.c src/tkbc.c src/tkbc-ui.c ${LIBS}

first.o: build raylib
	${CC} ${INCLUDE} ${CFLAGS} -c tkbc_scripts/first.c -o build/first.o

raylib:
	$(MAKE) -C ${RAYLIBPATH}

build:
	mkdir -p build

clean:
	rm -r build

.PHONY: all clean options tkbc tkbc.o build raylib server client
