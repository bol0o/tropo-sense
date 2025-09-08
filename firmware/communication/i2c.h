/**
 * @file i2c.h
 * @brief Low-level I²C/TWI driver for AVR.
 *
 * Provides blocking primitives for start, stop, read and write.
 * Higher-level device drivers (sensors, EEPROM, etc.) should
 * build on top of this interface.
 */

#ifndef I2C_H
#define I2C_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize I²C (TWI) hardware.
 *
 * Prescaler set to 1, bit rate default corresponds to ~100 kHz at F_CPU=16 MHz.
 */
void I2C_init(void);

/**
 * @brief Send START condition.
 */
void I2C_start(void);

/**
 * @brief Send STOP condition.
 */
void I2C_stop(void);

/**
 * @brief Write one byte to I²C.
 * @param data Byte to transmit.
 */
void I2C_write(uint8_t data);

/**
 * @brief Send START and then address + R/W bit.
 * @param address 7-bit device address.
 * @param read 0 = write, 1 = read.
 */
void I2C_start_with_address(uint8_t address, uint8_t read);

/**
 * @brief Read one byte and send ACK.
 * @return Received byte.
 */
uint8_t I2C_read_ack(void);

/**
 * @brief Read one byte and send NACK (end of transfer).
 * @return Received byte.
 */
uint8_t I2C_read_nack(void);

#ifdef __cplusplus
}
#endif

#endif /* I2C_H */