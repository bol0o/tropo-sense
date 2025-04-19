#include "i2c.h"

void I2C_init() {
    TWSR = 0x00;          // Prescaler set to 1
    TWBR = 32;            // Adjust according to F_CPU
    TWCR = (1 << TWEN);   // Enable TWI
}

void I2C_start() {
    TWCR = (1 << TWSTA) | (1 << TWEN) | (1 << TWINT);
    while (!(TWCR & (1 << TWINT)));  // Wait for completion
}

void I2C_write(uint8_t data) {
    TWDR = data;
    TWCR = (1 << TWEN) | (1 << TWINT);
    while (!(TWCR & (1 << TWINT)));  // Wait for completion
}

void I2C_start_with_address(uint8_t address, uint8_t read) {
    I2C_start();  // Initiate start condition
    I2C_write((address << 1) | read);  // Send device address with R/W bit
}

void I2C_stop() {
    TWCR = (1 << TWSTO) | (1 << TWEN) | (1 << TWINT);
}

uint8_t I2C_read_ack() {
    TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWEA);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

uint8_t I2C_read_nack() {
    TWCR = (1 << TWEN) | (1 << TWINT);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}