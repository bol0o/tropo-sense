#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"
#include "i2c.h"
#include <stdio.h>

#define INA219_ADDRESS 0x40

uint16_t INA219_read_register(uint8_t reg) {
    uint8_t high, low;

    I2C_start_with_address(INA219_ADDRESS, 0); // Write mode
    I2C_write(reg);
    I2C_start_with_address(INA219_ADDRESS, 1); // Read mode
    high = I2C_read_ack();
    low = I2C_read_nack();
    I2C_stop();

    return (high << 8) | low;
}

float INA219_read_bus_voltage() {
    uint16_t raw = INA219_read_register(0x02);
    raw >>= 3;
    return raw * 0.004; // in volts
}

float INA219_read_shunt_voltage() {
    int16_t raw = (int16_t)INA219_read_register(0x01);
    return raw * 0.00001; // in volts
}

int main(void) {
    // Init UART
    UART_init(MYUBRR);

    char buffer[128];
    
    while (1) {
        float bus_v = INA219_read_bus_voltage();
        float shunt_v = INA219_read_shunt_voltage();  // Shunt voltage in volts
        int bus_mv = (int)(bus_v * 1000);  // Bus voltage in millivolts
        int shunt_uv = (int)(shunt_v * 1000000);  // Shunt voltage in microvolts

        // Calculate current (in amps), then convert to milliamps
        float current = shunt_v / 0.1;  // Current in amps
        float current_ma = current * 1000;  // Convert to milliamps
        int current_ma_int = (int)current_ma;  // Integer part
        int current_ma_frac = (int)((current_ma - current_ma_int) * 1000);  // Fractional part in milliamps

        // Print values in a readable format
        snprintf(buffer, sizeof(buffer), 
                "Bus: %d.%03dV, Shunt: %d uV, Current: %d.%03d mA\r\n",
                bus_mv / 1000, bus_mv % 1000, shunt_uv, current_ma_int, current_ma_frac);

        UART_send_string(buffer);

        _delay_ms(50);
    }

    return 0;
}