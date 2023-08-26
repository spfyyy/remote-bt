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
gcc -Wall -fsanitize=address "$PROJ_DIR/src/test/test_suite.c" -o test || error "build failed"
./test || error "tests failed"
cd "$START_DIR"
