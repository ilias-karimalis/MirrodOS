add_executable(hello hello.c)

# Link MirrodosCore so `hello` gets the core library and headers
target_link_libraries(hello PRIVATE MirrodosCore)

# Ensure userspace executable is built as freestanding and with the same
# ABI/flags expected by the core library.
target_compile_options(hello PRIVATE -Wall -Werror -mcmodel=medany -ffreestanding -nostdlib -fno-exceptions -fno-stack-protector)
target_link_options(hello PRIVATE -nostdlib -ffreestanding)
