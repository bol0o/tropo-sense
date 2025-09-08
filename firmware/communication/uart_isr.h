/**
 * @file uart_isr.h
 * @brief UART driver with RX interrupt and ring buffer (AVR).
 *
 * Provides interrupt-driven RX (256-byte ring buffer) and
 * blocking TX. Error counters are kept in the ring buffer struct.
 */

#ifndef UART_ISR_H
#define UART_ISR_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Default UBRR formula for async double speed (U2X=1). */
#define MYUBRR (F_CPU / 8 / BAUD - 1)

/** RX buffer size in bytes. */
#define RX_BUF_SZ 256

/** RX ring buffer structure with error counters. */
typedef struct {
    volatile uint8_t buf[RX_BUF_SZ];
    volatile uint8_t head;
    volatile uint8_t tail;
    volatile uint16_t err_fe;   /**< Frame error counter */
    volatile uint16_t err_dor;  /**< Data overrun counter */
    volatile uint16_t err_upe;  /**< Parity error counter */
} uart_rx_ring_t;

/** Global RX buffer instance. */
extern uart_rx_ring_t uart_rx;

/**
 * @brief Initialize UART with RX interrupt enabled.
 * @param ubrr Value for UBRR0 register.
 */
void UART_init_ISR(unsigned int ubrr);

/**
 * @brief Check if RX buffer has data.
 * @return Non-zero if data available.
 */
uint8_t UART_data_available(void);

/**
 * @brief Get one byte from RX buffer.
 * @return Received byte, or -1 if empty.
 */
int16_t UART_receive(void);

/**
 * @brief Send one character (blocking).
 */
void UART_send(char c);

/**
 * @brief Send null-terminated string.
 */
void UART_send_string(const char* s);

#ifdef __cplusplus
}
#endif

#endif /* UART_ISR_H */
