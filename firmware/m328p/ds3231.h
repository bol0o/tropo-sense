#ifndef DS3231_H
#define DS3231_H

#include <avr/io.h>

uint8_t dec_to_bcd(uint8_t val);

uint8_t bcd_to_dec(uint8_t val);

uint8_t DS3231_get_seconds();

void DS3231_set_alarm1_next_15s();

void DS3231_clear_alarm1_flag();

#endif