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
gcc -Werror -fsanitize=address "$PROJ_DIR/src/torrent.c" -o torrent || error "build failed"
cd "$START_DIR"

