cmake_minimum_required(VERSION 3.16)
project(mirodos VERSION 1.0 LANGUAGES C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_COMPILER "/usr/bin/riscv64-unknown-elf-gcc")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Directories for scripts
set(SCRIPTS_DIR ${CMAKE_SOURCE_DIR}/Scripts)

# Add the Kernel subproject
add_subdirectory(Kernel)

set(IMAGE_NAME "MirodosKernel.iso")
add_custom_target(iso_file
    COMMAND ${SCRIPTS_DIR}/build_iso.sh ${IMAGE_NAME} $<TARGET_FILE:MirodosKernel.elf> ${SCRIPTS_DIR}/limine.conf ${SCRIPTS_DIR}/qemu_virt.dtb
    DEPENDS MirodosKernel.elf
    COMMENT "Building a bootable image file"
)

# Custom target to trigger OVMF download
add_custom_target(download_ovmf
    COMMAND ${SCRIPTS_DIR}/download_ovmf.sh
    COMMENT "Downloading OVMF for riscv64"
)

# Custom target to run the kernel in QEMU
add_custom_target(run
    COMMAND ${SCRIPTS_DIR}/qemu.sh ${IMAGE_NAME} ovmf/ovmf-code-riscv64.fd
    DEPENDS iso_file download_ovmf
    COMMENT "Running Octiron in QEMU"
)
