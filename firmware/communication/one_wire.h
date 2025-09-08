/**
 * @file one_wire.h
 * @brief Low-level bit-banging driver for 1-Wire bus (AVR).
 *
 * Provides basic reset, bit and byte read/write functions.
 * Uses busy-wait delays to generate timing.
 */

#ifndef ONE_WIRE_H
#define ONE_WIRE_H

#include <avr/io.h>
#include <util/delay.h>

#ifdef __cplusplus
extern "C" {
#endif

/* --- Hardware configuration ---
 * Define these macros in your project before including this header:
 *   ONE_WIRE_DDR   : DDRx register of the pin
 *   ONE_WIRE_PORT  : PORTx register of the pin
 *   ONE_WIRE_PIN_REG : PINx register of the pin
 *   ONE_WIRE_PIN   : bit index of the pin
 * 
 * If not defined, 1-Wire will use port PB0.
 */

#ifndef ONE_WIRE_PIN
#define ONE_WIRE_PIN PB0
#endif

#ifndef ONE_WIRE_PORT
#define ONE_WIRE_PORT PORTB
#endif

#ifndef ONE_WIRE_DDR
#define ONE_WIRE_DDR DDRB
#endif

#ifndef ONE_WIRE_PIN_REG
#define ONE_WIRE_PIN_REG PINB
#endif

/**
 * @brief Configure the 1-Wire pin as output.
 */
void one_wire_setOutput(void);

/**
 * @brief Configure the 1-Wire pin as input (high-Z).
 */
void one_wire_setInput(void);

/**
 * @brief Drive the 1-Wire line low.
 */
void one_wire_pullLow(void);

/**
 * @brief Release the 1-Wire line (pull-up active).
 */
void one_wire_release(void);

/**
 * @brief Read current state of the 1-Wire line.
 * @return 0 if low, non-zero if high.
 */
uint8_t one_wire_readPin(void);

/**
 * @brief Send reset pulse and detect presence.
 * @return 1 if presence detected, 0 otherwise.
 */
uint8_t one_wire_reset(void);

/**
 * @brief Write one bit to the bus.
 * @param bit 0 or 1
 */
void one_wire_writeBit(uint8_t bit);

/**
 * @brief Read one bit from the bus.
 * @return 0 or 1
 */
uint8_t one_wire_readBit(void);

/**
 * @brief Write one byte (LSB first).
 */
void one_wire_writeByte(uint8_t data);

/**
 * @brief Read one byte (LSB first).
 */
uint8_t one_wire_readByte(void);

#ifdef __cplusplus
}
#endif

#endif /* ONE_WIRE_H */
