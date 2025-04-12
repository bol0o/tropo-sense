#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include "uart.h"
#include "i2c.h"
#include "ds3231.h"

#define LED_PIN PB0

volatile uint8_t wake_flag = 0;

ISR(INT0_vect) {
    // Set flag and return; interrupts auto-disabled during ISR
    wake_flag = 1;
}

void setup_interrupt() {
    // INT0 on falling edge
    MCUCR |= (1 << ISC01);
    MCUCR &= ~(1 << ISC00);

    // Enable INT0
    GICR |= (1 << INT0);
}

void setup() {
    DDRB |= (1 << LED_PIN);     // LED as output
    PORTB &= ~(1 << LED_PIN);   // LED off

    UART_init(MYUBRR);
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

            // Turn on LED
            PORTB |= (1 << LED_PIN);
            _delay_ms(3000);
            PORTB &= ~(1 << LED_PIN);

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