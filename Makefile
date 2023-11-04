CFLAGS=-g -fsanitize=address

ifdef ComSpec
O=.obj
EXE=.exe
REMOTE_BT_LIB=remote_bt.lib
else
O=.o
EXE=
REMOTE_BT_LIB=libremote_bt.a
endif

lib: build/$(REMOTE_BT_LIB)

cli: build/remote_bt_cli$(EXE)

build/remote_bt_cli$(EXE): lib remote_bt_cli.c
	$(CC) $(CFLAGS) -o $@ remote_bt_cli.c -Lbuild -lremote_bt -lssh

build/$(REMOTE_BT_LIB): build/remote_bt$(O) build/torrent$(O) build/bencode$(O)
	ar r $@ $^

build/remote_bt$(O): remote_bt.c remote_bt.h types.h torrent.h ssh_config.c
	$(CC) $(CFLAGS) -c -o $@ remote_bt.c

build/torrent$(O): torrent.c torrent.h
	$(CC) $(CFLAGS) -c -o $@ torrent.c

build/bencode$(O): bencode.c bencode.h
	$(CC) $(CFLAGS) -c -o $@ bencode.c

clean:
	rm build/*$(O) build/$(REMOTE_BT_LIB) build/remote_bt_cli$(EXE)
