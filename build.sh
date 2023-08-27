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

# build depencies
if [ ! -d src/dependencies/build ]; then
	mkdir -p src/dependencies/build
fi

# build openssl (needed to build libssh)
if [ ! -e src/dependencies/build/openssl/lib64/libcrypto.a ]; then
	cd src/dependencies/openssl || error "openssl not found, did you clone submodules?"
	./Configure --prefix=$PROJ_DIR/src/dependencies/build/openssl --no-shared && make && make install || error "failed to build openssl"
	cd "$PROJ_DIR"
fi

# build libssh
if [ ! -e src/dependencies/build/libssh/lib/libssh.so ]; then
	if [ ! -d src/dependencies/libssh/build ]; then
		mkdir src/dependencies/libssh/build || error "could not make libssh build folder, did you clone submodules?"
	fi
	cd src/dependencies/libssh/build
	cmake -S "$PROJ_DIR/src/dependencies/libssh" -DCMAKE_INSTALL_PREFIX="$PROJ_DIR/src/dependencies/build/libssh" -DCMAKE_BUILD_TYPE=Release -DWITH_ZLIB=OFF -DOPENSSL_CRYPTO_LIBRARY="$PROJ_DIR/src/dependencies/build/openssl/lib64/libcrypto.a" -DOPENSSL_INCLUDE_DIR="$PROJ_DIR/src/dependencies/build/openssl/include" && make && make install || error "failed to build libssh"
	cd "$PROJ_DIR"
fi

if [ ! -d build ]; then
	mkdir build
fi

cd build
gcc -fsanitize=address -I"$PROJ_DIR/src/dependencies/build/libssh/include" "$PROJ_DIR/src/torrent.c" "$PROJ_DIR/src/dependencies/build/libssh/lib/libssh.so" -o torrent || error "build failed"
cd "$START_DIR"

