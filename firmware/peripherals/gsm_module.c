#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "gsm_module.h"
#include "../communication/uart_isr.h"
#include <string.h>
#include <util/delay.h>

/* -------------------- NARZĘDZIA RX/TX -------------------- */

static bool wait_for_tokens(const char **must, uint8_t n_must, const char **fatal, uint8_t n_fatal, uint32_t timeout_ms) {
    char acc[256]; size_t acc_len = 0;
    acc[0] = '\0';

    for(uint32_t t=0; t<timeout_ms; ++t){
        int16_t ch;
        while((ch = UART_receive()) >= 0){
            if(acc_len+1 < sizeof(acc)){
                acc[acc_len++] = (char)ch;
                acc[acc_len] = '\0';
            } else {
                /* przesuwamy okno, żeby trzymać tylko koniec */
                memmove(acc, acc+1, --acc_len);
                acc[acc_len++] = (char)ch;
                acc[acc_len] = '\0';
            }
        }

        /* najpierw fatale */
        for(uint8_t i=0; i<n_fatal; ++i){
            if(fatal[i] && strstr(acc, fatal[i])) return false;
        }

        /* potem sprawdź czy wszystkie must już są */
        bool all = true;
        for(uint8_t i=0; i<n_must; ++i){
            if(!must[i]) continue;
            if(!strstr(acc, must[i])) { all = false; break; }
        }
        if(all) return true;

        _delay_ms(1);
    }
    return false;
}

bool gsm_wait_prompt_gt(uint32_t timeout_ms) {
    for(uint32_t t=0; t<timeout_ms; ++t){
        int16_t ch;
        while((ch = UART_receive()) >= 0){
            if((char)ch == '>') return true;
        }
        _delay_ms(1);
    }
    return false;
}

bool gsm_cmd_ok(const char* cmd, uint32_t timeout_ms) {
    static const char* MUST[]  = { "OK" };
    static const char* FATAL[] = { "ERROR" };

    /* Wyczyść ogon z poprzednich URC, wyślij komendę */
    for(uint8_t i = 0; i < 10; i++){
        while (UART_data_available()) {
            (void) UART_receive();
            _delay_ms(1);
        }
    }
    UART_send_string(cmd); UART_send_string("\r\n");

    /* Czekamy aż pojawi się OK albo ERROR (w jednej pętli) */
    return wait_for_tokens(MUST, 1, FATAL, 1, timeout_ms);
}

bool gsm_ping(uint16_t timeout_ms) {
    return gsm_cmd_ok("AT", timeout_ms);
}

/* -------------------- INICJALIZACJA / ECHO -------------------- */

bool gsm_wait_ready(uint32_t total_timeout_ms) {
    bool got_atready = false;
    bool got_cpin = false;
    bool got_sms = false;

    static char line[128];
    uint8_t idx=0;

    for(uint32_t t=0; t<total_timeout_ms; ++t){
        int16_t ch;
        while((ch = UART_receive()) >= 0){
            char c = (char)ch;
            if(idx < sizeof(line)-1) line[idx++] = c;
            if(c=='\n' || c=='\r'){
                line[idx]='\0';
                if(strstr(line, "*ATREADY: 1")) got_atready=true;
                if(strstr(line, "+CPIN: READY")) got_cpin=true;
                if(strstr(line, "SMS DONE"))     got_sms=true;
                idx=0;
                if(got_atready && got_cpin && got_sms) return true;
            }
        }
        _delay_ms(1);
    }
    return false;
}

bool gsm_disable_echo(uint16_t timeout_ms) {
    return gsm_cmd_ok("ATE0", timeout_ms);
}

/* Zastępuje: wait_cmgs_result(...) i wszędzie gdzie było kilka stream_find po kolei */
static bool wait_cmgs_ok(uint32_t timeout_ms) {
    char acc[256]; size_t acc_len=0; acc[0]='\0';
    bool got_cmgs=false, got_ok=false;

    for(uint32_t t=0; t<timeout_ms; ++t){
        int16_t ch;
        while((ch = UART_receive()) >= 0){
            if(acc_len+1 < sizeof(acc)){
                acc[acc_len++] = (char)ch; acc[acc_len]='\0';
            } else {
                memmove(acc, acc+1, --acc_len);
                acc[acc_len++] = (char)ch; acc[acc_len]='\0';
            }
        }
        if(strstr(acc, "ERROR")) return false;
        if(strstr(acc, "+CMGS:")) got_cmgs = true;
        if(strstr(acc, "OK"))     got_ok   = true;

        if(got_cmgs && got_ok) return true;
        _delay_ms(1);
    }
    return false;
}

/* -------------------- HTTP POST -------------------- */

