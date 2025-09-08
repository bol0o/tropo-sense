/**
 * @file ds18b20.h
 * @brief Minimal driver for DS18B20 1-Wire temperature sensor.
 *
 * Provides a blocking read function that triggers a conversion,
 * waits for it to complete, and reads the temperature raw value.
 */

#ifndef DS18B20_H
#define DS18B20_H

#include <stdint.h>
#include "../communication/one_wire.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Read temperature from DS18B20.
 *
 * This function:
 *  - Issues a reset/presence pulse
 *  - Sends SKIP ROM (0xCC) and CONVERT T (0x44)
 *  - Waits up to 750 ms for conversion (12-bit resolution)
 *  - Sends SKIP ROM (0xCC) and READ SCRATCHPAD (0xBE)
 *  - Reads 2 bytes (temperature LSB and MSB)
 *
 * @return Raw signed 16-bit temperature value (1/16 °C units).
 *         Example: return value 0x00A0 → 10.0 °C
 *         Returns -1000 on bus error (no presence pulse).
 */
int16_t ds18b20_readTemperature(void);

#ifdef __cplusplus
}
#endif

#endif /* DS18B20_H */