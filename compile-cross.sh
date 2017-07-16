#!/bin/bash

set -e

source shrc.sh

source-binutils() {
  wget "ftp://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.gz"
  tar -xzf binutils-${BINUTILS_VERSION}.tar.gz
  rm binutils-${BINUTILS_VERSION}.tar.gz
}

source-gcc() {
  wget "ftp://ftp.gnu.org/gnu/gcc/gcc-6.3.0/gcc-${GCC_VERSION}.tar.gz"
  tar -xzf gcc-${GCC_VERSION}.tar.gz
  rm gcc-${GCC_VERSION}.tar.gz

  cd gcc-${GCC_VERSION}
  contrib/download_prerequisites
  find . ! -readable -prune -o -type f -print | grep configure | xargs sed -i "s/-V -qversion//g"
  cd $DIR_454
}

binutils() {
  mkdir -p $DIR_454/build-binutils
  cd $DIR_454/build-binutils
  $DIR_454/binutils-${BINUTILS_VERSION}/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
  make
  make install
  cd $DIR_454
}

gcc() {
  mkdir -p $DIR_454/build-gcc
  cd $DIR_454/build-gcc
  $DIR_454/gcc-${GCC_VERSION}/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
  make all-gcc
  make all-target-libgcc
  make install-gcc
  make install-target-libgcc
  cd $DIR_454
}

if [ $# -eq 0 ]; then
  source-binutils
  source-gcc
  binutils
  gcc
else
  for arg in $@; do
    $arg
  done
fi

echo compile-deps.sh completed successfully!
