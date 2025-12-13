cmake_minimum_required(VERSION 3.16)
project(mirodos VERSION 1.0 LANGUAGES C ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_COMPILER "riscv64-unknown-elf-gcc")
set(CMAKE_ASM_COMPILER "riscv64-unknown-elf-gcc")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Directories for scripts
set(SCRIPTS_DIR ${CMAKE_SOURCE_DIR}/Scripts)

# Add the Kernel subproject
add_subdirectory(Kernel)

# Add the Userspace subproject
add_subdirectory(Userspace)

# Custom target to run the kernel in QEMU
add_custom_target(run
    COMMAND ${SCRIPTS_DIR}/buildrun.sh $<TARGET_FILE:MirodosKernel.elf> workdir -D Userspace/hello/hello
    DEPENDS MirodosKernel.elf
    COMMENT "Running Mirodos in QEMU"
)
