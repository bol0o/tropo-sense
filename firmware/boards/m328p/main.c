#include <avr/io.h>
#include <util/delay.h>
#include "config.h"
#include "../../peripherals/gsm_module.h"

void swtich_gsm(void) {
    // ustaw PB1 jako wyjście
    DDRB |= (1 << PB1);

    // ustaw stan wysoki
    PORTB |= (1 << PB1);

    // odczekaj 3 sekundy
    _delay_ms(3000);

    // z powrotem niski
    PORTB &= ~(1 << PB1);
}

int main(void){
    UART_init_ISR(MYUBRR);
    sei();
    
    /* (1) start i echo */
    swtich_gsm();
    gsm_wait_ready(60000);
    gsm_disable_echo(1000);

    /* (2) HTTP POST */
    // const char* url  = "https://7fc9cee303d2.ngrok-free.app/ingest";
    // const char* ctyp = "application/json";
    // const char* body = "{\"hello\":\"świat\"}";
    // gsm_http_post(url, ctyp, body, strlen(body), /*HTTPDATA timeout s*/5, /*ACTION timeout ms*/120000);

    /* (3) SMS z polskimi znakami */
    // gsm_sms_send_ucs2("48xxxxxxxxx, "Test (źćń)", /*retries*/3, /*overall ms*/120000);

    swtich_gsm();
}
