#!/bin/bash

# Parse command line arguments
DISPLAY_FLAG="-nographic"
IMAGE_FILE=""
BIOS_FILE=""

while [[ $# -gt 0 ]]; do
    case $1 in
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
    echo "Usage: $0 [-d|--display] <image_file> <bios_file>"
    echo "  -d, --display    Enable display (GUI mode)"
    exit 1
fi

# Detect OS and set appropriate display option if display flag is set
DISPLAY_OPTIONS=""
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

qemu-system-riscv64 \
    -machine virt \
    -m 2G \
    -cpu rv64,sstc=true \
    -device qemu-xhci \
    -device usb-kbd \
    -device usb-mouse \
    -drive if=pflash,unit=0,format=raw,file=${BIOS_FILE},readonly=on \
    -cdrom ${IMAGE_FILE} \
    ${DISPLAY_OPTIONS} \
    -serial mon:stdio
