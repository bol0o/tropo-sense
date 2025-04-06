#ifndef F_CPU
    #define F_CPU 4000000UL  // Default if not defined
#endif

#ifndef BAUD
    #define BAUD 9600  // Default baud rate
#endif

#ifndef ONE_WIRE_PIN
    #define ONE_WIRE_PIN PB0 // Default BME280 address
#endif

#ifndef ONE_WIRE_PORT
    #define ONE_WIRE_PORT PORTB  // Default BME280 address
#endif

#ifndef ONE_WIRE_DDR
    #define ONE_WIRE_DDR DDRB  // Default BME280 address
#endif

#ifndef ONE_WIRE_PIN_REG
    #define ONE_WIRE_PIN_REG PINB  // Default BME280 address
#endif

#ifndef BME280_ADDR
    #define BME280_ADDR 0x76  // Default BME280 address
#endif