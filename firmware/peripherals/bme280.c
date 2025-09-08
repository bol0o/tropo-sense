/**
 * @file bme280.c
 * @brief BME280 driver implementation using a blocking I2C API.
 *
 * Expected I2C primitives (from your platform layer):
 *   void    I2C_start_with_address(uint8_t addr, uint8_t read);
 *   void    I2C_write(uint8_t data);
 *   uint8_t I2C_read_ack(void);
 *   uint8_t I2C_read_nack(void);
 *   void    I2C_stop(void);
 */

#include "bme280.h"
#include "../communication/i2c.h"
#include <util/delay.h>

/* Calibration coefficients */
uint16_t dig_T1, dig_P1;
int16_t  dig_T2, dig_T3, dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
uint8_t  dig_H1, dig_H3;
int16_t  dig_H2;
int16_t  dig_H4, dig_H5; /* stored as signed 12-bit fields after packing */
int8_t   dig_H6;
int32_t  t_fine;

/* --- Raw register access --- */

uint8_t bme280_read1Byte(uint8_t reg) {
    I2C_start_with_address(BME280_I2C_ADDRSS, 0);
    I2C_write(reg);
    I2C_stop();

    I2C_start_with_address(BME280_I2C_ADDRSS, 1);
    uint8_t v = I2C_read_nack();
    I2C_stop();
    return v;
}

uint16_t bme280_read2Byte(uint8_t reg) {
    I2C_start_with_address(BME280_I2C_ADDRSS, 0);
    I2C_write(reg);
    I2C_stop();

    I2C_start_with_address(BME280_I2C_ADDRSS, 1);
    uint16_t hi = I2C_read_ack();
    uint16_t lo = I2C_read_nack();
    I2C_stop();
    return (uint16_t)((hi << 8) | lo);
}

uint32_t bme280_read3Byte(uint8_t reg) {
    I2C_start_with_address(BME280_I2C_ADDRSS, 0);
    I2C_write(reg);
    I2C_stop();

    I2C_start_with_address(BME280_I2C_ADDRSS, 1);
    uint32_t b2 = I2C_read_ack();   /* MSB */
    uint32_t b1 = I2C_read_ack();   /* LSB of MSB+LSB pair */
    uint32_t b0 = I2C_read_nack();  /* XLSB */
    I2C_stop();
    return (b2 << 16) | (b1 << 8) | b0;
}

/* Helpers for endian/sign handling matching datasheet notation */
uint16_t read16_LE(uint8_t reg) {
    uint16_t v = bme280_read2Byte(reg);
    return (uint16_t)((v >> 8) | (v << 8));
}

int16_t readS16(uint8_t reg) {
    return (int16_t)bme280_read2Byte(reg);
}

int16_t readS16_LE(uint8_t reg) {
    return (int16_t)read16_LE(reg);
}

/* --- Calibration readout (must be called during init) --- */
void bme280_readCoefficients(void) {
    /* Temperature */
    dig_T1 = read16_LE(BME280_REGISTER_DIG_T1);
    dig_T2 = readS16_LE(BME280_REGISTER_DIG_T2);
    dig_T3 = readS16_LE(BME280_REGISTER_DIG_T3);

    /* Pressure */
    dig_P1 = read16_LE(BME280_REGISTER_DIG_P1);
    dig_P2 = readS16_LE(BME280_REGISTER_DIG_P2);
    dig_P3 = readS16_LE(BME280_REGISTER_DIG_P3);
    dig_P4 = readS16_LE(BME280_REGISTER_DIG_P4);
    dig_P5 = readS16_LE(BME280_REGISTER_DIG_P5);
    dig_P6 = readS16_LE(BME280_REGISTER_DIG_P6);
    dig_P7 = readS16_LE(BME280_REGISTER_DIG_P7);
    dig_P8 = readS16_LE(BME280_REGISTER_DIG_P8);
    dig_P9 = readS16_LE(BME280_REGISTER_DIG_P9);

    /* Humidity (note the special H4/H5 packing across E4/E5/E6) */
    dig_H1 = bme280_read1Byte(BME280_REGISTER_DIG_H1);
    dig_H2 = readS16_LE(BME280_REGISTER_DIG_H2);
    dig_H3 = bme280_read1Byte(BME280_REGISTER_DIG_H3);

    uint8_t e4 = bme280_read1Byte(BME280_REGISTER_DIG_H4);
    uint8_t e5 = bme280_read1Byte(BME280_REGISTER_DIG_H5);
    uint8_t e6 = bme280_read1Byte(BME280_REGISTER_DIG_H5 + 1);
    dig_H4 = (int16_t)((e4 << 4) | (e5 & 0x0F));     /* signed 12-bit */
    dig_H5 = (int16_t)((e6 << 4) | (e5 >> 4));       /* signed 12-bit */
    dig_H6 = (int8_t)bme280_read1Byte(BME280_REGISTER_DIG_H6);
}

