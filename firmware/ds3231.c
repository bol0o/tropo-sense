#include "ds3231.h"
#include "i2c.h"

#define DS3231_ADDRESS 0x68

uint8_t dec_to_bcd(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

uint8_t bcd_to_dec(uint8_t val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}

uint8_t DS3231_get_seconds() {
    I2C_start_with_address(DS3231_ADDRESS, 0);
    I2C_write(0x00);  // seconds register
    I2C_start_with_address(DS3231_ADDRESS, 1);
    uint8_t bcd_sec = I2C_read_nack();
    I2C_stop();
    return bcd_to_dec(bcd_sec);
}

void DS3231_set_alarm1_next_15s() {
    uint8_t current_sec = DS3231_get_seconds();
    uint8_t next_sec = ((current_sec / 15) + 1) * 15;
    if (next_sec >= 60) next_sec = 0;

    // Set Alarm1 for next_sec, match seconds only (A1M1 = 0, A1M2..4 = 1)
    I2C_start_with_address(DS3231_ADDRESS, 0);
    I2C_write(0x07);  // Alarm1 registers start at 0x07

    I2C_write(dec_to_bcd(next_sec));       // Seconds, A1M1 = 0
    I2C_write(0x80);                       // Minutes, A1M2 = 1 (don't care)
    I2C_write(0x80);                       // Hours, A1M3 = 1 (don't care)
    I2C_write(0x80);                       // Day/date, A1M4 = 1 (don't care)

    I2C_stop();

    // Enable A1 interrupt
    I2C_start_with_address(DS3231_ADDRESS, 0);
    I2C_write(0x0E);  // Control register
    I2C_write(0b00000101);  // INTCN=1, A1IE=1
    I2C_stop();
}

void DS3231_clear_alarm1_flag() {
    I2C_start_with_address(DS3231_ADDRESS, 0);
    I2C_write(0x0F);  // Status register
    I2C_write(0x00);  // Clear both A1F and A2F
    I2C_stop();
}

// from main.c
// #include <avr/io.h>
// #include <avr/interrupt.h>
// #include <avr/sleep.h>
// #include <util/delay.h>
// #include "uart.h"
// #include "i2c.h"
// #include "ds3231.h"

// #define LED_PIN PB0

// volatile uint8_t wake_flag = 0;

// ISR(INT0_vect) {
//     // Set flag and return; interrupts auto-disabled during ISR
//     wake_flag = 1;
// }

// void setup_interrupt() {
//     // INT0 on falling edge
//     MCUCR |= (1 << ISC01);
//     MCUCR &= ~(1 << ISC00);

//     // Enable INT0
//     GICR |= (1 << INT0);
// }

// void setup() {
//     DDRB |= (1 << LED_PIN);     // LED as output
//     PORTB &= ~(1 << LED_PIN);   // LED off

//     UART_init(MYUBRR);
//     I2C_init();

//     setup_interrupt();

//     // Clear any lingering alarm flags
//     DS3231_clear_alarm1_flag();

//     // Set first alarm
//     DS3231_set_alarm1_next_15s();
//     UART_send_string("First alarm set\r\n");
// }

// int main(void) {
//     setup();

//     sei(); // Enable global interrupts

//     while (1) {
//         if (wake_flag) {
//             wake_flag = 0;

//             UART_send_string("Woke up from alarm!\r\n");

//             // Turn on LED
//             PORTB |= (1 << LED_PIN);
//             _delay_ms(3000);
//             PORTB &= ~(1 << LED_PIN);

//             // Clear alarm flag
//             UART_send_string("Clearing alarm flag...\r\n");
//             DS3231_clear_alarm1_flag();

//             // Set next alarm
//             DS3231_set_alarm1_next_15s();
//             UART_send_string("Next alarm set for 15s\r\n");
//         }

//         // Sleep until interrupt
//         set_sleep_mode(SLEEP_MODE_IDLE);
//         sleep_enable();
//         sleep_cpu();
//         sleep_disable();  // Continue after wake
//     }
// }