#!/bin/bash

set -e

# Parse command line arguments:
KERNEL_ELF=""
DRIVE_FILE=""
WORK_DIR=""
DISPLAY_FLAG="-nographic"

while [[ $# -gt 0 ]]; do
	case $1 in
	-D|--drive)
		if [[ -z "$2" ]]; then
			echo "Error: $1 requires a path argument."
			echo "Usage: $0 [-d|--display] [-D|--drive <path>] <kernel_elf> <work_dir>"
			exit 1
		fi
		DRIVE_FILE="$2"
		shift 2
		;;
	-d|--display)
		DISPLAY_FLAG="true"
		shift
		;;
	*)
		if [[ -z "$KERNEL_ELF" ]]; then
			KERNEL_ELF=$1
		elif [[ -z "$WORK_DIR" ]]; then
			WORK_DIR=$1
		fi
		shift
		;;
	esac
done

# Check if the required arguments were provided.
if [[ -z "$KERNEL_ELF" || -z "$WORK_DIR" ]]; then
	echo "Usage: $0 [-d|--display] [-D|--drive <path>] <kernel_elf> <work_dir>"
	echo "	-d, --display		Enable display (GUI mode)."
	echo "	-D, --drive			Attach a block device image which will be accessed through virtio."
	exit 1
fi

# Convert KERNEL_ELF and DRIVE_FILE to absolute paths before changing to WORK_DIR
KERNEL_ELF=$(realpath "$KERNEL_ELF")
if [[ -n "$DRIVE_FILE" ]]; then
    DRIVE_FILE=$(realpath "$DRIVE_FILE")
fi

# Detect the Host operating system and set appropriate display option if display flag is set
DISPLAY_OPTIONS="-nographic"
if [[ "$DISPLAY_FLAG" == "true" ]]; then
    if [[ "$OSTYPE" == "darwin"* ]]; then
        DISPLAY_OPTIONS="-device ramfb -display cocoa"
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        DISPLAY_OPTIONS="-device ramfb -display gtk"
    else
        echo "Error: Unsupported operating system: $OSTYPE"
        echo "This script only supports macOS (darwin) and Linux (linux-gnu)"
        exit 1
    fi
fi

# Set the drive options for qemu if a drive was provided.
EXTRA_DRIVE_OPTS=()
if [[ -n "$DRIVE_FILE" ]]; then
    # Create a named drive 'rootfs' and attach as virtio-blk
    EXTRA_DRIVE_OPTS+=( -drive "file=${DRIVE_FILE},if=none,format=raw,id=rootfs" )
    EXTRA_DRIVE_OPTS+=( -device virtio-blk-device,drive=rootfs )
fi


# Check if the WORK_DIR exists, create it if not and then change to it.
if [ ! -d "$WORK_DIR" ]; then
    echo "WORK_DIR does not exist, creating it..."
    mkdir -p "$WORK_DIR"
else
    echo "WORK_DIR already exists."
fi
cd "$WORK_DIR"

# Check if the limine directory exists and create it if not.
LIMINE_DIR=limine
if [ ! -d "$LIMINE_DIR" ]; then
	echo "The $WORK_DIR/limine/ directory dones not exist, cloning it now."
    git clone https://github.com/limine-bootloader/limine.git --branch=v9.x-binary --depth=1
else
	echo "The $WORK_DIR/limine/ directory already exists, skipping clone."
fi

# Check if the limine binary has been built and if not build it.
if [ ! -f "$LIMINE_DIR/limine" ]; then
	echo "The $WORK_DIR/limine/limine binary does not exists, building it now."
	cd "$LIMINE_DIR"
    make -j$(nproc)
    cd ..
else
	echo "The $WORK_DIR/limine/limine binary already exists, skipping building."
fi

