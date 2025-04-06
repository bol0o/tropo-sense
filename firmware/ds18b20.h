#ifndef DS18B20_H
#define DS18B20_H

#include <stdint.h>
#include "one_wire.h"

int16_t ds18b20_readTemperature();

#endif // DS18B20_H