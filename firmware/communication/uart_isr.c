/**
 * @file uart_isr.c
 * @brief AVR UART driver with RX interrupt and ring buffer.
 *
 * RX handled in ISR, data stored in uart_rx ring buffer.
 * TX functions are blocking.
 */

#include "uart_isr.h"

/* Global ring buffer instance */
uart_rx_ring_t uart_rx = { .head=0, .tail=0, .err_fe=0, .err_dor=0, .err_upe=0 };

void UART_init_ISR(unsigned int ubrr) {
    UCSR0A |= (1 << U2X0); // enable double speed mode

    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;

    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0); // RX/TX + RX interrupt
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);             // 8N1
}

/* RX interrupt service routine */
ISR(USART_RX_vect) {
    uint8_t status = UCSR0A;
    uint8_t data   = UDR0;

    /* Record error flags */
    if (status & (1<<FE0))  uart_rx.err_fe++;
    if (status & (1<<DOR0)) uart_rx.err_dor++;
    if (status & (1<<UPE0)) uart_rx.err_upe++;

    uint8_t next = (uart_rx.head + 1) % RX_BUF_SZ;
    if (next != uart_rx.tail) { // not full
        uart_rx.buf[uart_rx.head] = data;
        uart_rx.head = next;
    } else {
        // overflow: could increment separate counter if needed
    }
}

uint8_t UART_data_available(void) {
    return (uart_rx.head != uart_rx.tail);
}

int16_t UART_receive(void) {
    if (uart_rx.head == uart_rx.tail) return -1;

    uint8_t c = uart_rx.buf[uart_rx.tail];
    uart_rx.tail = (uart_rx.tail + 1) % RX_BUF_SZ;
    return c;
}

void UART_send(char c) {
    while(!(UCSR0A & (1<<UDRE0))); // wait until TX buffer empty
    UDR0 = (uint8_t)c;
}

void UART_send_string(const char* s) {
    while(*s) UART_send(*s++);
}
