# clang bug with size_t, variadics, optimization and compiler caching
# gcc is broken when using a format string that just includes "%s" longer ones are fine like "Hello %s" with NULL.
# printf("%s", NULL);

all: build skystep skystep-client skystep-server skystep-win64 skystep-client-win64 skystep-server-win64

build:
	cc -o cb cb2.c

build-dir: build
	./cb build-dir

clean: build
	./cb clean

assets: build
	./cb assets

script_api_string: build
	./cb script_api_string

first.o: build
	./cb first.o

first.o-win64: build
	./cb first.o windows

skystep:
	make skystep-client
	cp build/skystep-client build/skystep

skystep-client: build
	./cb skystep-client

skystep-win64:
	make skystep-client-win64
	cp build/skystep-client.exe build/skystep.exe

skystep-client-win64: build
	./cb skystep-client windows

skystep-server: build
	./cb skystep-server

skystep-server-win64: build
	./cb skystep-server windows

libtcc: build
	./cb libtcc

libtcc-linux: build
	./cb libtcc linux

libtcc-windows: build
	./cb libtcc windows



test: build
	./cb test

test-verbose: build
	./cb test verbose

test-short: build
	./cb test short


# --- Installation ---------------------------------------------------------
PREFIX ?= $(HOME)/.local

INSTALL ?= install
BINDIR ?= $(PREFIX)/bin
DESKTOP_DIR ?= $(PREFIX)/share/applications
ICON_DIR ?= $(PREFIX)/share/icons/hicolor/256x256/apps

install: build
	./cb skystep
	./cb skystep-client
	$(INSTALL) -d $(BINDIR) $(DESKTOP_DIR) $(ICON_DIR)
	$(INSTALL) -m 755 build/skystep $(BINDIR)/skystep
	$(INSTALL) -m 755 build/skystep-client $(BINDIR)/skystep-client
	$(INSTALL) -m 644 packaging/skystep.desktop $(DESKTOP_DIR)/
	$(INSTALL) -m 644 assets/Logos/256x256_Logo.png $(ICON_DIR)/skystep.png
	@update-desktop-database $(DESKTOP_DIR) 2>/dev/null || true
	@gtk-update-icon-cache -f -t $(PREFIX)/share/icons/hicolor 2>/dev/null || true


.PHONY: all clean build build-dir assets script_api_string first.o first.o-win64 test test-verbose test-short libtcc libtcc-linux libtcc-windows skystep skystep-win64 skystep-client skystep-client-win64 skystep-server skystep-server-win64 install
