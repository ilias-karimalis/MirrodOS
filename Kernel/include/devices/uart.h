#pragma once

#include <types/number.h>

struct uart_driver
{
        volatile u8* base;
        void (*handle_interrupt)(struct uart_driver* driver);
};

void
uart_driver_init(struct uart_driver* driver, void* base);

void
uart_driver_putchar(struct uart_driver* driver, char c);

void
uart_driver_handle_interrupt(struct uart_driver* driver);