//my_uart.c
#include "my_uart.h"
#include <avr/io.h>

#define BAUD 9600
#define MYUBRR (F_CPU/16/BAUD-1)

static FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

void uart_init(void) {
    UBRR0H = (uint8_t)(MYUBRR >> 8);
    UBRR0L = (uint8_t)MYUBRR;
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

int uart_putchar(char c, FILE *stream) {
    if (c == '\n') uart_putchar('\r', stream);
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = c;
    return 0;
}

FILE* uart_get_stdout(void) {
    return &uart_output;
}