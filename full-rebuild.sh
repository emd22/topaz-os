#!/bin/bash


BINARY_DIR=./build
DISK_IMAGE_PATH=./topaz.img

dd if=/dev/zero of=${DISK_IMAGE_PATH} bs=1M count=32
# format our fat16 filesystem, use our boot16 bootloader as the bootstrap with 12 hidden sectors
mformat -i ${DISK_IMAGE_PATH} -B ${BINARY_DIR}/boot16.bin -H 12

./build.sh