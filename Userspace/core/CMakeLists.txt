# Enable assertions in debug mode
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(ENABLE_ASSERTIONS)
    message(STATUS "Assertions enabled (Debug build)")
endif()

add_library(MirrodosCore STATIC
    src/crt0.s
)

target_include_directories(MirrodosCore PUBLIC include/)
target_compile_options(MirrodosCore PRIVATE -Wall -Werror -mcmodel=medany -ffreestanding -nostdlib -fno-exceptions -fno-stack-protector)
target_link_options(MirrodosCore PRIVATE -nostdlib -ffreestanding)
