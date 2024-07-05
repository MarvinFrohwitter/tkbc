CC = clang
RAYLIBPATH = external/raylib-5.0/src
INCLUDE = -I ${RAYLIBPATH} -I tkbc_scripts/ -I build/ -I src/
LIBS = -lraylib -lm
CFLAGS = -x c -O0 -pedantic -Wall -Wextra -ggdb

all: options tkbc raylib build

options:
	@echo tbk.c build options:
	@echo "CFLAGS = ${CFLAGS}"
	@echo "LIBS   = ${LIBS}"
	@echo "CC     = ${CC}"


tkbc: build raylib first.o
	${CC} ${INCLUDE} ${CFLAGS} ${LIBS} -o build/tkbc src/main.c src/tkbc.c src/tkbc-ui.c

first.o: build raylib
	${CC} ${INCLUDE} ${CFLAGS} -c tkbc_scripts/first.c -o build/first.o

raylib:
	$(MAKE) -C ${RAYLIBPATH}

build:
	mkdir -p build

clean:
	rm -r build

.PHONY: all clean options tkbc tkbc.o build raylib
