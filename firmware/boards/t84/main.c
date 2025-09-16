// main.c — ATtiny84 + USI I2C Slave Driver (CalcProgrammer1)
#include <avr/io.h>
#include <avr/interrupt.h>
#include "usi_i2c_slave.h"
#include <util/delay.h>

// *** WAŻNE ***
// Ta tablica jest zdefiniowana w bibliotece (usi_i2c_slave.c).
// My tylko deklarujemy do niej extern i wpisujemy wskaźniki:
extern unsigned char* USI_Slave_register_buffer[];

// Własne zmienne „wystawiane” po I2C
volatile uint8_t led_ctrl = 0;   // rejestr 0x00: 0/1 -> LED OFF/ON

static inline void led_init(void){
  DDRB  |= (1<<PB0);
  PORTB &= ~(1<<PB0);
}
static inline void led_apply(uint8_t v){
  if (v & 1) PORTB |=  (1<<PB0);
  else       PORTB &= ~(1<<PB0);
}

int main(void){
  led_init();

  // 1) Zmapuj swoje zmienne na kolejne rejestry I2C
  //    (każda pozycja to *wskaźnik na bajt*)
  USI_Slave_register_buffer[0] = (unsigned char*)&led_ctrl;           // 0x00

  // 2) Start slava pod adresem 0x27
  USI_I2C_Init(0x27);

  sei();

  // Pętla: licznik i reakcja na zapis do rejestru 0x00
  for(;;){
    // prosty „heartbeat”
    led_apply(led_ctrl);

    // drobna pauza (opcjonalnie)
    _delay_ms(10);
  }
}
