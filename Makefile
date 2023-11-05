ifdef ComSpec
O=.obj
EXE=.exe
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

lib: build/$(REMOTE_BT_LIB)

cli: build/remote_bt_cli$(EXE)

build/remote_bt_cli$(EXE): build/$(REMOTE_BT_LIB) remote_bt_cli.c
ifdef ComSpec
	$(CC) $(CFLAGS) $(OUTPUT) remote_bt_cli.c /link /LIBPATH:build remote_bt.lib ssh.lib libcrypto.lib
else
	$(CC) $(CFLAGS) $(OUTPUT) remote_bt_cli.c -Lbuild -lremote_bt -lssh -lcrypto
endif

build/$(REMOTE_BT_LIB): build/remote_bt$(O) build/torrent$(O) build/bencode$(O)
ifdef ComSpec
	lib $^ -OUT:$@
else
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
	rm build/*$(O) build/*.pdb build/*.ilk build/$(REMOTE_BT_LIB) build/remote_bt_cli$(EXE)
else
	rm build/*$(O) build/$(REMOTE_BT_LIB) build/remote_bt_cli$(EXE)
endif
