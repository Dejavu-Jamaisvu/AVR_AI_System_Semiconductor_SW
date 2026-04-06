//my_uart.h
#ifndef MY_UART_H
#define MY_UART_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void uart_init(void);
int uart_putchar(char c, FILE *stream);
FILE* uart_get_stdout(void);

#ifdef __cplusplus
}
#endif

#endif