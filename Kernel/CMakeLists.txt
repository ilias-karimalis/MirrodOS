# Enable assertions in debug mode
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(ENABLE_ASSERTIONS)
    message(STATUS "Assertions enabled (Debug build)")
endif()

add_executable(MirodosKernel.elf
    src/devices/device_tree/blob.c
    src/devices/device.c
    src/devices/plic.c
    src/devices/uart.c
    src/devices/virtio/mmio.c
    src/devices/virtio/blk.c
    src/fmt/print.c
    src/kernel_entry.c
    src/kvspace.c
    src/limine/platform_info.c
    src/memory.c
    src/pmm.c
    src/types/error.c
    src/types/bump_alloc.c
    src/types/slab.c
    src/types/str_view.c
    src/uart.c
    src/trap.c
    src/asm/trap.s
    src/riscv.c
)
target_include_directories(MirodosKernel.elf PRIVATE include/)
target_compile_options(MirodosKernel.elf PRIVATE -Wall -Werror -mcmodel=medany -ffreestanding -nostdlib -fno-exceptions -fno-stack-protector)
set(KERNEL_LD_SCRIPT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/kernel_limine.ld")
target_link_options(MirodosKernel.elf PRIVATE  -T${KERNEL_LD_SCRIPT_PATH} -nostdlib -ffreestanding -Wl,-Map=MirodosKernel.elf.map)
