set arch i386:x86-64:intel
target remote localhost:1234
symbol-file image/boot/kernel.bin
tui enable
