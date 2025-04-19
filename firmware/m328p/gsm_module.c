#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"
#include <string.h>

#define LED_DDR  DDRD
#define LED_PORT PORTD
#define RED_LED    PD5
#define YELLOW_LED PD6
#define GREEN_LED  PD7

void blink_led(uint8_t pin, uint8_t times) {
    for (uint8_t i = 0; i < times; i++) {
        PORTD |= (1 << pin);
        _delay_ms(300);
        PORTD &= ~(1 << pin);
        _delay_ms(100);
    }
}

void wait_for_response_and_flash() {
    char buffer[128];
    uint8_t idx = 0;
    memset(buffer, 0, sizeof(buffer));

    // Wait up to ~5 seconds for response
    for (uint16_t t = 0; t < 5000; t += 10) {
        if (UART_data_available()) {
            blink_led(GREEN_LED, 1);
            return;
        }
        _delay_ms(10);
    }

    // If no response at all
    blink_led(RED_LED, 3);
}

void send_sms() {
    // Send AT+CMGF=0
    UART_send_string("AT+CMGF=0\r\n");
    blink_led(YELLOW_LED, 1);
    wait_for_response_and_flash();

    // Send length-prefixed command to send SMS (PDU mode)
    UART_send_string("AT+CMGS=38\r\n");  // Replace 38 with your PDU message length
    blink_led(YELLOW_LED, 1);
    wait_for_response_and_flash();

    // Send your PDU-encoded message here
    UART_send_string("\r");  // Replace with actual valid PDU
    UART_send(26); // CTRL+Z to send
    blink_led(YELLOW_LED, 1);
    wait_for_response_and_flash();
}


int main(void) {
    // Init UART
    UART_init(MYUBRR);

    LED_DDR |= (1 << RED_LED) | (1 << YELLOW_LED) | (1 << GREEN_LED);   // Set pins as outputs
    LED_PORT &= ~((1 << RED_LED) | (1 << YELLOW_LED) | (1 << GREEN_LED)); // Ensure all are off initially

    // _delay_ms(3000);  // Give GSM module time to initialize
    blink_led(GREEN_LED, 1);
    send_sms();

    // UART_send_string("AT\r\n");
    // blink_led(YELLOW_LED, 1);

    while (1) {
        // Idle loop
    }

    return 0;
}