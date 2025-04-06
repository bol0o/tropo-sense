#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>
#include <string.h>
#include <avr/interrupt.h>

#define I2C_SLAVE_ADDR 0x08
#define BUFFER_SIZE 32
#define UART_BAUDRATE 9600

volatile uint8_t received_command = 0;
volatile char gsm_response[BUFFER_SIZE] = "INIT";  // Default response
volatile uint8_t index = 0;  // Tracks current byte being sent

void uart_init() {
    uint16_t ubrr = F_CPU / 16 / UART_BAUDRATE - 1;
    UBRRH = (uint8_t)(ubrr >> 8);
    UBRRL = (uint8_t)ubrr;
    UCSRB = (1 << TXEN) | (1 << RXEN);
    UCSRC = (1 << UCSZ1) | (1 << UCSZ0);
}

void uart_send(char *data) {
    while (*data) {
        while (!(UCSRA & (1 << UDRE)));  // Wait until TX buffer is empty
        UDR = *data++;
    }
}

void uart_receive() {
    memset((char *)gsm_response, 0, BUFFER_SIZE);
    uint8_t i = 0;

    while (i < BUFFER_SIZE - 1) {
        while (!(UCSRA & (1 << RXC)));  // Wait for data
        gsm_response[i] = UDR;
        if (gsm_response[i] == '\n') break;  // Stop on newline
        i++;
    }
    gsm_response[i] = '\0';  // Null-terminate string
}

void i2c_init() {
    TWAR = (I2C_SLAVE_ADDR << 1);  // Set I2C slave address
    TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT) | (1 << TWIE);  // Enable I2C + interrupts
}

ISR(TWI_vect) {
    switch (TWSR & 0xF8) {
        case TW_SR_SLA_ACK:  // Master started communication
            index = 0;  // Reset index for new read
            TWCR |= (1 << TWINT) | (1 << TWEA);
            break;

        case TW_SR_DATA_ACK:  // Master sent data
            received_command = TWDR;  // Store command from master
            TWCR |= (1 << TWINT) | (1 << TWEA);
            break;

        case TW_ST_SLA_ACK:   // Master requests data
        case TW_ST_DATA_ACK:
        {
            size_t length = strlen((const char *)gsm_response);
            if (index < length) {
                TWDR = gsm_response[index++];  // Send byte
            } else {
                TWDR = 0x00;  // Send null terminator at the end
            }
            TWCR |= (1 << TWINT) | (1 << TWEA);
            break;
        }

        case TW_ST_LAST_DATA:
            TWCR |= (1 << TWINT) | (1 << TWEA);
            break;

        default:
            TWCR |= (1 << TWINT) | (1 << TWEA);
            break;
    }
}

void handle_request() {
    if (received_command == 1) {  // Command received from RPi
        uart_send("AT+CSQ\r\n");  // Example GSM query
        _delay_ms(1000);  // Give GSM module time to respond
        uart_receive();  // Store response in gsm_response
        received_command = 0;  // Reset command flag
    }
}

int main() {
    uart_init();
    i2c_init();
    sei();  // Enable global interrupts

    while (1) {
        handle_request();
    }
}