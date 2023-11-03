CC=cc
CFLAGS=-g

cli: build/remote_bt_cli.exe

build/remote_bt_cli.exe: remote_bt_cli.c build/remote_bt.o build/bencode.o build/torrent.o
	$(CC) $(CFLAGS) -o build/remote_bt_cli.exe remote_bt_cli.c build/remote_bt.o build/torrent.o build/bencode.o -lssh

build/remote_bt.o: remote_bt.c remote_bt.h types.h ssh_config.c
	$(CC) $(CFLAGS) -c -o build/remote_bt.o remote_bt.c

build/torrent.o: torrent.c torrent.h
	$(CC) $(CFLAGS) -c -o build/torrent.o torrent.c

build/bencode.o: bencode.c bencode.h
	$(CC) $(CFLAGS) -c -o build/bencode.o bencode.c
