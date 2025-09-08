#ifndef INA219_H
#define INA219_H

uint16_t INA219_read_register(uint8_t reg);
float INA219_read_bus_voltage();
float INA219_read_shunt_voltage();

#endif