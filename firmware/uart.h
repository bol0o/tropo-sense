#ifndef UART_H
#define UART_H

#include <avr/io.h>
#include "config.h"

// Define CPU clock and baud rate
#define F_CPU 4000000UL  // 4 MHz CPU Clock
#define BAUD 9600
#define MYUBRR (F_CPU / 16 / BAUD - 1)

void UART_init(unsigned int ubrr);
void UART_send(char data);
char UART_receive();
void UART_send_string(const char* str);
void UART_send_float(float value);
void UART_send_number(uint32_t value);

#endif