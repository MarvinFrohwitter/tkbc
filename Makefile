# clang bug with size_t, variadics, optimization and compiler caching
# gcc is broken when using a format string that just includes "%s" longer ones are fine like "Hello %s" with NULL.
# printf("%s", NULL);

all: build tkbc client server tkbc-win64 client-win64 server-win64

build:
	cc -o cb cb2.c

build-dir: build
	./cb build-dir

clean: build
	./cb clean

first.o: build
	./cb first.o

first.o-win64: build
	./cb first.o windows

tkbc: build
	./cb tkbc

tkbc-win64: build
	./cb tkbc windows


client: build
	./cb client

client-win64: build
	./cb client windows


server: build
	./cb server

server-win64: build
	./cb server windows



test: build
	./cb test

test-verbose: build
	./cb test verbose

test-short: build
	./cb test short


.PHONY: all clean tkbc tkbc.o build client test server poll-server
