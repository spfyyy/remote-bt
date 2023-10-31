#!/bin/sh
START_DIR=$(pwd)
cd $(dirname $0)
PROJ_DIR=$(pwd)

error()
{
	echo "$1" >&2
	cd "$START_DIR"
	exit 1
}

if [ ! -d build ]; then
	mkdir build
fi

cd build
gcc "$PROJ_DIR/src/remote_bt.c" -g -o torrent -lssh || error "build failed"
cd "$START_DIR"

