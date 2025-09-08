#ifndef F_CPU
    #define F_CPU 16000000UL
#endif

#ifndef BAUD
    #define BAUD 115200
#endif

#ifndef ONE_WIRE_PIN
    #define ONE_WIRE_PIN PB0
#endif

#ifndef ONE_WIRE_PORT
    #define ONE_WIRE_PORT PORTB
#endif

#ifndef ONE_WIRE_DDR
    #define ONE_WIRE_DDR DDRB
#endif

#ifndef ONE_WIRE_PIN_REG
    #define ONE_WIRE_PIN_REG PINB
#endif

#ifndef BME280_ADDR
    #define BME280_ADDR 0x76
#endif