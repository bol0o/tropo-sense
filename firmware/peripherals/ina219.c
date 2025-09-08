#include <avr/io.h>
#include <util/delay.h>
#include "i2c.h"
#include <stdio.h>

#define INA219_ADDRESS 0x40

uint16_t INA219_read_register(uint8_t reg) {
    uint8_t high, low;

    I2C_start_with_address(INA219_ADDRESS, 0); // Write mode
    I2C_write(reg);
    I2C_start_with_address(INA219_ADDRESS, 1); // Read mode
    high = I2C_read_ack();
    low = I2C_read_nack();
    I2C_stop();

    return (high << 8) | low;
}

float INA219_read_bus_voltage() {
    uint16_t raw = INA219_read_register(0x02);
    raw >>= 3;

    return raw * 0.004; // in volts
}

float INA219_read_shunt_voltage() {
    int16_t raw = (int16_t) INA219_read_register(0x01);

    return raw * 0.00001; // in volts
}