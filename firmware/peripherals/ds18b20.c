/**
 * @file ds18b20.c
 * @brief DS18B20 1-Wire temperature sensor implementation.
 *
 * Uses low-level 1-Wire functions from one_wire.h.
 * Blocking read, maximum conversion time = 750 ms.
 */

#include "ds18b20.h"
#include <util/delay.h> // for _delay_ms

int16_t ds18b20_readTemperature(void) {
    // Reset bus and check presence
    if (!one_wire_reset()) return -1000;

    // Skip ROM (single device assumed), start conversion
    one_wire_writeByte(0xCC);
    one_wire_writeByte(0x44);

    _delay_ms(750); // wait for conversion (12-bit resolution)

    // Reset and prepare to read scratchpad
    if (!one_wire_reset()) return -1000;

    one_wire_writeByte(0xCC);  // Skip ROM
    one_wire_writeByte(0xBE);  // Read Scratchpad

    uint8_t lsb = one_wire_readByte();
    uint8_t msb = one_wire_readByte();

    return (int16_t)((msb << 8) | lsb); // raw 1/16 °C units
}