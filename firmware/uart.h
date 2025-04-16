#ifndef UART_H
#define UART_H

#include <avr/io.h>
#include "config.h"

#define MYUBRR (F_CPU / 16 / BAUD - 1)

void UART_init(unsigned int ubrr);
uint8_t UART_data_available();
void UART_send(char data);
char UART_receive();
void UART_send_string(const char* str);
void UART_send_float(float value);
void UART_send_number(uint32_t value);

#endif