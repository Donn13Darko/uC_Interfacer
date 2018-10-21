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

#include "uc-generic-fsm.h"
#include "uc-generic-io.h"
#include "arduino-uno-uart-minor-keys.h"
#include "gui-pin-io-base-minor-keys.h"
#include "comms-general.h"
#include <Servo.h>

// Buffer Variables
const int len = 32;

// Setup pin watch for DIO
const uint8_t num_DIO = 14;
uint8_t DIO_SET[num_DIO];
uint16_t DIO_VAL[num_DIO];
Servo DIO_SERVO[num_DIO];
float PWM_SCALE = 255.0 / 100.0;

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
const uint8_t dio_data_len = 3*num_DIO + 1;
const uint8_t aio_data_len = 3*num_AIO + 1;
uint8_t read_data[dio_data_len];

// Just ignore these functions for now
void uc_data_transmit(uint8_t, const uint8_t*, uint8_t) {}
void uc_programmer(uint8_t, const uint8_t*, uint8_t) {}
void uc_custom_cmd(uint8_t, const uint8_t*, uint8_t) {}
void uc_aio(uint8_t pin_num, uint8_t setting, uint16_t value) {}
void uc_remote_conn() {}

// Arduino setup function
void setup()
{
    Serial.setTimeout(5000);
    Serial.begin(115200);
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
    // Set all DIO to INPUTs
    uint8_t VALUE;
    for (int i = 0; i < num_DIO; i++)
    {
        VALUE = DIO_SET[i];
        if ((VALUE == IO_SERVO_US) || (VALUE == IO_SERVO_DEG)) DIO_SERVO[i].detach();
        pinMode(i, INPUT);
    }

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
    uint16_t val;
    uint8_t j = 1;
    read_data[0] = MINOR_KEY_IO_DIO_READ;
    for (uint8_t i = 0; i < num_DIO; i++)
    {
        // Iterate over pins
        switch (DIO_SET[i])
        {
            case IO_INPUT:
                val = (digitalRead(i)) ? IO_ON : IO_OFF;
                break;
            case IO_SERVO_US:
            case IO_SERVO_DEG:
                val = DIO_SERVO[i].read();
                break;
            case IO_PWM:
            case IO_OUTPUT:
                val = DIO_VAL[i];
                break;
        }

        read_data[j++] = i;
        read_data[j++] = (uint8_t) ((val >> 8) & 0xFF);
        read_data[j++] = (uint8_t) (val & 0xFF);
    }

    // Send data to GUI
    uc_send((uint8_t*) read_data, dio_data_len);
}

// Read and return the AIO states
void uc_aio_read()
{
    uint8_t j = 1;
    uint16_t val = 0;
    read_data[0] = MINOR_KEY_IO_AIO_READ;
    for (uint8_t i = 0; i < num_AIO; i++)
    {
        // Scale value
        val = (uint16_t) (AIO_SCALE * analogRead(i));

        read_data[j++] = i;
        read_data[j++] = (uint8_t) ((val >> 8) & 0xFF);
        read_data[j++] = (uint8_t) (val & 0xFF);
    }

    // Send data to GUI
    uc_send((uint8_t*) read_data, aio_data_len);
}

// Set the DIO as per the command
void uc_dio(uint8_t pin_num, uint8_t setting, uint16_t value)
{
    // Set follow up packets to IO and pin val
    uint8_t PIN = pin_num;
    uint8_t IO = setting;
    uint16_t PIN_VAL = value;

    if (DIO_SET[PIN] != IO)
    {
        switch (DIO_SET[PIN])
        {
            case IO_SERVO_US:
            case IO_SERVO_DEG:
                DIO_SERVO[PIN].detach();
                break;
            default:
                break;
        }
        
        switch (IO)
        {
            case IO_INPUT:
                pinMode(PIN, INPUT);
                break;
            case IO_OUTPUT:
                pinMode(PIN, OUTPUT);
                break;
            case IO_PWM:
                pinMode(PIN, OUTPUT);
                break;
            case IO_SERVO_US:
            case IO_SERVO_DEG:
                DIO_SERVO[PIN].attach(PIN);
                break;
            default:
                return;
        }
        
        DIO_SET[PIN] = IO;
    }

    switch (DIO_SET[PIN])
    {
        case IO_OUTPUT:
            digitalWrite(PIN, PIN_VAL);
            break;
        case IO_PWM:
            analogWrite(PIN, (int) (PWM_SCALE * (float) PIN_VAL));
            break;
        case IO_SERVO_US:
            DIO_SERVO[PIN].writeMicroseconds(PIN_VAL);
            break;
        case IO_SERVO_DEG:
            DIO_SERVO[PIN].write(PIN_VAL);
            break;
        default:
            return;
    }

    DIO_VAL[PIN] = PIN_VAL;
}

