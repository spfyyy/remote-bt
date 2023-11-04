ifdef ComSpec
O=.obj
EXE=.exe
REMOTE_BT_LIB=remote_bt.lib
CFLAGS=-Zi -Fobuild/ -Fdbuild/ -Febuild/
else
O=.o
EXE=
REMOTE_BT_LIB=libremote_bt.a
CFLAGS=-g
endif

lib: build/$(REMOTE_BT_LIB)

cli: build/remote_bt_cli$(EXE)

build/remote_bt_cli$(EXE): lib remote_bt_cli.c
	$(CC) $(CFLAGS) remote_bt_cli.c /link /LIBPATH:build remote_bt.lib ssh.lib libcrypto.lib

build/$(REMOTE_BT_LIB): build/remote_bt$(O) build/torrent$(O) build/bencode$(O)
	lib $^ -OUT:$@
# ar r $@ $^

build/remote_bt$(O): remote_bt.c remote_bt.h types.h ssh_config.c torrent.h bencode.h
	$(CC) $(CFLAGS) -c remote_bt.c

build/torrent$(O): torrent.c torrent.h bencode.h
	$(CC) $(CFLAGS) -c torrent.c

build/bencode$(O): bencode.c bencode.h
	$(CC) $(CFLAGS) -c bencode.c

clean:
	rm build/*$(O) build/*.pdb build/*.ilk build/$(REMOTE_BT_LIB) build/remote_bt_cli$(EXE) &2>/dev/null
