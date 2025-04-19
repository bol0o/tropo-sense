#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"
#include "i2c.h"
#include "ds18b20.h"
#include "bme280.h"
#include <stdio.h>

int main(void) {
    UART_init(MYUBRR);
    I2C_init();
    bme280_init();

    while (1) {
        int16_t ground_temp = ds18b20_readTemperature();
        float air_temp = bme280_readTemperature();
        float pressure = bme280_readPressure();
        float humidity = bme280_readHumidity();

        UART_send_number(ground_temp);
        UART_send_float(air_temp);
        UART_send_float(pressure);
        UART_send_float(humidity);

        // UART_send_string("Test\r");

        _delay_ms(3000);
    }

    return 0;
}