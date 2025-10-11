#!/bin/bash


# Parse command line arguments
DISPLAY_FLAG="-nographic"
IMAGE_FILE=""
BIOS_FILE=""
DRIVE_FILE=""

while [[ $# -gt 0 ]]; do
    case $1 in
        -D|--drive)
            if [[ -z "$2" ]]; then
                echo "Error: $1 requires a path argument"
                echo "Usage: $0 [-d|--display] [-D|--drive <path>] <image_file> <bios_file>"
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
            if [[ -z "$IMAGE_FILE" ]]; then
                IMAGE_FILE=$1
            elif [[ -z "$BIOS_FILE" ]]; then
                BIOS_FILE=$1
            fi
            shift
            ;;
    esac
done

# Check if required arguments are provided
if [[ -z "$IMAGE_FILE" || -z "$BIOS_FILE" ]]; then
    echo "Usage: $0 [-d|--display] [-D|--drive <path>] <image_file> <bios_file>"
    echo "  -d, --display    Enable display (GUI mode)"
    echo "  -D, --drive      Attach a block device image as virtio-blk (optional)"
    exit 1
fi

# Detect OS and set appropriate display option if display flag is set
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

set -ex

# Optional drive/device arguments
EXTRA_DRIVE_OPTS=()
if [[ -n "$DRIVE_FILE" ]]; then
    # Create a named drive 'rootfs' and attach as virtio-blk
    EXTRA_DRIVE_OPTS+=( -drive "file=${DRIVE_FILE},if=none,format=raw,id=rootfs" )
    EXTRA_DRIVE_OPTS+=( -device virtio-blk-device,drive=rootfs )
fi

qemu-system-riscv64 \
    -machine virt \
    -m 2G \
    -cpu rv64,sstc=true \
    -device qemu-xhci \
    -device usb-kbd \
    -device usb-mouse \
    -drive if=pflash,unit=0,format=raw,file="${BIOS_FILE}",readonly=on \
    -cdrom "${IMAGE_FILE}" \
    "${EXTRA_DRIVE_OPTS[@]}" \
    ${DISPLAY_OPTIONS} \
    -serial mon:stdio