# Check if the OVMF directory exists and download it if not.
OVMF_DIR=ovmf
if [ ! -d "$OVMF_DIR" ]; then
	echo "The $WORK_DIR/ovmf/ directory does not exist, downloading firmware now."
	mkdir -p "$OVMF_DIR"
	curl -Lo "$OVMF_DIR/ovmf-code-riscv64.fd" "https://github.com/osdev0/edk2-ovmf-nightly/releases/latest/download/ovmf-code-riscv64.fd"
	dd if=/dev/zero of="$OVMF_DIR/ovmf-code-riscv64.fd" bs=1 count=0 seek=33554432 2>/dev/null
else
	echo "The $WORK_DIR/ovmf/ directory exists, skipping firmware download."
fi
OVMF_BIOS=ovmf/ovmf-code-riscv64.fd

# Check if DTB file exists and if not generate it.
DTB_FILE=qemu_virt.dtb
if [ ! -f "$DTB_FILE" ]; then
	echo "The $WORK_DIR/qemu_virt.dtb file does not exist, generating it now."
	qemu-system-riscv64 \
		    -machine virt,dumpdtb=$DTB_FILE \
		    -m 2G \
		    -cpu rv64,sstc=true \
		    -device qemu-xhci \
		    -device usb-kbd \
		    -device usb-mouse \
		    "${EXTRA_DRIVE_OPTS[@]}" \
		    ${DISPLAY_OPTIONS} \
		    -serial mon:stdio
fi

# Check if the limine configuration file exists and if not generate it.
LIMINE_CONF=limine.conf
if [ ! -f "$LIMINE_CONF" ]; then
	echo "The $WORK_DIR/limine.conf file does not exist, generating it now."
	cat <<EOF > limine.conf
# Timeout in seconds that Limine will use before automatically booting.
timeout: 1
serial: yes

# The entry name that will be displayed in the boot menu.
/Octiron
    # We use the Limine boot protocol.
    protocol: limine

    # Path to the kernel to boot. boot():/ represents the partition on which limine.conf is located.
    path: boot():/boot/MirrodOS.elf
    dtb_path: boot():/boot/qemu_virt.dtb
EOF
fi


# Create the ISO folder and copy over the kernel.
ISO_DIR=iso
if [ -d "$ISO_DIR" ]; then
	rm -rf "$ISO_DIR"
fi
mkdir -p "$ISO_DIR"
mkdir -p "$ISO_DIR/boot"
cp -v $KERNEL_ELF $ISO_DIR/boot/MirrodOS.elf
cp -v $DTB_FILE $ISO_DIR/boot/qemu_virt.dtb
mkdir -p "$ISO_DIR/boot/limine"
cp -v $LIMINE_CONF $ISO_DIR/boot/limine/
cp -v $LIMINE_DIR/limine-uefi-cd.bin $ISO_DIR/boot/limine/
cp -v $LIMINE_DIR/limine-bios-cd.bin $ISO_DIR/boot/limine/
cp -v $LIMINE_DIR/limine-bios.sys $ISO_DIR/boot/limine/
mkdir -p $ISO_DIR/EFI/BOOT
cp -v $LIMINE_DIR/BOOTRISCV64.EFI $ISO_DIR/EFI/BOOT/

# Bundle up the ISO folder into a bootable image.
IMAGE_NAME=MirrodOS.iso
xorriso -as mkisofs -R -r -J -b /boot/limine/limine-bios-cd.bin \
        -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
        -apm-block-size 2048 --efi-boot /boot/limine/limine-uefi-cd.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        $ISO_DIR -o ${IMAGE_NAME}

$LIMINE_DIR/limine bios-install $IMAGE_NAME

# Run QEMU with this new image
qemu-system-riscv64 \
    -machine virt \
    -m 2G \
    -cpu rv64,sstc=true \
    -device qemu-xhci \
    -device usb-kbd \
    -device usb-mouse \
    -drive if=pflash,unit=0,format=raw,file="${OVMF_BIOS}",readonly=on \
    -cdrom "${IMAGE_NAME}" \
    "${EXTRA_DRIVE_OPTS[@]}" \
    ${DISPLAY_OPTIONS} \
    -serial mon:stdio \
    -global virtio-mmio.force-legacy=false
