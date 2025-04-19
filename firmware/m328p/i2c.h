#ifndef I2C_H
#define I2C_H

#include <avr/io.h>

// Initializes I2C interface
void I2C_init();

// Sends a start condition
void I2C_start();

// Sends a stop condition
void I2C_stop();

// Writes a byte to I2C
void I2C_write(uint8_t data);

// Sends a start condition followed by device address and R/W mode (0 for write, 1 for read)
void I2C_start_with_address(uint8_t address, uint8_t read);

// Reads a byte with ACK
uint8_t I2C_read_ack();

// Reads a byte with NACK
uint8_t I2C_read_nack();

#endif // I2C_H