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

# build openssl (needed to build libssh)
if [ ! -e src/dependencies/openssl/libcrypto.so ]; then
	cd src/dependencies/openssl || error "openssl not found, did you clone submodules?"
	./Configure no-apps no-docs && make || error "failed to build openssl"
	cd "$PROJ_DIR"
fi

# build libssh
if [ ! -e src/dependencies/libssh/build/lib/libssh.so ]; then
	if [ ! -d src/dependencies/libssh/build ]; then
		mkdir src/dependencies/libssh/build || error "could not make libssh build folder, did you clone submodules?"
	fi
	cd src/dependencies/libssh/build
	cmake -S "$PROJ_DIR/src/dependencies/libssh" -DCMAKE_BUILD_TYPE=Release -DWITH_ZLIB=OFF -DWITH_EXAMPLES=OFF -DOPENSSL_CRYPTO_LIBRARY="$PROJ_DIR/src/dependencies/openssl/libcrypto.so" -DOPENSSL_INCLUDE_DIR="$PROJ_DIR/src/dependencies/openssl/include" && cmake --build . --config Release || error "failed to build libssh"
	cd "$PROJ_DIR"
fi

# ssh_config.c is private, so it needs to be created
if [ ! -e src/ssh_config.c ]; then
	cp src/ssh_config.c.example src/ssh_config.c || error "could not create ssh_config.c"
fi

if [ ! -d build ]; then
	mkdir build
fi

cd build
gcc -fsanitize=address -I"$PROJ_DIR/src/dependencies/libssh/include" -I"$PROJ_DIR/src/dependencies/libssh/build/include" "$PROJ_DIR/src/remote_bt.c" "$PROJ_DIR/src/dependencies/libssh/build/lib/libssh.so" -g -o torrent || error "build failed"
cd "$START_DIR"

