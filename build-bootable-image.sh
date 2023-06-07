#!/bin/sh

nasm -f bin boot16.asm -o boot16.bin
nasm -f bin stage2.asm -o stage2.bin

mformat -i test.img -B boot16.bin -F
mcopy -i test.img stage2.bin ::

qemu-system-i386 -m 64 test.img