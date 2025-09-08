/**
 * @file ds3231.c
 * @brief DS3231 RTC driver implementation using blocking I2C API.
 *
 * Dependencies:
 *  - i2c.h : must provide
 *      void I2C_start_with_address(uint8_t addr, uint8_t read);
 *      void I2C_write(uint8_t data);
 *      uint8_t I2C_read_ack(void);
 *      uint8_t I2C_read_nack(void);
 *      void I2C_stop(void);
 */

#include "ds3231.h"
#include "../communication/i2c.h"

uint8_t dec_to_bcd(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

uint8_t bcd_to_dec(uint8_t val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}

uint8_t DS3231_get_seconds(void) {
    I2C_start_with_address(DS3231_I2C_ADDRESS, 0);
    I2C_write(0x00);  // Seconds register
    I2C_start_with_address(DS3231_I2C_ADDRESS, 1);
    uint8_t bcd_sec = I2C_read_nack();
    I2C_stop();
    return bcd_to_dec(bcd_sec);
}

void DS3231_set_alarm1_next_15s(void) {
    uint8_t current_sec = DS3231_get_seconds();
    uint8_t next_sec = ((current_sec / 15) + 1) * 15;
    if (next_sec >= 60) next_sec = 0;

    // Configure Alarm1: match seconds only (A1M1=0, A1M2..4=1)
    I2C_start_with_address(DS3231_I2C_ADDRESS, 0);
    I2C_write(0x07);  // Alarm1 register start

    I2C_write(dec_to_bcd(next_sec));  // Seconds
    I2C_write(0x80);                  // Minutes (don't care)
    I2C_write(0x80);                  // Hours (don't care)
    I2C_write(0x80);                  // Day/date (don't care)
    I2C_stop();

    // Enable Alarm1 interrupt
    I2C_start_with_address(DS3231_I2C_ADDRESS, 0);
    I2C_write(0x0E);       // Control register
    I2C_write(0b00000101); // INTCN=1, A1IE=1
    I2C_stop();
}

void DS3231_clear_alarm1_flag(void) {
    I2C_start_with_address(DS3231_I2C_ADDRESS, 0);
    I2C_write(0x0F);  // Status register
    I2C_write(0x00);  // Clear A1F and A2F
    I2C_stop();
}
