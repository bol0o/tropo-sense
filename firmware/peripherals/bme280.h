/**
 * @file bme280.h
 * @brief Minimal BME280 driver (I2C): init + temperature/pressure/humidity.
 *
 * Public API is platform-agnostic; low-level I2C lives in the .c file.
 */

#ifndef BME280_H
#define BME280_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief BME280 I2C address*/
#ifndef BME280_I2C_ADDRESS
#define BME280_I2C_ADDRSS 0x76
#endif

/**  Calibration registers (per datasheet) */
#define BME280_REGISTER_DIG_T1              0x88
#define BME280_REGISTER_DIG_T2              0x8A
#define BME280_REGISTER_DIG_T3              0x8C

#define BME280_REGISTER_DIG_P1              0x8E
#define BME280_REGISTER_DIG_P2              0x90
#define BME280_REGISTER_DIG_P3              0x92
#define BME280_REGISTER_DIG_P4              0x94
#define BME280_REGISTER_DIG_P5              0x96
#define BME280_REGISTER_DIG_P6              0x98
#define BME280_REGISTER_DIG_P7              0x9A
#define BME280_REGISTER_DIG_P8              0x9C
#define BME280_REGISTER_DIG_P9              0x9E

#define BME280_REGISTER_DIG_H1              0xA1
#define BME280_REGISTER_DIG_H2              0xE1
#define BME280_REGISTER_DIG_H3              0xE3
#define BME280_REGISTER_DIG_H4              0xE4
#define BME280_REGISTER_DIG_H5              0xE5
#define BME280_REGISTER_DIG_H6              0xE7

/* Reset */
#define BME280_REGISTER_SOFTRESET           0xE0
#define BME280_REGISTER_POWERONRESET        0xB6

/* Configuration / control */
#define BME280_REGISTER_CONTROLHUMID        0xF2
#define BME280_REGISTER_STATUS              0xF3
#define BME280_REGISTER_CONTROL             0xF4
#define BME280_REGISTER_CONFIG              0xF5

/* Data registers (MSB..LSB) */
#define BME280_REGISTER_PRESSUREDATA        0xF7
#define BME280_REGISTER_TEMPDATA            0xFA
#define BME280_REGISTER_HUMIDDATA           0xFD

/* Public API */
/**
 * @brief Initialize BME280: soft reset, basic oversampling and mode,
 *        then read calibration coefficients into internal state.
 */
void bme280_init(void);

/**
 * @brief Read temperature in degrees Celsius.
 */
float bme280_readTemperature(void);

/**
 * @brief Read pressure in hPa.
 */
float bme280_readPressure(void);

/**
 * @brief Read relative humidity in %RH.
 */
float bme280_readHumidity(void);

/* Low-level helpers (exposed if you need them elsewhere) */
uint8_t  bme280_read1Byte(uint8_t reg);
uint16_t bme280_read2Byte(uint8_t reg);
uint32_t bme280_read3Byte(uint8_t reg);
uint16_t read16_LE(uint8_t reg);
int16_t  readS16(uint8_t reg);
int16_t  readS16_LE(uint8_t reg);
void     bme280_readCoefficients(void);

#ifdef __cplusplus
}
#endif

#endif /* BME280_H */
