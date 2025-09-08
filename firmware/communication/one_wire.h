#ifndef ONE_WIRE_H
#define ONE_WIRE_H

#include <avr/io.h>
#include <util/delay.h>

void one_wire_setOutput();
void one_wire_setInput();
void one_wire_pullLow();
void one_wire_release();
uint8_t one_wire_readPin();
uint8_t one_wire_reset();
void one_wire_writeBit(uint8_t bit);
uint8_t one_wire_readBit();
void one_wire_writeByte(uint8_t data);
uint8_t one_wire_readByte();

#endif // ONE_WIRE_H