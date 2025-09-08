/**
 * @file ds3231.h
 * @brief Minimal DS3231 RTC driver (I2C).
 *
 * Provides helper functions for BCD conversion and basic access
 * to the DS3231 real-time clock:
 *  - Reading current seconds
 *  - Setting Alarm1 to the next 15-second mark
 *  - Clearing Alarm1 flag
 */

#ifndef DS3231_H
#define DS3231_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief DS3231 I2C address*/
#ifndef DS3231_I2C_ADDRESS
#define DS3231_I2C_ADDRESS 0x68
#endif

/**
 * @brief Convert decimal value to BCD format.
 * @param val Decimal value (0–99).
 * @return Value encoded in BCD.
 */
uint8_t dec_to_bcd(uint8_t val);

/**
 * @brief Convert BCD value to decimal.
 * @param val BCD-encoded value.
 * @return Decimal value.
 */
uint8_t bcd_to_dec(uint8_t val);

/**
 * @brief Read current seconds from DS3231 (0–59).
 * @return Current second in decimal.
 */
uint8_t DS3231_get_seconds(void);

/**
 * @brief Configure Alarm1 to trigger at the next 15-second boundary.
 *
 * Example: if current time = 23 s → alarm set for 30 s.
 * If current time = 59 s → alarm set for 0 s of the next minute.
 */
void DS3231_set_alarm1_next_15s(void);

/**
 * @brief Clear Alarm1 flag in DS3231 status register.
 */
void DS3231_clear_alarm1_flag(void);

#ifdef __cplusplus
}
#endif

#endif /* DS3231_H */
