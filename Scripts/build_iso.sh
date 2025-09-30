#!/bin/bash

IMAGE_NAME=$1
KERNEL_PATH=$2
LIMINE_CONF_PATH=$3
DTB_PATH=$4

# Download and compile limine
if [ ! -d "limine" ]; then
	git clone https://github.com/limine-bootloader/limine.git --branch=v9.6.0-binary --depth=1
	cd limine
	make -j$(nproc)
	cd ..
fi

mkdir -p iso_root/boot
cp -v ${KERNEL_PATH} iso_root/boot/MirodosKernel.elf
cp -v ${DTB_PATH} iso_root/boot/qemu_virt.dtb

mkdir -p iso_root/boot/limine
cp -v ${LIMINE_CONF_PATH} iso_root/boot/limine/
cp -v limine/limine-uefi-cd.bin iso_root/boot/limine/
cp -v limine/limine-bios-cd.bin iso_root/boot/limine/
cp -v limine/limine-bios.sys iso_root/boot/limine

mkdir -p iso_root/EFI/BOOT
cp -v limine/BOOTRISCV64.EFI iso_root/EFI/BOOT/

xorriso -as mkisofs -R -r -J -b /boot/limine/limine-bios-cd.bin \
        -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
        -apm-block-size 2048 --efi-boot /boot/limine/limine-uefi-cd.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        iso_root/ -o ${IMAGE_NAME}

./limine/limine bios-install ${IMAGE_NAME}