bool gsm_http_post(const char* url,
                   const char* content_type,
                   const char* data,
                   uint32_t    data_len,
                   uint16_t    httpdata_timeout_s,
                   uint32_t    action_timeout_ms) {
    if(!gsm_cmd_ok("AT+HTTPINIT", 3000)) return false;

    {   char cmd[256];
        snprintf(cmd, sizeof(cmd), "AT+HTTPPARA=\"URL\",\"%s\"", url);
        if(!gsm_cmd_ok(cmd, 5000)) { (void)gsm_cmd_ok("AT+HTTPTERM", 2000); return false; }
    }
    {   char cmd[160];
        snprintf(cmd, sizeof(cmd), "AT+HTTPPARA=\"CONTENT\",\"%s\"",
                 content_type ? content_type : "application/json");
        if(!gsm_cmd_ok(cmd, 3000)) { (void)gsm_cmd_ok("AT+HTTPTERM", 2000); return false; }
    }

    {   char cmd[64];
        snprintf(cmd, sizeof(cmd), "AT+HTTPDATA=%lu,%u",
                 (unsigned long)data_len, (unsigned)(httpdata_timeout_s*1000U));
        UART_send_string(cmd); UART_send_string("\r\n");

        /* prompt: "DOWNLOAD" lub '>' */
        bool got_prompt = wait_for_tokens((const char*[]){"DOWNLOAD"},1,(const char*[]) {NULL},0,5000)
                       || gsm_wait_prompt_gt(5000);
        if(!got_prompt) { (void)gsm_cmd_ok("AT+HTTPTERM", 2000); return false; }

        UART_send_string(data);
        UART_send_string("\r\n");
        /* większość FW po danych daje "OK" — ale niektóre nie; tutaj spróbujmy chwilę poczekać, ale nie traktujmy braku OK jako błąd krytyczny */
        (void)wait_for_tokens((const char*[]){"OK"},1,(const char*[]){"ERROR"},1,10000);
    }

    UART_send_string("AT+HTTPACTION=1\r\n");

    /* Czekamy na URC z wynikiem; parsowanie statusu opcjonalne */
    char acc[256]; size_t acc_len=0; acc[0]='\0';
    int http_status = -1;
    uint32_t ms=0;

    while(ms < action_timeout_ms){
        int16_t ch;
        while((ch = UART_receive()) >= 0){
            if(acc_len+1 < sizeof(acc)){ acc[acc_len++]=(char)ch; acc[acc_len]='\0'; }
            else { memmove(acc, acc+1, --acc_len); acc[acc_len++]=(char)ch; acc[acc_len]='\0'; }
        }
        char *p = strstr(acc, "+HTTPACTION:");
        if(p){
            /* próbuj sparsować ",<status>," */
            int m1, m2;
            if(sscanf(p, "+HTTPACTION: %d,%d,%d", &m1, &http_status, &m2) >= 2){
                /* opcjonalnie doczekaj jeszcze na "OK", ale nie wymagaj */
                (void)wait_for_tokens((const char*[]){"OK"},1,(const char*[]){"ERROR"},1,3000);
                break;
            }
        }
        if(strstr(acc,"ERROR")) { (void)gsm_cmd_ok("AT+HTTPTERM", 2000); return false; }
        _delay_ms(1); ++ms;
    }

    (void)gsm_cmd_ok("AT+HTTPTERM", 3000);
    return (http_status >= 200 && http_status < 400); /* uznaj 2xx/3xx jako sukces */
}

/* -------------------- UTF-8 → UCS2 (big-endian) -------------------- */

static uint16_t utf8_next_ucs2(const char* s, size_t* consumed){
    const unsigned char c = (unsigned char)s[0];

    if (c < 0x80){ *consumed = 1; return (uint16_t)c; }

    if ((c & 0xE0) == 0xC0){
        unsigned char c2 = (unsigned char)s[1];
        *consumed = 2;
        return (uint16_t)(((c & 0x1F) << 6) | (c2 & 0x3F));
    }

    /* Prosty fallback — trzybajtowe znaki mapuj jako znak zapytania */
    *consumed = (c & 0xF0) == 0xE0 ? 3 : 1;
    return (uint16_t)'?';
}

static void utf8_to_ucs2_hex(const char* text, char* out_hex, size_t out_sz, size_t* out_bytes){
    size_t i=0, bytes=0;
    char* p = out_hex;

    while(text[i] && (size_t)(p - out_hex) < (out_sz - 4)){
        size_t cns=0;
        uint16_t u = utf8_next_ucs2(&text[i], &cns);
        i += cns;
        p += sprintf(p, "%02X%02X", (u>>8)&0xFF, u&0xFF);
        bytes += 2;
    }
    *p = '\0';
    if(out_bytes) *out_bytes = bytes;
}

