#!/bin/sh

BUILD_DIR=build

nasm -fbin boot16.asm -o $BUILD_DIR/boot16.bin
nasm -fbin stage2.asm -o $BUILD_DIR/stage2.bin

as --32 b32strap.S -c -o $BUILD_DIR/b32strap.o
gcc -Os -c -m32 -fno-stack-protector -fno-pie -ffreestanding -nostdlib -nostdinc -nostartfiles boot32.c -o $BUILD_DIR/boot32.o

ld --oformat=binary -melf_i386 -nostdlib -Tlinker.ld $BUILD_DIR/b32strap.o $BUILD_DIR/boot32.o -o $BUILD_DIR/boot32.bin

mformat -i test.img -B $BUILD_DIR/boot16.bin -H 8
dd if=$BUILD_DIR/stage2.bin of=test.img bs=512 count=1 seek=1 conv=notrunc
dd if=$BUILD_DIR/boot32.bin of=test.img bs=512 count=7 seek=2 conv=notrunc
#mcopy -i test.img STAGE2.BIN ::

qemu-system-i386 -m 32 test.img -monitor stdio