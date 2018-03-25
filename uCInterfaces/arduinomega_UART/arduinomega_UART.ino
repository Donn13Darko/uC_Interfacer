#include <Wire.h>
#include "../C_Helpers/json_structs.h"

// Buffer Variables
const int len = 128;
char data[len];
int i = 0;

// Reading Variables
int readLen;
char* str;

// Setup pin watch for DIO
const int num_DIO = 14;
char DIO_SET[num_DIO];
char DIO_VAL[num_DIO];
float PWM_SCALE = 255.0 / 100.0;

// Setup pin watch for AIO
const int num_AIO = 6;
// No char array since no Analog outputs

// Setup map values for AIO (0-5V)
float AIO_LOW = 0.0;
float AIO_HIGH = 5.0;
float AIO_RES = 1024.0;
float AIO_SCALE = (AIO_HIGH - AIO_LOW) / AIO_RES;

// Setup infor for I2C connections
bool i2cConnected = false;

// Function prototypes
void parseReceived();
void READ_INFO();
void SET_DIO();
void SET_AIO();
void RESET_VALUES();
void RETURN_RECEIVED();
void SET_CONN();
void SEND_CONN();
void DISCONNECT();

// Arduino setup function
void setup()
{
    Serial.setTimeout(10000);
    Serial.begin(19200);

    RESET_VALUES();
}

// Arduino Loop function
void loop()
{
    // Wait for serial data to be available
    while (Serial.available() > 0)
    {
        // Read data until newline char
        readLen = Serial.readBytesUntil('\n', data, sizeof(data));
        if (readLen != 0)
        {
          parseReceived();

          // Set buffer back to 0 after done parsing
          memset(data, 0, sizeof(data));
          i = 0;
        }
    }

    delay(10);
}

// Reset all internal values, for use with new connections
void RESET_VALUES()
{
    memset(data, 0, sizeof(data));
    memset(DIO_SET, DIO::INPUT, sizeof(DIO));
    memset(DIO_VAL, DIO::OFF, sizeof(DIO));

    // Set all DIO to INPUTs
    for (int i = 0; i < num_DIO; i++)
    {
        pinMode(i, INPUT);
    }

    i = 0;
}

// Parse the received data and set hardware accordingly
void parseReceived()
{
    char key = data[i];
    i = i + 1;

    switch (key)
    {
        case json_keys::read_values:
            READ_INFO();
            break;
        case json_keys::set_DIO:
            SET_DIO();
            break;
        case json_keys::set_AIO:
            SET_AIO();
            break;
        case json_keys::remote_connect:
            SET_CONN();
            break;
        case json_keys::remote_send:
            SEND_CONN();
            break;
        case json_keys::reset:
            RESET_VALUES();
            break;
        default:
            RETURN_RECEIVED();
            break;
    }
}

// Read and return the requested information
void READ_INFO()
{
    value = data[i];
    Serial.print(json_keys::read_values);
    switch (value)
    {
        case READ:AIO:
        {
            Serial.print(READ::AIO);
            float val = 0.0;
            for (int i = 0; i < num_AIO; i++)
            {
                val = AIO_SCALE * analogRead(i);
                Serial.print(val);
            }
            Serial.print("\n");
            break;
        }
        case READ::DIO:
        {
            Serial.print(READ::DIO);
            char val;
            for (int i = 0; i < num_DIO; i++)
            {
                if (DIO_SET[i] == DIO::INPUT) val = (digitalRead(i)) ? DIO::ON : DIO::OFF;
                else val = DIO_VAL[i];
                Serial.print(val);
            }
            Serial.print("\n");
            break;
        }
        default:
            RETURN_RECEIVED();
            break;
    }
}

// Set the DIO as per the command
void SET_DIO()
{
    if (strlen(&data) < (i+2))
    {
        RETURN_RECEIVED();
        return;
    }

    char PIN = data[i+0];
    char IO = data[i+1];
    char VAL = data[i+2];
    i = i + 3;

    int p = atoi(PIN);
    int v = atoi(VAL);
    if (DIO_SET[p] != IO)
    {
        switch (IO)
        {
            case DIO::INPUT:
                pinMode(p, INPUT);
                DIO_SET[p] = DIO::INPUT;
                break;
            case DIO::OUTPUT:
                pinMode(p, OUTPUT);
                DIO_SET[p] = DIO::OUTPUT;
                break;
            case DIO::PWM:
                pinMode(p, OUTPUT);
                DIO_SET[p] = DIO::PWM;
                break;
            default:
                RETURN_RECEIVED();
                break;
        }
    }

    if (DIO_VAL[p] == DIO::OUTPUT) digitalWrite(p, v);
    else if (DIO_VAL[p] == DIO::PWM) analogWrite(p, (int) (PWM_SCALE * (float) v));
    else RETURN_RECEIVED();
}

// Empty for the Arduino Uno since no Analog out other than PWM
// (Could setup analog pins as digital outputs put no need)
void SET_AIO()
{
    RETURN_RECEIVED();
    return;
}

void RETURN_RECEIVED()
{
    Serial.print(data);
    Serial.print('\n');
}

void SET_CONN()
{
    char func = data[i];
    i = i + 1;

    switch (func)
    {
        case REMOTE_CONN::disconnect:
            DISCONNECT();
            break;
        case REMOTE_CONN::UART:
            // UART Stuff
            break;
        case REMOTE_CONN::SPI:
            // SPI Stuff
            break;
        case REMOTE_CONN::I2C:
            // I2C Stuff
            break;
        default:
            RETURN_RECEIVED();
            break;
    }
}

void SEND_CONN()
{
    // Send stuff across connection (must already have connected)
}

void DISCONNECT()
{
    char conn = data[i];
    i = i + 1;

    switch (conn)
    {
        case REMOTE_CONN::UART:
            // UART Stuff
            break;
        case REMOTE_CONN::SPI:
            // SPI Stuff
            break;
        case REMOTE_CONN::I2C:
            // I2C Stuff
            break;
        default:
            RETURN_RECEIVED();
            break;
    }
}
