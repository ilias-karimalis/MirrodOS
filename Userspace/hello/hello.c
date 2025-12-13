// Hello.c - A simple userspace program that prints "Hello, MirrodOS!" to the console.
#include <syscall.h>

int
main()
{
        // printf("Hello, MirrodOS!\n");
        const char* msg = "Hello, MirrodOS!\n";
        SYSCALL1(SYSCALL_WRITE, msg);
        return 0;
}
