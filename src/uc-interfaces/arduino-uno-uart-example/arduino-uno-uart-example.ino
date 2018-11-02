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

#include "arduino-uno-uart-minor-keys.h"
#include "uc-generic-fsm.h"
#include "uc-generic-io.h"

#include <Servo.h>

// Set and clear bit helpers
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

// Define extern variables
const uint8_t uc_dio_num_pins = 14;
const uint8_t uc_aio_num_pins = 6;
// Pulled from arduino-uno-uart-minor-keys.h
// Mapped to combo values of input and output
const uint8_t uc_dio_input = IO_INPUT;
const uint8_t uc_dio_output = IO_OUTPUT;

// Buffer Variables
const int len = 32;

// Setup pin setting info and watch arrays
uint8_t DIO_SET[uc_dio_num_pins];
uint16_t DIO_VAL[uc_dio_num_pins];
Servo DIO_SERVO[uc_dio_num_pins];
float PWM_SCALE = 255.0f / 100.0f;

// Setup pin watch for AIO
// No arrays since no Analog outputs

// Setup map values for AIO (0-5V)
float AIO_LOW = 0.0f;
float AIO_HIGH = 5.0f;
float AIO_RES = 1024.0f;
float AIO_RANGE = 100.0f;
float AIO_SCALE = AIO_RANGE * ((AIO_HIGH - AIO_LOW) / AIO_RES);

// Read array (more DIO than AIO)
uint16_t read_data[uc_dio_num_pins];

// Ignore uc_aio_set (can't use on arudino)
void uc_aio_set(uint8_t, uint8_t, uint16_t) {}

// Ignore uc_remote_conn (not implemented yet)
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
    // Clear any running DIO functions (PWM and servo)
    for (uint8_t i = 0; i < uc_dio_num_pins; i++)
    {
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

void uc_delay_us(uint32_t us)
{
    delayMicroseconds(us);
}

void uc_delay_ms(uint32_t ms)
{
    delay(ms);
}

uint32_t uc_bytes_available()
{
    return Serial.available();
}

uint8_t uc_send(uint8_t* data, uint32_t data_len)
{
    return Serial.write(data, data_len);
}

// Read and return the DIO state
uint16_t uc_dio_read(uint8_t pin_num)
{
    // Verify pin_num in range
    if (uc_dio_num_pins < pin_num) return 0;
    
    // Read and return
    switch (DIO_SET[pin_num])
    {
        case IO_INPUT:
            if (pin_num < 8) return (PIND & (1 << pin_num)) ? IO_ON : IO_OFF;
            else return (PINB & (1 << (pin_num - 8))) ? IO_ON : IO_OFF;
            break;
        case IO_PWM:
        case IO_OUTPUT:
        case IO_SERVO_US:
        case IO_SERVO_DEG:
            return DIO_VAL[pin_num];
            break;
        default:
            return 0;
            break;
    }
}

// Read and return the DIO states
uint16_t* uc_dio_read_all()
{
    // Compose data
    for (uint8_t i = 0; i < uc_dio_num_pins; i++)
    {
        // Add pin value to list (convert to big endian)
        read_data[i] = __builtin_bswap16(uc_dio_read(i));
    }

    // Send data to GUI (use fsm_send to add checksum)
    return read_data;
}

// Read and return the AIO state
uint16_t uc_aio_read(uint8_t pin_num)
{
    // Verify pin_num in range
    if (uc_aio_num_pins < pin_num) return 0;

    // Read and return
    return (uint16_t) (AIO_SCALE * analogRead(pin_num));
}

// Read and return the AIO states
uint16_t* uc_aio_read_all()
{
    // Compose data
    for (uint8_t i = 0; i < uc_aio_num_pins; i++)
    {
        // Add pin value to list (convert to big endian)
        read_data[i] = __builtin_bswap16(uc_aio_read(i));
    }

    // Return a pointer to read_data array
    return read_data;
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
void uc_dio_set(uint8_t pin_num, uint8_t setting, uint16_t value)
{
    // Verify pin_num in range
    if (uc_dio_num_pins < pin_num) return;

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

