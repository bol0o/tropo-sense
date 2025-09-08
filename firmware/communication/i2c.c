/**
 * @file i2c.c
 * @brief I²C/TWI implementation for AVR (blocking).
 *
 * Uses hardware TWI registers (TWBR, TWCR, TWDR, etc.).
 */

#include <avr/io.h>
#include "i2c.h"

void I2C_init(void) {
    TWSR = 0x00;          // Prescaler = 1
    TWBR = 32;            // Bit rate (adjust for F_CPU)
    TWCR = (1 << TWEN);   // Enable TWI
}

void I2C_start(void) {
    TWCR = (1 << TWSTA) | (1 << TWEN) | (1 << TWINT);
    while (!(TWCR & (1 << TWINT)));  // Wait for START sent
}

void I2C_write(uint8_t data) {
    TWDR = data;
    TWCR = (1 << TWEN) | (1 << TWINT);
    while (!(TWCR & (1 << TWINT)));  // Wait for transmission complete
}

void I2C_start_with_address(uint8_t address, uint8_t read) {
    I2C_start();
    I2C_write((address << 1) | (read & 0x01));
}

void I2C_stop(void) {
    TWCR = (1 << TWSTO) | (1 << TWEN) | (1 << TWINT);
}

uint8_t I2C_read_ack(void) {
    TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWEA);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

uint8_t I2C_read_nack(void) {
    TWCR = (1 << TWEN) | (1 << TWINT);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}
