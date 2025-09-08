/**
 * @file one_wire.c
 * @brief 1-Wire bus implementation for AVR.
 *
 * Bit-banging with busy-wait delays. Timing follows DS18B20 datasheet.
 */

#include "one_wire.h"

void one_wire_setOutput(void) {
    ONE_WIRE_DDR |= (1 << ONE_WIRE_PIN);
}

void one_wire_setInput(void) {
    ONE_WIRE_DDR &= ~(1 << ONE_WIRE_PIN);
}

void one_wire_pullLow(void) {
    ONE_WIRE_PORT &= ~(1 << ONE_WIRE_PIN);
}

void one_wire_release(void) {
    ONE_WIRE_PORT |= (1 << ONE_WIRE_PIN);
}

uint8_t one_wire_readPin(void) {
    return (ONE_WIRE_PIN_REG & (1 << ONE_WIRE_PIN));
}

uint8_t one_wire_reset(void) {
    one_wire_setOutput();
    one_wire_pullLow();
    _delay_us(480);          // reset pulse
    one_wire_release();
    one_wire_setInput();

    _delay_us(70);           // wait for presence
    uint8_t presence = !one_wire_readPin();
    _delay_us(410);          // finish timeslot

    return presence;
}

void one_wire_writeBit(uint8_t bit) {
    one_wire_setOutput();
    one_wire_pullLow();
    _delay_us(2);

    if (bit) one_wire_release(); // release early for '1'

    _delay_us(60);
    one_wire_release();
}

uint8_t one_wire_readBit(void) {
    one_wire_setOutput();
    one_wire_pullLow();
    _delay_us(2);

    one_wire_release();
    one_wire_setInput();
    _delay_us(10);

    uint8_t bit = one_wire_readPin();
    _delay_us(50);

    return bit;
}

void one_wire_writeByte(uint8_t data) {
    for (uint8_t i = 0; i < 8; i++) {
        one_wire_writeBit(data & 0x01);
        data >>= 1;
    }
}

uint8_t one_wire_readByte(void) {
    uint8_t data = 0;
    for (uint8_t i = 0; i < 8; i++) {
        data >>= 1;
        if (one_wire_readBit()) data |= 0x80;
    }
    return data;
}
