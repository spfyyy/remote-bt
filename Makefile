ifdef ComSpec
O=.obj
EXE=.exe
CFLAGS=/Zi /std:c11 /WX /DWINDOWS
COMPILE=/c
OUTPUT=/Fobuild/ /Fdbuild/ /Febuild/
else
O=.o
EXE=
CFLAGS=-g -std=c11 -Werror -fPIC
COMPILE=-c
OUTPUT=-o $@
endif

ifdef ComSpec
lib: build/remote_bt.lib build/remote_bt.dll
else
lib: build/libremote_bt.so
endif

cli: build/remote_bt_cli$(EXE)

ifdef ComSpec
build/remote_bt_cli$(EXE): build/remote_bt.lib build/remote_bt.dll remote_bt_cli.c
	$(CC) $(CFLAGS) $(OUTPUT) remote_bt_cli.c /link /LIBPATH:build remote_bt.lib
else
build/remote_bt_cli$(EXE): build/libremote_bt.so remote_bt_cli.c
	$(CC) $(CFLAGS) $(OUTPUT) remote_bt_cli.c -Lbuild -lremote_bt
endif

ifdef ComSpec
build/remote_bt.lib build/remote_bt.dll: build/remote_bt$(O) build/torrent$(O) build/bencode$(O)
	LINK /DLL $^ ssh.lib libcrypto.lib /OUT:build\remote_bt.dll /IMPLIB:build\remote_bt.lib
else
build/libremote_bt.so: build/remote_bt$(O) build/torrent$(O) build/bencode$(O)
	$(CC) $(CFLAGS) $(OUTPUT) -shared $^ -lssh
endif

build/remote_bt$(O): build remote_bt.c remote_bt.h types.h ssh_config.c torrent.h
	$(CC) $(CFLAGS) $(COMPILE) $(OUTPUT) remote_bt.c

build/torrent$(O): build torrent.c torrent.h bencode.h
	$(CC) $(CFLAGS) $(COMPILE) $(OUTPUT) torrent.c

build/bencode$(O): build bencode.c bencode.h
	$(CC) $(CFLAGS) $(COMPILE) $(OUTPUT) bencode.c

build:
	mkdir "build"

clean:
	rm build/*
