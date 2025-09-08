#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include <math.h>

// Updated pin definitions for ATtiny84
#define WIND_BUTTON_PIN        PB0
#define RAIN_BUTTON_PIN        PB1
#define WIND_VANE_CHANNEL      0

#define SOLAR_CURRENT_CHANNEL  1
#define SOLAR_VOLTAGE_CHANNEL  2

#define READINGS_NR            5
#define WIND_DIR_READINGS      110
#define TIMER_LENGTH           5
#define I2C_SLAVE_ADDRESS      0x42

// CONSTANTS

typedef struct {
    float voltage;
    float direction_deg;
} WindDir;

const WindDir wind_table[] = {
    {1.36, 0},
    {0.40, 22.5},
    {0.49, 45},
    {0.06, 67.5},
    {0.07, 90},
    {0.05, 112.5},
    {0.15, 135},
    {0.10, 157.5},
    {0.25, 180},
    {0.21, 202.5},
    {0.84, 225},
    {0.76, 247.5},
    {2.37, 270},
    {1.56, 292.5},
    {1.91, 315},
    {1.05, 337.5}
};

#define WIND_TABLE_LEN (sizeof(wind_table) / sizeof(wind_table[0]))

typedef union {
    float f;
    uint8_t bytes[4];
} FloatUnion;

// VARIABLES

volatile uint8_t wind_dir_idx = 0;
volatile uint8_t timers = 0;

volatile uint8_t energy_update_flag = 0;
volatile uint8_t wind_dir_update_flag = 0;
uint16_t wind_dir[WIND_DIR_READINGS] = {0};

uint8_t tx_index = 0;

typedef struct {
    uint16_t rain_count;
    uint16_t wind_count;
    uint16_t interval_wind_count;
    uint16_t max_interval_wind_count;
    uint16_t avg_wind_dir;
    FloatUnion energy_generated;
} Data;

volatile Data data;

// HELPER FUNCTIONS

void init(uint8_t i2c_address) {
    // PIN INIT
    // Set WIND_BUTTON_PIN (PCINT0) and RAIN_BUTTON_PIN (PCINT1) as inputs with pull-ups
    DDRB &= ~((1 << WIND_BUTTON_PIN) | (1 << RAIN_BUTTON_PIN));
    PORTB |= (1 << WIND_BUTTON_PIN) | (1 << RAIN_BUTTON_PIN); // Enable pull-ups

    // Enable pin change interrupts for PB0 and PB1
    GIMSK |= (1 << PCIE0); // Enable PCINT0-7
    PCMSK0 |= (1 << PCINT0) | (1 << PCINT1); // Enable interrupts on PB0 and PB1

    // ADC INIT
    ADMUX = (1 << REFS1) | (1 << REFS0); // Internal 2.56V Vref
    ADCSRA = (1 << ADEN)                 // Enable ADC
           | (1 << ADPS1) | (1 << ADPS0); // Prescaler 8

    // USI (I2C) INIT - ATtiny84 uses USI for TWI
    // Note: USI implementation is different from standard TWI
    // This is a simplified initialization
    USICR = (1 << USIWM1) | (1 << USICS1) | (1 << USICS0); // I2C mode, external clock
    USIDR = 0xFF; // Release SDA
    USISR = (1 << USISIF) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC);
    DDRB &= ~((1 << PB0) | (1 << PB1)); // Ensure SDA and SCL are inputs

    // TIMER INIT
    // CTC mode: WGM12 = 1
    TCCR1B |= (1 << WGM12);
    TCCR1A = 0;

    // Prescaler = 1024
    TCCR1B |= (1 << CS12) | (1 << CS10);

    // OCR1A = (F_CPU / prescaler) * seconds - 1
    OCR1A = (F_CPU / 1024) * TIMER_LENGTH - 1;

    // Enable Timer1 compare match A interrupt
    TIMSK1 |= (1 << OCIE1A);
}

uint16_t ADC_read(uint8_t channel) {
    // Keep internal Vref, set ADC channel
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F); 
    _delay_us(10); // Small delay for Vref to settle
    ADCSRA |= (1 << ADSC); // Start conversion
    while (ADCSRA & (1 << ADSC)); // Wait
    return ADC;
}

