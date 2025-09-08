#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>

void blink_led(void)
{
    PORTC |= (1 << PC0);   // LED ON
    _delay_ms(100);        // 100 ms świecenia
    PORTC &= ~(1 << PC0);  // LED OFF
}

int main(void)
{
    // PB1 i PC0 jako wyjścia
    DDRB |= (1 << PB1);
    DDRC |= (1 << PC0);

    while (1)
    {
        // Włącz MOSFET
        PORTB |= (1 << PB1);
        blink_led();
        for (uint8_t i = 0; i < 30; i++) _delay_ms(1000); // 30 s

        // Wyłącz MOSFET
        PORTB &= ~(1 << PB1);
        blink_led();
        for (uint8_t i = 0; i < 30; i++) _delay_ms(1000); // 30 s
    }
}