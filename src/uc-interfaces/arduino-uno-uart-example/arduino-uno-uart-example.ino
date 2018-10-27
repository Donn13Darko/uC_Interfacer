/*
 * uC Interface - A GUI for Programming & Interfacing with Microcontrollers
 * Copyright (C) 2018  Mitchell Oleson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "gui-base-major-keys.h"
#include "arduino-uno-uart-minor-keys.h"
#include "gui-pin-io-base-minor-keys.h"
#include "uc-generic-fsm.h"
#include "uc-generic-io.h"
#include <Servo.h>

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

// Buffer Variables
const int len = 32;

// Setup pin watch for DIO
const uint8_t num_DIO = 14;
uint8_t DIO_SET[num_DIO];
uint16_t DIO_VAL[num_DIO];
Servo DIO_SERVO[num_DIO];
float PWM_SCALE = 255.0f / 100.0f;

// Setup pin watch for AIO
const uint8_t num_AIO = 6;
// No char array since no Analog outputs

// Setup map values for AIO (0-5V)
float AIO_LOW = 0.0f;
float AIO_HIGH = 5.0f;
float AIO_RES = 1024.0f;
float AIO_RANGE = 100.0f;
float AIO_SCALE = AIO_RANGE * ((AIO_HIGH - AIO_LOW) / AIO_RES);

// Read array (more DIO than AIO)
const uint8_t dio_data_len = 2*num_DIO;
const uint8_t aio_data_len = 2*num_AIO;
uint8_t read_data[dio_data_len];

// Just ignore these functions for now
void uc_data_transmit(uint8_t, const uint8_t*, uint8_t) {}
void uc_programmer(uint8_t, const uint8_t*, uint8_t) {}
void uc_custom_cmd(uint8_t, const uint8_t*, uint8_t) {}
void uc_aio(uint8_t pin_num, uint8_t setting, uint16_t value) {}
void uc_remote_conn() {}

// Function prototypes
void set_pwm_on(uint8_t pin);
void set_pwm_off(uint8_t pin);

// Arduino setup function
void setup()
{
    // Set ADC to fast mode
    ADCSRA |= 0x02; // Set bit 2
    ADCSRA &= 0xFC; // Clear bits 1 & 0
    
    // Init Serial transfer
    Serial.setTimeout(packet_timeout);
    Serial.begin(115200);

    // Init fsm
    fsm_setup(len);
}

// Arduino Loop function
void loop()
{
    // fsm_poll() blocks forever (infinite loop)
    // Use repeated calls to fsm_isr() to run other things during the main loop
    // fsm_isr will return false while still waiting and true when ready
    fsm_poll();
}

// Reset all internal values, for use with new connections
void uc_reset()
{
    uint8_t VALUE;
    for (int i = 0; i < num_DIO; i++)
    {
        VALUE = DIO_SET[i];
        switch (DIO_SET[i])
        {
            case IO_PWM:
                set_pwm_off(i);
                break;
            case IO_SERVO_US:
            case IO_SERVO_DEG:
                DIO_SERVO[i].detach();
                break;
        }
    }
    
    // Set all DIOs to INPUTS
    DDRD &= 0x00;
    DDRB &= 0xC0;

    // Reset buffered data
    memset(DIO_SET, IO_INPUT, sizeof(DIO_SET));
    memset(DIO_VAL, IO_OFF, sizeof(DIO_VAL));

    // Reset Receive Buffer
    uc_reset_buffers();
}

void uc_reset_buffers()
{
    // Reset and flush buffers
    Serial.flush();
    while (Serial.available()) { Serial.read(); }
}

uint8_t uc_getch()
{
    return Serial.read();
}

void uc_delay(uint32_t ms)
{
    delay(ms);
}

uint8_t uc_bytes_available()
{
    return Serial.available();
}

uint8_t uc_send(uint8_t* data, uint32_t data_len)
{
    return Serial.write(data, data_len);
}

// Read and return the DIO states
void uc_dio_read()
{
    // Setup variables
    uint16_t val;
    uint8_t j = 0;    

    // Compose data
    for (uint8_t i = 0; i < num_DIO; i++)
    {        
        // Iterate over pins
        switch (DIO_SET[i])
        {
            case IO_INPUT:
                if (i < 8) val = (PIND & (1 << i)) ? IO_ON : IO_OFF;
                else val = (PINB & (1 << (i - 8))) ? IO_ON : IO_OFF;
                break;
            case IO_PWM:
            case IO_OUTPUT:
            case IO_SERVO_US:
            case IO_SERVO_DEG:
                val = DIO_VAL[i];
                break;
            default:
                val = 0;
                break;
        }

        // Add pin value to list (convert to big endian)
        *((uint16_t*)(read_data + j)) = __builtin_bswap16(val);
        j += 2;
    }

    // Send data to GUI (use fsm_send to add checksum)
    fsm_send(MAJOR_KEY_IO, MINOR_KEY_IO_DIO_READ, (uint8_t*) read_data, j);
}

// Read and return the AIO states
void uc_aio_read()
{
    // Setup variables
    uint16_t val;
    uint8_t j = 0;

    // Compose data
    for (uint8_t i = 0; i < num_AIO; i++)
    {
        // Scale value
        val = (uint16_t) (AIO_SCALE * analogRead(i));

        // Add pin value to list (convert to big endian)
        *((uint16_t*)(read_data + j)) = __builtin_bswap16(val);
        j += 2;
    }

    // Send data to GUI (use fsm_send to add checksum)
    fsm_send(MAJOR_KEY_IO, MINOR_KEY_IO_AIO_READ, (uint8_t*) read_data, j);
}

// Connect PWM timer
void set_pwm_on(uint8_t pin)
{
    switch (pin)
    {
        case 3:
            sbi(TCCR2A, COM2B1);
            break;
        case 5:
            sbi(TCCR0A, COM0B1);
            break;
        case 6:
            sbi(TCCR0A, COM0A1);
            break;
        case 9:
            sbi(TCCR1A, COM1A1);
            break;
        case 10:
            sbi(TCCR1A, COM1B1);
            break;
        case 11:
            sbi(TCCR2A, COM2A1);
            break;
        default:
            break;
    }
}

// Disconnect PWM timer
void set_pwm_off(uint8_t pin)
{
    switch (pin)
    {
        case 3:
            cbi(TCCR2A, COM2B1);
            break;
        case 5:
            cbi(TCCR0A, COM0B1);
            break;
        case 6:
            cbi(TCCR0A, COM0A1);
            break;
        case 9:
            cbi(TCCR1A, COM1A1);
            break;
        case 10:
            cbi(TCCR1A, COM1B1);
            break;
        case 11:
            cbi(TCCR2A, COM2A1);
            break;
        default:
            break;
    }
}

// Set the DIO as per the command
void uc_dio(uint8_t pin_num, uint8_t setting, uint16_t value)
{
    // If setting changed
    if (DIO_SET[pin_num] != setting)
    {
        switch (DIO_SET[pin_num])
        {
            case IO_PWM:
                set_pwm_off(pin_num);
                break;
            case IO_SERVO_US:
            case IO_SERVO_DEG:
                DIO_SERVO[pin_num].detach();
                break;
            default:
                break;
        }
        
        switch (setting)
        {
            case IO_INPUT:
                if (pin_num < 8) cbi(DDRD, pin_num);
                else cbi(DDRB, pin_num - 8);
                break;
            case IO_PWM:
            case IO_OUTPUT:
                if (pin_num < 8) DDRD |= (1 << pin_num);
                else DDRB |= (1 << (pin_num - 8));
                break;
            case IO_SERVO_US:
            case IO_SERVO_DEG:
                DIO_SERVO[pin_num].attach(pin_num);
                break;
            default:
                return;
        }

        DIO_SET[pin_num] = setting;
    }

    switch (setting)
    {
        case IO_PWM:
            if (value && (value != 100))
            {
                set_pwm_on(pin_num);
                value = (int) (PWM_SCALE * (float) value);
                switch(pin_num)
                {
                    case 3:
                        OCR2B = value;
                        break;
                    case 5:
                        OCR0B = value;
                        break;
                    case 6:
                        OCR0A = value;
                        break;
                    case 9:
                        OCR1A = value;
                        break;
                    case 10:
                        OCR1B = value;
                        break;
                    case 11:
                        OCR2A = value;
                        break;
                }
                break;
            }
            // Handle edge cases of 0 and 100
            set_pwm_off(pin_num);
            value = !!value;
        case IO_OUTPUT:
            if (pin_num < 8) PORTD = (PORTD & ~(1 << pin_num)) | (value << pin_num);
            else PORTB = (PORTB & ~(1 << (pin_num - 8))) | (value << (pin_num - 8));
            break;
        case IO_SERVO_US:
            DIO_SERVO[pin_num].writeMicroseconds(value);
            break;
        case IO_SERVO_DEG:
            DIO_SERVO[pin_num].write(value);
            break;
        default: // Do nothing for IO_INPUT
            return;
    }

    DIO_VAL[pin_num] = value;
}

