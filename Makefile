ifdef ComSpec
O=.obj
EXE=.exe
REMOTE_BT_DLL=remote_bt.dll
REMOTE_BT_LIB=remote_bt.lib
CFLAGS=/Zi /std:c11 /WX
COMPILE=/c
OUTPUT=/Fobuild/ /Fdbuild/ /Febuild/
else
O=.o
EXE=
REMOTE_BT_LIB=libremote_bt.a
CFLAGS=-g -std=c11 -Werror
COMPILE=-c
OUTPUT=-o $@
endif

ifdef ComSpec
lib: build/$(REMOTE_BT_LIB) build/$(REMOTE_BT_DLL)
else
lib: build/$(REMOTE_BT_LIB)
endif

cli: build/remote_bt_cli$(EXE)

build/remote_bt_cli$(EXE): build/$(REMOTE_BT_LIB) remote_bt_cli.c
ifdef ComSpec
	$(CC) $(CFLAGS) $(OUTPUT) remote_bt_cli.c /link /LIBPATH:build $(REMOTE_BT_LIB)
else
	$(CC) $(CFLAGS) $(OUTPUT) remote_bt_cli.c -Lbuild -lremote_bt -lssh -lcrypto
endif

ifdef ComSpec
build/$(REMOTE_BT_LIB) build/$(REMOTE_BT_DLL): build/remote_bt$(O) build/torrent$(O) build/bencode$(O)
	LINK /DLL $^ ssh.lib libcrypto.lib /OUT:build\$(REMOTE_BT_DLL) /IMPLIB:build\$(REMOTE_BT_LIB)
else
build/$(REMOTE_BT_LIB): build/remote_bt$(O) build/torrent$(O) build/bencode$(O)
	ar r $@ $^
endif

build/remote_bt$(O): remote_bt.c remote_bt.h types.h ssh_config.c torrent.h
	$(CC) $(CFLAGS) $(COMPILE) $(OUTPUT) remote_bt.c

build/torrent$(O): torrent.c torrent.h bencode.h
	$(CC) $(CFLAGS) $(COMPILE) $(OUTPUT) torrent.c

build/bencode$(O): bencode.c bencode.h
	$(CC) $(CFLAGS) $(COMPILE) $(OUTPUT) bencode.c

clean:
ifdef ComSpec
	rm build/*$(O) build/*.pdb build/*.ilk build/*.ilk build/*.lib build/*.dll build/*.exe
else
	rm build/*$(O) build/*.a build/*.so build/remote_bt_cli$(EXE)
endif
