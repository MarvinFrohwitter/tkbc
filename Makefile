CC = cc
RAYLIBPATH = external/raylib-5.0/src
INCLUDE = -I ${RAYLIBPATH} -I tkbc_scripts/ -I build/ -I src/
LIBS = -lraylib -lm
CFLAGS = -x c -O3 -pedantic -Wall -Wextra -std=c11 -ggdb

all: options tkbc raylib build

options:
	@echo tbk.c build options:
	@echo "CFLAGS = ${CFLAGS}"
	@echo "LIBS   = ${LIBS}"
	@echo "CC     = ${CC}"


tkbc: build raylib first.o
	${CC} ${INCLUDE} ${CFLAGS} ${LIBS} -o build/tkbc src/main.c src/tkbc.c

first.o: build raylib
	${CC} ${INCLUDE} ${CFLAGS} ${LIBS} -c tkbc_scripts/first.c -o build/first.o

raylib:
	$(MAKE) -C ${RAYLIBPATH}

build:
	mkdir -p build

clean:
	rm -r build

.PHONY: all clean options tkbc tkbc.o build raylib
