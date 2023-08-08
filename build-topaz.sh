#!/bin/bash

pushd build || return
cmake ../ || exit
make || exit
popd || return

BINARY_DIR=./build
DISK_IMAGE_PATH=./topaz.img

# copy our C bootloader and our stage 2 bootstrapper to our hidden sectors before our FAT filesystem
dd if=${BINARY_DIR}/Stage2.bin of=${DISK_IMAGE_PATH} bs=512 count=1 seek=1 conv=notrunc
dd if=${BINARY_DIR}/Boot32 of=${DISK_IMAGE_PATH} bs=512 count=31 seek=2 conv=notrunc

qemu-system-i386 -m 32 topaz.img -monitor stdio