uint8_t crc8(uint8_t *data, uint8_t length) {
    uint8_t crc = 0x00; // Initial CRC value
    uint8_t polynomial = 0x07; // CRC-8 polynomial (x^8 + x^2 + x + 1)

    for (uint8_t i = 0; i < length; i++) {
        crc ^= data[i]; // XOR the byte with the current CRC value
        for (uint8_t j = 0; j < 8; j++) { // Process each bit
            if (crc & 0x80) { // Check if the most significant bit is set
                crc = (crc << 1) ^ polynomial; // Shift and apply the polynomial
            } else {
                crc <<= 1; // Just shift if no XOR is needed
            }
        }
    }
    return crc; // Return the calculated CRC
}

uint8_t calculate_checksum() {
    uint8_t checksum = 0;
    
    checksum += (uint8_t)data.rain_count;  // LSB of rain_count
    checksum += (uint8_t)(data.rain_count >> 8);  // MSB of rain_count
    checksum += (uint8_t)data.wind_count;  // LSB of wind_count
    checksum += (uint8_t)(data.wind_count >> 8);  // MSB of wind_count
    checksum += (uint8_t)data.max_interval_wind_count;  // LSB of max_interval_wind_count
    checksum += (uint8_t)(data.max_interval_wind_count >> 8);  // MSB of max_interval_wind_count
    checksum += (uint8_t)data.avg_wind_dir;  // LSB of avg_wind_dir
    checksum += (uint8_t)(data.avg_wind_dir >> 8);  // MSB of avg_wind_dir
    checksum += data.energy_generated.bytes[0];  // XLSB of energy_generated
    checksum += data.energy_generated.bytes[1];  // LSB of energy_generated
    checksum += data.energy_generated.bytes[2];  // MSB of energy_generated
    checksum += data.energy_generated.bytes[3];  // XMSB of energy_generated
    
    return checksum;
}

// WIND SPEED

void update_max_wind_interval() {
    if (data.max_interval_wind_count < data.interval_wind_count) {
        data.max_interval_wind_count = data.interval_wind_count;
    }

    data.interval_wind_count = 0;
}

// WIND DIRECTION

uint16_t angle_to_int(float angle) {
    return (uint16_t)(angle * 100.0f + 0.5f);  // Round to nearest 0.01°
}

float int_to_angle(uint16_t val) {
    return (float)val / 100.0f;
}

float get_average(uint16_t *angles, uint8_t length) {
    float sin_sum = 0.0f;
    float cos_sum = 0.0f;
    float r, s, c, arc, average;

    // Sum sine and cosine components
    for (uint8_t i = 0; i < length; i++) {
        r = int_to_angle(angles[i]) * (M_PI / 180.0f);  // Degrees to radians
        sin_sum += sinf(r);
        cos_sum += cosf(r);
    }

    // Compute mean direction
    s = sin_sum / (float)length;
    c = cos_sum / (float)length;
    arc = atanf(s / c) * (180.0f / M_PI);  // Radians to degrees

    // Adjust quadrant
    if (s > 0 && c > 0) {
        average = arc;
    } else if (c < 0) {
        average = arc + 180.0f;
    } else if (s < 0 && c > 0) {
        average = arc + 360.0f;
    } else {
        average = 0.0f;  // Handle edge cases
    }

    return (average == 360.0f) ? 0 : angle_to_int(average);
}

float direction_from_voltage(float voltage) {
    uint8_t closest_index = 0;
    float smallest_diff = fabsf(voltage - wind_table[0].voltage);

    for (uint8_t i = 1; i < WIND_TABLE_LEN; i++) {
        float current_diff = fabsf(voltage - wind_table[i].voltage);
        if (current_diff < smallest_diff) {
            smallest_diff = current_diff;
            closest_index = i;
        }
    }

    return wind_table[closest_index].direction_deg;
}

void update_wind_dir_readings() {
    uint16_t adc_val;
    float adc_voltage;
    uint16_t readings[READINGS_NR];

    if (wind_dir_idx >= WIND_DIR_READINGS) {
        return;
    }

    for (uint8_t i = 0; i < READINGS_NR; i++) {
        adc_val = ADC_read(WIND_VANE_CHANNEL);
        adc_voltage = (adc_val / 1023.0) * 2.56;

        readings[i] = angle_to_int(direction_from_voltage(adc_voltage));

        _delay_ms(200);
    }

    wind_dir[wind_dir_idx++] = get_average(readings, READINGS_NR);
    data.avg_wind_dir = angle_to_int(get_average(wind_dir, wind_dir_idx + 1));
}

// SOLAR ENERGY

