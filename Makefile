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

assets: build
	./cb assets

first.o: build
	./cb first.o

first.o-win64: build
	./cb first.o windows

tkbc: build
	./cb client
	cp ./build/client ./build/tkbc

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


# --- Installation ---------------------------------------------------------
PREFIX ?= $(HOME)/.local

INSTALL ?= install
BINDIR ?= $(PREFIX)/bin
DESKTOP_DIR ?= $(PREFIX)/share/applications
ICON_DIR ?= $(PREFIX)/share/icons/hicolor/256x256/apps

install: build
	./cb tkbc
	./cb client
	$(INSTALL) -d $(BINDIR) $(DESKTOP_DIR) $(ICON_DIR)
	$(INSTALL) -m 755 build/tkbc $(BINDIR)/tkbc
	$(INSTALL) -m 755 build/client $(BINDIR)/client
	$(INSTALL) -m 644 packaging/tkbc.desktop $(DESKTOP_DIR)/
	$(INSTALL) -m 644 assets/Logos/256x256_Logo.png $(ICON_DIR)/tkbc.png
	@update-desktop-database $(DESKTOP_DIR) 2>/dev/null || true
	@gtk-update-icon-cache -f -t $(PREFIX)/share/icons/hicolor 2>/dev/null || true


.PHONY: all clean tkbc tkbc.o build client test server poll-server install
