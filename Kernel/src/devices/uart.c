#include <devices/uart.h>
#include <fmt/print.h>
#include <types/error.h>

void
uart_driver_init(struct uart_driver* driver, void* base)
{
        driver->base = (volatile u8*)base;
        driver->handle_interrupt = uart_driver_handle_interrupt;
        // Enable "Received Data Available" interrupt (bit 0 of IER)
        driver->base[1] = 0x01;
}

error_t
uart_driver_getchar(struct uart_driver* driver, char* c)
{
        if ((driver->base[5] & 0x01) == 0) {
                return EC_UART_DRIVER_NO_DATA;
        }
        *c = driver->base[0];
        return EC_SUCCESS;
}

void
uart_driver_putchar(struct uart_driver* driver, char c)
{
        *driver->base = c;
}

void
uart_driver_handle_interrupt(struct uart_driver* driver)
{
        /// TODO: In the future, it would probaby be better to be able to register a buffer to store this input and send
        /// it to the correct process, rather than just echoing it back to the UART.
        char c;
        error_t err = uart_driver_getchar(driver, &c);
        if (error_is_ok(err)) {
                switch (c) {
                        case 8: // Backspace
                                uart_driver_putchar(driver, '\b');
                                uart_driver_putchar(driver, ' ');
                                uart_driver_putchar(driver, '\b');
                                break;
                        case 10:
                        case 13:
                                uart_driver_putchar(driver, '\n');
                                break;
                        default:
                                uart_driver_putchar(driver, c);
                                break;
                }
        }
}