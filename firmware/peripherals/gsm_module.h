#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Wymaga: uart_isr.h (TX/RX ring) i zainicjalizowanego UART-a. */

/* ------------- Inicjalizacja / echo / gotowość ------------- */

/* Czeka na zestaw URC po starcie modemu:
   "*ATREADY: 1", "+CPIN: READY", "SMS DONE", "PB DONE".
   Zwraca true, gdy komplet w czasie <= total_timeout_ms. */
bool gsm_wait_ready(uint32_t total_timeout_ms);

/* Wyłącza echo ATE0 i czeka na "OK". */
bool gsm_disable_echo(uint16_t timeout_ms);

/* Prosty "AT" ping (dla diagnostyki). */
bool gsm_ping(uint16_t timeout_ms);

/* ------------- HTTP POST (AT+HTTP...) ------------- */

/* Wysyła:
   AT+HTTPINIT
   AT+HTTPPARA="URL","<url>"
   AT+HTTPPARA="CONTENT","<content_type>"
   AT+HTTPDATA=<data_len>,<data_timeout_s>
   <data>\r\n
   AT+HTTPACTION=1
   (czeka na "+HTTPACTION: 1,<status>,<len>" oraz "OK")
   AT+HTTPTERM

   Uwaga: wiele modułów po HTTPDATA oczekuje promptu "DOWNLOAD".
*/
bool gsm_http_post(const char* url,
                   const char* content_type,
                   const char* data,
                   uint32_t    data_len,
                   uint16_t    httpdata_timeout_s,
                   uint32_t    action_timeout_ms);

/* ------------- SMS PDU (UCS2, polskie znaki) ------------- */

/* Buduje PDU (TPDU + pusty SMSC "00") w buforze out_hex (ASCII-HEX).
   msisdn_e164: np. "48660123456" (bez plusa; jeśli masz "+48...", możesz pominąć '+')
   text_utf8:   tekst z polskimi znakami (UTF-8)
   Zwraca liczbę bajtów TPDU (wartość pod AT+CMGS=<len>) lub 0 przy błędzie.
*/
size_t gsm_build_pdu_submit_ucs2(const char* msisdn_e164,
                                 const char* text_utf8,
                                 char*       out_hex,
                                 size_t      out_hex_sz);

/* Wysyła SMS:
   - AT+CMGF=0
   - AT+CMGS=<tpdu_len>
   - <pdu_hex>
   - Ctrl+Z
   Czeka na "+CMGS:" i "OK". Próbuje max_retries razy. */
bool gsm_sms_send_ucs2(const char* msisdn_e164,
                       const char* text_utf8,
                       uint8_t     max_retries,
                       uint32_t    overall_timeout_ms);

/* ------------- Narzędzia (opcjonalnie możesz użyć samodzielnie) ------------- */

/* Wysyła komendę AT (bez CRLF nie wysyłaj), czeka na "OK" (true) lub "ERROR"(false). */
bool gsm_cmd_ok(const char* cmd, uint32_t timeout_ms);

/* Szuka podciągu w przychodzącym strumieniu przez timeout_ms (ms). */
bool gsm_stream_find(const char* needle, uint32_t timeout_ms);

/* Czyta aż zobaczy znak '>' (np. po CMGS lub HTTPDATA). */
bool gsm_wait_prompt_gt(uint32_t timeout_ms);