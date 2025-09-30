#!/bin/bash

URL="https://github.com/osdev0/edk2-ovmf-nightly/releases/latest/download/ovmf-code-riscv64.fd"
DEST_DIR=ovmf

mkdir -p ${DEST_DIR}
curl -Lo ${DEST_DIR}/ovmf-code-riscv64.fd ${URL}
dd if=/dev/zero of=${DEST_DIR}/ovmf-code-riscv64.fd bs=1 count=0 seek=33554432 2>/dev/null	