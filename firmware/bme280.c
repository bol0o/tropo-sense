/*
 * BME280 sensor driver
 * Based on original code by Michael Köhler (2017)
 * Distributed under the GPL-3.0 license
 * Modified by Paweł Bolek, 2025
*/

#include "bme280.h"
#include "i2c.h"
#include "uart.h"
#include <util/delay.h>

// BME280 Calibration Coefficients
uint16_t dig_T1, dig_P1;
int16_t dig_T2, dig_T3, dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9, dig_H2, dig_H4, dig_H5;
uint8_t dig_H1, dig_H3;
int8_t dig_H6;
uint32_t t_fine;

// Read one byte from the BME280 register
uint8_t bme280_read1Byte(uint8_t reg) {
    uint8_t value;
    I2C_start_with_address(BME280_ADDR, 0);
    I2C_write(reg);
    I2C_stop();

    I2C_start_with_address(BME280_ADDR, 1);
    value = I2C_read_nack();
    I2C_stop();
    
    return value;
}

// Read two bytes from the BME280 register
uint16_t bme280_read2Byte(uint8_t reg) {
    uint16_t value;
    I2C_start_with_address(BME280_ADDR, 0);
    I2C_write(reg);
    I2C_stop();

    I2C_start_with_address(BME280_ADDR, 1);
    value = I2C_read_ack();
    value <<= 8;
    value |= I2C_read_nack();
    I2C_stop();
    return value;
}

// Read three bytes from the BME280 register
uint32_t bme280_read3Byte(uint8_t reg) {
    uint32_t value;
    I2C_start_with_address(BME280_ADDR, 0);
    I2C_write(reg);
    I2C_stop();

    I2C_start_with_address(BME280_ADDR, 1);
    value = I2C_read_ack();
    value <<= 8;
    value |= I2C_read_ack();
    value <<= 8;
    value |= I2C_read_nack();
    I2C_stop();

    return value;
}

// Read 16-bit value from the register (Little Endian)
uint16_t read16_LE(uint8_t reg) {
    uint16_t temp = bme280_read2Byte(reg);
    return (temp >> 8) | (temp << 8); 
}

// Read signed 16-bit value
int16_t readS16(uint8_t reg) {
    return (int16_t)bme280_read2Byte(reg);
}

// Read signed 16-bit value (Little Endian)
int16_t readS16_LE(uint8_t reg) {
    return (int16_t)read16_LE(reg);
}

// Read the BME280 calibration coefficients
void bme280_readCoefficients() {
    dig_T1 = read16_LE(BME280_REGISTER_DIG_T1);
    dig_T2 = readS16_LE(BME280_REGISTER_DIG_T2);
    dig_T3 = readS16_LE(BME280_REGISTER_DIG_T3);

    dig_P1 = read16_LE(BME280_REGISTER_DIG_P1);
    dig_P2 = readS16_LE(BME280_REGISTER_DIG_P2);
    dig_P3 = readS16_LE(BME280_REGISTER_DIG_P3);
    dig_P4 = readS16_LE(BME280_REGISTER_DIG_P4);
    dig_P5 = readS16_LE(BME280_REGISTER_DIG_P5);
    dig_P6 = readS16_LE(BME280_REGISTER_DIG_P6);
    dig_P7 = readS16_LE(BME280_REGISTER_DIG_P7);
    dig_P8 = readS16_LE(BME280_REGISTER_DIG_P8);
    dig_P9 = readS16_LE(BME280_REGISTER_DIG_P9);

    dig_H1 = bme280_read1Byte(BME280_REGISTER_DIG_H1);
    dig_H2 = readS16_LE(BME280_REGISTER_DIG_H2);
    dig_H3 = bme280_read1Byte(BME280_REGISTER_DIG_H3);
    dig_H4 = (bme280_read1Byte(BME280_REGISTER_DIG_H4) << 4) | (bme280_read1Byte(BME280_REGISTER_DIG_H4 + 1) & 0xF);
    dig_H5 = (bme280_read1Byte(BME280_REGISTER_DIG_H5 + 1) << 4) | (bme280_read1Byte(BME280_REGISTER_DIG_H5) >> 4);
    dig_H6 = (int8_t)bme280_read1Byte(BME280_REGISTER_DIG_H6);
}

// Initialize the BME280 sensor
void bme280_init() {
    // Perform a soft reset
    I2C_start_with_address(BME280_ADDR, 0);
    I2C_write(BME280_REGISTER_SOFTRESET); // Send reset 
    I2C_write(BME280_REGISTER_POWERONRESET); // use the complete power-on procedure
    I2C_stop();
    _delay_ms(10); // Wait for reset to complete
    
    // Configure
    I2C_start_with_address(BME280_ADDR, 0);
    I2C_write(BME280_REGISTER_CONTROLHUMID);
    I2C_write(0x01);

    // Configure filter, standby time, and SPI mode
    I2C_write(BME280_REGISTER_CONFIG);
    I2C_write((0x03 << 5) | (0x00 << 2) | (0x00)); // 011 000 0 0 (refer to BME280 datasheet)

    // Configure pressure, temperature, and sensor mode
    I2C_write(BME280_REGISTER_CONTROL);
    I2C_write((0x01 << 5) | (0x01 << 2) | 0x03); // 011 000 0 0 (refer to BME280 datasheet)

    I2C_stop();
    _delay_ms(100); // Wait for configurations to adjust

    // Read calibration coefficients
    bme280_readCoefficients();
}

// Read temperature from BME280
float bme280_readTemperature() {    
    int32_t var1, var2;

    uint32_t adc_T = bme280_read3Byte(BME280_REGISTER_TEMPDATA);
    adc_T >>= 4;
    
    var1  = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2  = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    
    t_fine = var1 + var2;

    uint32_t T  = (t_fine * 5 + 128) >> 8;
    
    return T / 100.0;
}

// Read pressure from BME280
float bme280_readPressure() {    
    int64_t var1, var2, p;
    
    bme280_readTemperature(); // must be done first to get t_fine
    
    int32_t adc_P = bme280_read3Byte(BME280_REGISTER_PRESSUREDATA);
    adc_P >>= 4;
    
    var1 = ((int64_t)t_fine) - 128000ul;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1*(int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47)+var1)) * ((int64_t)dig_P1) >> 33;
    
    if (var1 == 0) {
        return 0;
    }

    p = 1048576ul - adc_P;
    p = (((p<<31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    return (float)p / (256.0f * 100.0f);
}

// Read humidity from BME280
float bme280_readHumidity() {    
    int32_t v_x1_u32r;
    bme280_readTemperature(); // must be done first to get t_fine
    
    int32_t adc_H = bme280_read2Byte(BME280_REGISTER_HUMIDDATA);
    
    v_x1_u32r = (t_fine - ((int32_t)76800));
    
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) -
                    (((int32_t)dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) *
                 (((((((v_x1_u32r * ((int32_t)dig_H6)) >> 10) *
                      (((v_x1_u32r * ((int32_t)dig_H3)) >> 11) + ((int32_t)32768))) >> 10) +
                    ((int32_t)2097152)) * ((int32_t)dig_H2) + 8192) >> 14));
    
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                               ((int32_t)dig_H1)) >> 4));
    
    v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
    v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
    float h = (v_x1_u32r >> 12);
    return h / 1024;
}
