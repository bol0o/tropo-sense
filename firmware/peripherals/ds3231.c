#include "ds3231.h"
#include "i2c.h"

#define DS3231_ADDRESS 0x68

uint8_t dec_to_bcd(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

uint8_t bcd_to_dec(uint8_t val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}

uint8_t DS3231_get_seconds() {
    I2C_start_with_address(DS3231_ADDRESS, 0);
    I2C_write(0x00);  // seconds register
    I2C_start_with_address(DS3231_ADDRESS, 1);
    uint8_t bcd_sec = I2C_read_nack();
    I2C_stop();
    return bcd_to_dec(bcd_sec);
}

void DS3231_set_alarm1_next_15s() {
    uint8_t current_sec = DS3231_get_seconds();
    uint8_t next_sec = ((current_sec / 15) + 1) * 15;
    if (next_sec >= 60) next_sec = 0;

    // Set Alarm1 for next_sec, match seconds only (A1M1 = 0, A1M2..4 = 1)
    I2C_start_with_address(DS3231_ADDRESS, 0);
    I2C_write(0x07);  // Alarm1 registers start at 0x07

    I2C_write(dec_to_bcd(next_sec));       // Seconds, A1M1 = 0
    I2C_write(0x80);                       // Minutes, A1M2 = 1 (don't care)
    I2C_write(0x80);                       // Hours, A1M3 = 1 (don't care)
    I2C_write(0x80);                       // Day/date, A1M4 = 1 (don't care)

    I2C_stop();

    // Enable A1 interrupt
    I2C_start_with_address(DS3231_ADDRESS, 0);
    I2C_write(0x0E);  // Control register
    I2C_write(0b00000101);  // INTCN=1, A1IE=1
    I2C_stop();
}

void DS3231_clear_alarm1_flag() {
    I2C_start_with_address(DS3231_ADDRESS, 0);
    I2C_write(0x0F);  // Status register
    I2C_write(0x00);  // Clear both A1F and A2F
    I2C_stop();
}