#include "one_wire.h"

void one_wire_setOutput() {
    ONE_WIRE_DDR |= (1 << ONE_WIRE_PIN);
}

void one_wire_setInput() {
    ONE_WIRE_DDR &= ~(1 << ONE_WIRE_PIN);
}

void one_wire_pullLow() {
    ONE_WIRE_PORT &= ~(1 << ONE_WIRE_PIN);
}

void one_wire_release() {
    ONE_WIRE_PORT |= (1 << ONE_WIRE_PIN);
}

uint8_t one_wire_readPin() {
    return (ONE_WIRE_PIN_REG & (1 << ONE_WIRE_PIN));
}

uint8_t one_wire_reset() {
    one_wire_setOutput();
    one_wire_pullLow();
    _delay_us(480);
    one_wire_release();
    one_wire_setInput();

    _delay_us(70);
    uint8_t presence = !one_wire_readPin();
    _delay_us(410);

    return presence;
}

void one_wire_writeBit(uint8_t bit) {
    one_wire_setOutput();
    one_wire_pullLow();
    _delay_us(2);

    if (bit) one_wire_release();

    _delay_us(60);
    one_wire_release();
}

uint8_t one_wire_readBit() {
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

uint8_t one_wire_readByte() {
    uint8_t data = 0;
    for (uint8_t i = 0; i < 8; i++) {
        data >>= 1;
        if (one_wire_readBit()) data |= 0x80;
    }
    return data;
}