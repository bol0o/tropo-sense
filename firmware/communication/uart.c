#include "uart.h"
#include <util/delay.h>
#include <stdio.h>

void UART_init(unsigned int ubrr) {
    UCSR0A |= (1 << U2X0); // Enable fast mode to allow higher BAUD
    
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0); // Enable RX and TX
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8-bit, 1 stop bit, no parity
}

void UART_send(char data) {
    while (!(UCSR0A & (1 << UDRE0))); // Wait until buffer is empty
    UDR0 = data;
}

uint8_t UART_data_available() {
    return (UCSR0A & (1 << RXC0));  // RXC0 is set when unread data is in UDR0
}

char UART_receive() {
    while (!(UCSR0A & (1 << RXC0))); // Wait until data is received
    
    return UDR0;
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

    snprintf(buffer, sizeof(buffer), "%d.%02d\n", int_part, frac_part);
    UART_send_string(buffer);
}

void UART_send_number(uint32_t value) {
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%lu\n", value);  // Use %lu for uint32_t
    UART_send_string(buffer);
}