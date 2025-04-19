#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdio.h>
#include "uart.h"
#include "i2c.h"
#include "ds18b20.h"
#include "bme280.h"
#include "ds3231.h"

volatile uint8_t wake_flag = 0;

ISR(INT0_vect) {
    // Set flag and return; interrupts auto-disabled during ISR
    wake_flag = 1;
}

void setup_interrupt() {
    EIMSK |= (1 << INT0);      // Enable external interrupt INT0
    EICRA |= (1 << ISC01);     // ISC01 = 1, ISC00 = 0 => falling edge
    EICRA &= ~(1 << ISC00);
}

void setup() {
    UART_init(MYUBRR);
    UART_init(MYUBRR);
    bme280_init();
    I2C_init();

    setup_interrupt();

    // Clear any lingering alarm flags
    DS3231_clear_alarm1_flag();

    // Set first alarm
    DS3231_set_alarm1_next_15s();
    UART_send_string("First alarm set\r\n");
}

int main(void) {
    setup();

    sei(); // Enable global interrupts

    while (1) {
        if (wake_flag) {
            wake_flag = 0;

            UART_send_string("Woke up from alarm!\r\n");

            int16_t ground_temp = ds18b20_readTemperature();
            float air_temp = bme280_readTemperature();
            float pressure = bme280_readPressure();
            float humidity = bme280_readHumidity();

            UART_send_number(ground_temp);
            UART_send_float(air_temp);
            UART_send_float(pressure);
            UART_send_float(humidity);

            // Clear alarm flag
            UART_send_string("Clearing alarm flag...\r\n");
            DS3231_clear_alarm1_flag();

            // Set next alarm
            DS3231_set_alarm1_next_15s();
            UART_send_string("Next alarm set for 15s\r\n");
        }

        // Sleep until interrupt
        set_sleep_mode(SLEEP_MODE_IDLE);
        sleep_enable();
        sleep_cpu();
        sleep_disable();  // Continue after wake
    }
}