/* -------------------- Numer → BCD (swapped) -------------------- */

static void sanitize_msisdn(char* dst, size_t dst_sz, const char* src){
    size_t j=0;
    for(size_t i=0; src[i] && j+1<dst_sz; ++i){
        if(src[i]=='+') continue;
        if(isdigit((unsigned char)src[i])) dst[j++]=src[i];
    }
    dst[j]='\0';
}

static void msisdn_to_bcd_swapped(const char* msisdn, char* out_hex, size_t out_sz){
    size_t nlen = strlen(msisdn);
    char* p = out_hex;
    for(size_t i=0; i<nlen && (size_t)(p - out_hex) < out_sz-2; i+=2){
        char d1 = msisdn[i];
        char d2 = (i+1<nlen) ? msisdn[i+1] : 'F';
        p += sprintf(p, "%c%c", d2, d1);
    }
    *p = '\0';
}

/* -------------------- Budowa PDU (UCS2) -------------------- */

size_t gsm_build_pdu_submit_ucs2(const char* msisdn_e164,
                                 const char* text_utf8,
                                 char*       out_hex,
                                 size_t      out_hex_sz)
{
    char num[32]; sanitize_msisdn(num, sizeof(num), msisdn_e164);
    size_t nlen = strlen(num);
    if(nlen==0) return 0;

    char ud_hex[560]; size_t ud_bytes=0; /* 280 znaków UCS2 max → 560 bajtów, ale SMS zwykle 70 znaków UCS2 */
    utf8_to_ucs2_hex(text_utf8, ud_hex, sizeof(ud_hex), &ud_bytes);

    char bcd[64]="";
    msisdn_to_bcd_swapped(num, bcd, sizeof(bcd));

    /* SMSC = 00 (domyślne centrum operatora) */
    /* TPDU: 11 | MR=00 | DA_len | DA_type=91 | DA(bcd) | PID=00 | DCS=08 | VP=AA | UDL | UDH */
    int written = snprintf(out_hex, out_hex_sz,
        "00"        /* SMSC */
        "11"        /* SMS-SUBMIT, TP-RD=0 */
        "00"        /* TP-MR */
        "%02X"      /* TP-DA length (digits) */
        "91"        /* Type-of-Address: international */
        "%s"        /* TP-DA digits in BCD swapped */
        "00"        /* PID */
        "08"        /* DCS = UCS2 */
        "AA"        /* Validity (rel) ~4 dni */
        "%02X"      /* UDL (bytes of UCS2) */
        "%s",       /* User Data UCS2 hex */
        (unsigned)(nlen),
        bcd,
        (unsigned)ud_bytes,
        ud_hex
    );
    if(written <= 0) return 0;

    /* Długość TPDU (dla AT+CMGS) = całość bez pierwszego pola SMSC ("00").
       Czyli: (strlen(out_hex)/2) - 1 bajt */
    size_t total_octets = (size_t)strlen(out_hex)/2;
    if(total_octets == 0) return 0;
    size_t tpdu_len = total_octets - 1;

    return tpdu_len;
}

/* -------------------- Wysyłka SMS (UCS2) -------------------- */

static bool sms_send_pdu_once(const char* pdu_hex, size_t tpdu_len, uint32_t timeouts_ms){
    char cmd[32];
    if(!gsm_cmd_ok("AT+CMGF=0", 3000)) return false;

    snprintf(cmd, sizeof(cmd), "AT+CMGS=%lu", (unsigned long)tpdu_len);
    UART_send_string(cmd); UART_send_string("\r\n");

    if(!gsm_wait_prompt_gt(5000)) return false;

    UART_send_string(pdu_hex);
    UART_send(0x1A); /* Ctrl+Z */

    return wait_cmgs_ok(timeouts_ms);
}

bool gsm_sms_send_ucs2(const char* msisdn_e164,
                       const char* text_utf8,
                       uint8_t     max_retries,
                       uint32_t    overall_timeout_ms)
{
    char pdu[640];
    size_t tpdu_len = gsm_build_pdu_submit_ucs2(msisdn_e164, text_utf8, pdu, sizeof(pdu));
    if(tpdu_len == 0) return false;

    uint32_t per_try = overall_timeout_ms / (max_retries ? max_retries : 1);
    for(uint8_t i=0; i<(max_retries?max_retries:1); ++i){
        if(gsm_ping(1000) && sms_send_pdu_once(pdu, tpdu_len, per_try)) return true;
        /* krótka przerwa między próbami */
        for(uint8_t d=0; d<10; ++d) _delay_ms(100);
    }
    return false;
}
