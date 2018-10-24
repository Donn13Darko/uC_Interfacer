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

// Buffer Variables
const int len = 32;

// Setup pin watch for DIO
const uint8_t num_DIO = 14;
uint8_t DIO_SET[num_DIO];
uint16_t DIO_VAL[num_DIO];
Servo DIO_SERVO[num_DIO];
float PWM_SCALE = (float) 255.0 / (float) 100.0;

// Setup pin watch for AIO
const uint8_t num_AIO = 6;
// No char array since no Analog outputs

// Setup map values for AIO (0-5V)
float AIO_LOW = 0.0;
float AIO_HIGH = 5.0;
float AIO_RES = 1024.0;
float AIO_RANGE = 100.0;
float AIO_SCALE = AIO_RANGE * ((AIO_HIGH - AIO_LOW) / AIO_RES);

// Read array (more DIO than AIO)
const uint8_t dio_data_len = 2*num_DIO;
const uint8_t aio_data_len = 2*num_AIO;
uint8_t read_data[dio_data_len+3];

// Just ignore these functions for now
void uc_data_transmit(uint8_t, const uint8_t*, uint8_t) {}
void uc_programmer(uint8_t, const uint8_t*, uint8_t) {}
void uc_custom_cmd(uint8_t, const uint8_t*, uint8_t) {}
void uc_aio(uint8_t pin_num, uint8_t setting, uint16_t value) {}
void uc_remote_conn() {}

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
    // Use repeated calls to fsm_isr() to run other things in the main loop
    fsm_poll();
}

// Reset all internal values, for use with new connections
void uc_reset()
{
    // Detach any servos
    uint8_t VALUE;
    for (int i = 0; i < num_DIO; i++)
    {
        VALUE = DIO_SET[i];
        if ((VALUE == IO_SERVO_US) || (VALUE == IO_SERVO_DEG)) DIO_SERVO[i].detach();
    }
    
    // Set DIOs to INPUTS
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

uint8_t uc_send(uint8_t* data, uint8_t data_len)
{
    return Serial.write(data, data_len);
}

// Read and return the DIO states
void uc_dio_read()
{
    // Setup variables
    uint16_t val;
    uint8_t j = 0;

    // Compose stage 1
    read_data[j++] = MAJOR_KEY_READ_RESPONSE; // Major Key
    read_data[j++] = MINOR_KEY_IO_DIO_READ;   // Minor Key
    read_data[j++] = dio_data_len;            // Num s2 bytes
    

    // Compose stage 2
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
    fsm_send((uint8_t*) read_data, j);
}

// Read and return the AIO states
void uc_aio_read()
{
    // Setup variables
    uint16_t val;
    uint8_t j = 0;
    
    // Compose stage 1
    read_data[j++] = MAJOR_KEY_READ_RESPONSE; // Major Key
    read_data[j++] = MINOR_KEY_IO_AIO_READ;   // Minor Key
    read_data[j++] = aio_data_len;            // Num s2 bytes

    // Compose stage 2
    for (uint8_t i = 0; i < num_AIO; i++)
    {
        // Scale value
        val = (uint16_t) (AIO_SCALE * analogRead(i));

        // Add pin value to list (convert to big endian)
        *((uint16_t*)(read_data + j)) = __builtin_bswap16(val);
        j += 2;
    }

    // Send data to GUI (use fsm_send to add checksum)
    fsm_send((uint8_t*) read_data, j);
}

// Set the DIO as per the command
void uc_dio(uint8_t pin_num, uint8_t setting, uint16_t value)
{
    if (DIO_SET[pin_num] != setting)
    {
        switch (DIO_SET[pin_num])
        {
            case IO_PWM:
                switch (pin_num)
                {
                    case 3:
                        TCCR2A &= ~(1 << COM2B1);
                        break;
                    case 5:
                        TCCR0A &= ~(1 << COM0B1);
                        break;
                    case 6:
                        TCCR0A &= ~(1 << COM0A1);
                        break;
                    case 9:
                        TCCR1A &= ~(1 << COM1A1);
                        break;
                    case 10:
                        TCCR1A &= ~(1 << COM1B1);
                        break;
                    case 11:
                        TCCR2A &= ~(1 << COM2A1);
                        break;
                    default:
                        break;
                }
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
                if (pin_num < 8) DDRD &= ~(1 << pin_num);
                else DDRB &= ~(1 << (pin_num - 8));
                break;
            case IO_PWM:
                switch (pin_num)
                {
                    case 3:
                        TCCR2A |= (1 << COM2B1);
                        break;
                    case 5:
                        TCCR0A |= (1 << COM0B1);
                        break;
                    case 6:
                        TCCR0A |= (1 << COM0A1);
                        break;
                    case 9:
                        TCCR1A |= (1 << COM1A1);
                        break;
                    case 10:
                        TCCR1A |= (1 << COM1B1);
                        break;
                    case 11:
                        TCCR2A |= (1 << COM2A1);
                        break;
                    default:
                        break;
                }
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
            if ((value != 0) && (value != 100))
            {
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
            if (value == 100)
            {
              value = 1;
            }
            // Fall through if 0 or 100 to set as digital pin
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

