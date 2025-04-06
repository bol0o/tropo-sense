#include "uart.h"
#include <stdio.h>

void UART_init(unsigned int ubrr) {
    UBRRH = (unsigned char)(ubrr >> 8);
    UBRRL = (unsigned char)ubrr;
    UCSRB = (1 << RXEN) | (1 << TXEN); // Enable RX and TX
    UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0); // 8-bit, 1 stop bit, no parity
}

void UART_send(char data) {
    while (!(UCSRA & (1 << UDRE))); // Wait until buffer is empty
    UDR = data;
}

char UART_receive() {
    while (!(UCSRA & (1 << RXC))); // Wait until data is received
    return UDR;
}

void UART_send_string(const char* str) {
    while (*str) {
        UART_send(*str++);
    }
}

void UART_send_float(float value) {
    char buffer[20];
    int int_part = (int)value;  // Integer part
    int frac_part = (int)((value - int_part) * 100);

    snprintf(buffer, sizeof(buffer), "%d.%02d", int_part, frac_part);
    UART_send_string(buffer);
}

void UART_send_number(uint32_t value) {
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%lu\n", value);  // Use %lu for uint32_t
    UART_send_string(buffer);
}