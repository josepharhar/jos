export DIR_454=$(pwd)

export PREFIX="$DIR_454/cross"
export TARGET="x86_64-elf"
export PATH="$PREFIX:$PREFIX/bin:$PATH"

mkdir -p $PREFIX
mkdir -p "$DIR_454/image"

export BINUTILS_VERSION="2.28"
export GCC_VERSION="6.3.0"

alias configure-binutils="$DIR_454/binutils-${BINUTILS_VERSION}/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror"
alias make-binutils="make && make install"

alias configure-gcc="$DIR_454/gcc-${GCC_VERSION}/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers"
alias make-gcc="make all-gcc && make all-target-libgcc && make install-gcc && make install-target-libgcc"

alias ccc="${TARGET}-gcc"
alias emu="qemu-system-x86_64 -curses -drive format=raw,file=os.img"
