#include "ds18b20.h"

int16_t ds18b20_readTemperature() {
    if (!one_wire_reset()) return -1000;

    one_wire_writeByte(0xCC);  // Skip ROM
    one_wire_writeByte(0x44);  // Start temperature conversion

    _delay_ms(750);

    if (!one_wire_reset()) return -1000;

    one_wire_writeByte(0xCC);
    one_wire_writeByte(0xBE);  // Read Scratchpad

    uint8_t lsb = one_wire_readByte();
    uint8_t msb = one_wire_readByte();

    return ((msb << 8) | lsb);
}