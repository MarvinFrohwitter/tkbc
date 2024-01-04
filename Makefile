CC = cc
LIBS = -lraylib
CFLAGS = -x c -O3 -pedantic -Wall -Wextra -std=c11 -ggdb

all: options tkbc tkb.c.o

options:
	@echo tbk.c build options:
	@echo "CFLAGS = ${CFLAGS}"
	@echo "LIBS   = ${LIBS}"
	@echo "CC     = ${CC}"


tkbc:
	${CC} -I ../raylib-5.0/src/ ${CFLAGS} ${LIBS} -o tkbc main.c

tkb.c.o:
	${CC} -I ../raylib-5.0/src/ ${CFLAGS} ${LIBS} -c main.c -o $@


clean:
	rm -f tkbc tkb.c.o

.PHONY: all clean options tkbc tkb.c.o
