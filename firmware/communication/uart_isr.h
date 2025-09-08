#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#define MYUBRR (F_CPU / 8 / BAUD - 1)

#define RX_BUF_SZ 256

typedef struct {
    volatile uint8_t buf[RX_BUF_SZ];
    volatile uint8_t head;
    volatile uint8_t tail;
    volatile uint16_t err_fe;   // frame error
    volatile uint16_t err_dor;  // data overrun
    volatile uint16_t err_upe;  // parity
} uart_rx_ring_t;

extern uart_rx_ring_t uart_rx;

void UART_init_ISR(unsigned int ubrr);
uint8_t UART_data_available(void);
int16_t UART_receive(void);      // -1 gdy pusto
void UART_send(char c);
void UART_send_string(const char* s);