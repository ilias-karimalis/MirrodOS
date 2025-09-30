#include <types/number.h>
#include <uart.h>

volatile u8* uart_base_address;

void
uart_initialize(void* base_address)
{
        uart_base_address = (volatile u8*)base_address;
}

void
uart_put_char(char c)
{
        // while ((uart_base_address[0b011] >> 7) != 0);
        *uart_base_address = c;
}

char
uart_get_char()
{
        while ((uart_base_address[0b101] & 0x01) == 0);
        return *uart_base_address;
}