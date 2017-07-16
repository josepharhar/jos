#!/bin/bash

set -e

source shrc.sh

wget "ftp://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.gz"
tar -xzf binutils-${BINUTILS_VERSION}.tar.gz

wget "ftp://ftp.gnu.org/gnu/gcc/gcc-6.3.0/gcc-${GCC_VERSION}.tar.gz"
tar -xzf gcc-${GCC_VERSION}.tar.gz

cd gcc-${GCC_VERSION}
contrib/download_prerequisites
find . ! -readable -prune -o -type f -print | grep configure | xargs sed -i "s/-V -qversion//g"
cd ..

mkdir build-binutils
cd build-binutils
$DIR_454/binutils-${BINUTILS_VERSION}/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install
cd ..

mkdir build-gcc
cd build-gcc
$DIR_454/gcc-${GCC_VERSION}/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc
cd ..

echo compile-deps.sh completed successfully!