void update_energy_generated() {
    float total_voltage = 0;
    float total_current = 0;

    for (uint8_t i = 0; i < READINGS_NR; i++) {
        uint16_t adc_val_voltage = ADC_read(SOLAR_VOLTAGE_CHANNEL); // Voltage input on ADC2 (PA2)
        uint16_t adc_val_current = ADC_read(SOLAR_CURRENT_CHANNEL); // Current input on ADC1 (PA1)

        float voltage = 12000 * ((adc_val_voltage / 1023.0) * 2.56) / 1000; // V

        float voltage_current = (adc_val_current / 1023.0) * 2.56;
        float current = (voltage_current * 1000.0) / (0.1 * 30000.0);  // A (R = 0.1Ω, G = 30x)

        total_voltage += voltage;
        total_current += current;

        _delay_ms(100);
    }

    float avg_voltage = total_voltage / READINGS_NR;
    float avg_current = total_current / READINGS_NR;

    float power_w = avg_voltage * avg_current;
    float energy_wh = power_w * (5.5 / 3600.0); // 5 sec → hours

    data.energy_generated.f += energy_wh * 1000;
}

// INTERRUPTS

ISR(PCINT0_vect) {
    // Check which pin triggered the interrupt
    if (!(PINB & (1 << WIND_BUTTON_PIN))) {
        data.wind_count++;
        data.interval_wind_count++;
    }
    
    if (!(PINB & (1 << RAIN_BUTTON_PIN))) {
        data.rain_count++;
    }
}

ISR(TIM1_COMPA_vect) {
    timers++;
    update_max_wind_interval();

    energy_update_flag = 0;

    if (timers % 2 == 0) {
        wind_dir_update_flag = 1;
    }
}

ISR(USI_STR_vect) {
    // USI Start Condition Interrupt
    // Handle I2C start condition
    USISR |= (1 << USIOIF); // Clear interrupt flag
}

ISR(USI_OVF_vect) {
    // USI Overflow Interrupt - used for I2C data transfer
    uint8_t usi_data = USIDR;
    uint8_t checksum = calculate_checksum();
    
    if (USISR & (1 << USIPF)) {
        // Stop condition detected
        USISR |= (1 << USIPF); // Clear stop flag
    } else {
        // Handle data transfer
        if (usi_data == 'R') {
            // Reset command
            timers = 0;
            TCNT1 = 0;

            data.rain_count = 0;
            data.wind_count = 0;
            data.interval_wind_count = 0;
            data.max_interval_wind_count = 0;
            for (uint8_t i = 0; i < wind_dir_idx; i++) {
                wind_dir[i] = 0;
            }
            wind_dir_idx = 0;
            data.energy_generated.f = 0.0f;
        } else {
            // Data request
            switch (tx_index) {
                case 0:
                    USIDR = (uint8_t)data.rain_count; // LSB
                    break;
                case 1:
                    USIDR = (uint8_t)(data.rain_count >> 8); // MSB
                    break;
                case 2:
                    USIDR = (uint8_t)data.wind_count; // LSB
                    break;
                case 3:
                    USIDR = (uint8_t)(data.wind_count >> 8); // MSB
                    break;
                case 4:
                    USIDR = (uint8_t)data.max_interval_wind_count; // LSB
                    break;
                case 5:
                    USIDR = (uint8_t)(data.max_interval_wind_count >> 8); // MSB
                    break;
                case 6:
                    USIDR = (uint8_t)(data.avg_wind_dir); // LSB
                    break;
                case 7:
                    USIDR = (uint8_t)(data.avg_wind_dir >> 8); // MSB
                    break;
                case 8:
                    USIDR = data.energy_generated.bytes[0]; // XLSB
                    break;
                case 9:
                    USIDR = data.energy_generated.bytes[1]; // LSB
                    break;
                case 10:
                    USIDR = data.energy_generated.bytes[2]; // MSB
                    break;
                case 11:
                    USIDR = data.energy_generated.bytes[3]; // XMSB
                    break;
                case 12:
                    USIDR = checksum;  // Send checksum byte
                    break;
                default:
                    USIDR = 0xFF;  // Default value
                    break;
            }
            tx_index++;
        }
    }
    
    USISR |= (1 << USIOIF); // Clear overflow flag
}

// MAIN LOOP

int main(void) {
    init(I2C_SLAVE_ADDRESS);
    sei(); // Enable global interrupts

    while (1) {
        if (energy_update_flag) {
            energy_update_flag = 0;
            update_energy_generated();
        }

        if (wind_dir_update_flag) {
            wind_dir_update_flag = 0;
            update_wind_dir_readings();
        }
    }
}