/* --- Init & measurements --- */
void bme280_init(void) {
    /* Soft reset */
    I2C_start_with_address(BME280_I2C_ADDRSS, 0);
    I2C_write(BME280_REGISTER_SOFTRESET);
    I2C_write(BME280_REGISTER_POWERONRESET);
    I2C_stop();
    _delay_ms(10);

    /* Humidity oversampling = x1 (must write ctrl_hum before ctrl_meas) */
    I2C_start_with_address(BME280_I2C_ADDRSS, 0);
    I2C_write(BME280_REGISTER_CONTROLHUMID);
    I2C_write(0x01); /* osrs_h = 1 */
    I2C_stop();

    /* Config: filter=2 (optional), standby=0.5ms, 3-wire SPI off */
    I2C_start_with_address(BME280_I2C_ADDRSS, 0);
    I2C_write(BME280_REGISTER_CONFIG);
    I2C_write((0x02 << 2) | 0x00); /* filter=2, t_sb=0, spi3w_en=0 */
    I2C_stop();

    /* ctrl_meas: temp x1, press x1, normal mode */
    I2C_start_with_address(BME280_I2C_ADDRSS, 0);
    I2C_write(BME280_REGISTER_CONTROL);
    I2C_write((0x01 << 5) | (0x01 << 2) | 0x03); /* osrs_t=1, osrs_p=1, mode=3 */
    I2C_stop();

    _delay_ms(100);

    /* Load calibration */
    bme280_readCoefficients();
}

float bme280_readTemperature(void) {
    /* Read uncompensated temperature (20-bit) */
    uint32_t adc_T = bme280_read3Byte(BME280_REGISTER_TEMPDATA) >> 4;

    /* Compensation (datasheet §4.2.3) */
    int32_t var1 = ((((int32_t)adc_T >> 3) - ((int32_t)dig_T1 << 1)) * (int32_t)dig_T2) >> 11;
    int32_t var2 = (((((int32_t)adc_T >> 4) - (int32_t)dig_T1) *
                      (((int32_t)adc_T >> 4) - (int32_t)dig_T1)) >> 12) * (int32_t)dig_T3) >> 14;

    t_fine = var1 + var2;
    int32_t T  = (t_fine * 5 + 128) >> 8; /* °C * 100 */
    return (float)T / 100.0f;
}

float bme280_readPressure(void) {
    /* Require t_fine from temperature path */
    (void)bme280_readTemperature();

    int32_t adc_P = (int32_t)(bme280_read3Byte(BME280_REGISTER_PRESSUREDATA) >> 4);

    int64_t var1 = (int64_t)t_fine - 128000;
    int64_t var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1) * (int64_t)dig_P1) >> 33;

    if (var1 == 0) {
        return 0.0f; /* avoid div-by-zero */
    }

    int64_t p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);

    return (float)p / (256.0f * 100.0f); /* Pa/256 → Pa; /100 → hPa */
}

float bme280_readHumidity(void) {
    /* Require t_fine from temperature path */
    (void)bme280_readTemperature();

    int32_t adc_H = (int32_t)bme280_read2Byte(BME280_REGISTER_HUMIDDATA);

    int32_t v_x1_u32r = t_fine - ((int32_t)76800);
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)dig_H4) << 20)
                   - (((int32_t)dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15)
                 * (((((((v_x1_u32r * ((int32_t)dig_H6)) >> 10)
                        * ((v_x1_u32r * ((int32_t)dig_H3)) >> 11) + ((int32_t)32768))) >> 10)
                      + ((int32_t)2097152)) * ((int32_t)dig_H2) + 8192) >> 14));
    v_x1_u32r = v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7)
                              * ((int32_t)dig_H1)) >> 4);
    if (v_x1_u32r < 0) v_x1_u32r = 0;
    if (v_x1_u32r > 419430400) v_x1_u32r = 419430400;

    float h = (float)(v_x1_u32r >> 12) / 1024.0f;
    return h; /* %RH */
}
