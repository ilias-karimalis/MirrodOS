#pragma once

/// Initializes the uart library with the given base address.
void
uart_initialize(void* base_address);

/// Sends a character over the uart.
void
uart_put_char(char c);

/// Receives a character over the uart.
char
uart_get_char();
