#include <json_info.h>
#include <Wire.h>
#include <Servo.h>

// Buffer Variables
const int len = 32;
uint8_t data[len];
uint8_t KEY, VALUE;

// Reading Variables
uint8_t readLen;

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

// Setup info for I2C connections
bool i2cConnected = false;

// Setup info for Programming
uint8_t PROG_MODE = PROGRAMMING_INFO_ICSP;

// Function prototypes
uint8_t READ_NEXT(uint8_t rdata[], uint8_t n, uint32_t timeout);
void PARSE();
void READ_INFO();
void SET_DIO();
void SET_AIO();
void RESET_VALUES();
void RETURN_RECEIVED();
void REMOTE_SET();
void REMOTE_CONNECT();
void REMOTE_SEND();
void REMOTE_DISCONNECT();
void RECV_FILE();
void RECV_PROGRAM();

// Arduino setup function
void setup()
{
    Serial.setTimeout(5000);
    Serial.begin(115200);

    RESET_VALUES();
}

// Arduino Loop function
void loop()
{
    // Read key/value pair (will wait for up to 1 seconds)
    if (READ_NEXT(data, 2, 1000) == 2)
    {
        KEY = data[0];
        VALUE = data[1];

        // Parse values
        PARSE();

        // Set buffer back to 0 after done parsing
        memset(data, 0, sizeof(data));
    }
}

// Reset all internal values, for use with new connections
void RESET_VALUES()
{
    // Set all DIO to INPUTs
    for (int i = 0; i < num_DIO; i++)
    {
        VALUE = DIO_SET[i];
        if ((VALUE == IO_SERVO_US) || (VALUE == IO_SERVO_DEG)) DIO_SERVO[i].detach();
        pinMode(i, INPUT);
    }

    // Reset buffered data
    memset(data, 0, sizeof(data));
    memset(DIO_SET, IO_INPUT, sizeof(DIO_SET));
    memset(DIO_VAL, IO_OFF, sizeof(DIO_VAL));

    // Reset and flush buffers
    KEY = 0;
    VALUE = 0;
    Serial.flush();
    while (Serial.available()) { Serial.read(); }
}

// Read the next n bytes into rdata and return if successful
uint8_t READ_NEXT(uint8_t rdata[], uint8_t n, uint32_t timeout)
{
    uint8_t delay_time = 10;
    uint32_t curr_time = 0;
    
    // Wait until all bytes loaded into serial buffer or timeout
    while (Serial.available() < n)
    {
        delay(delay_time);
        curr_time += delay_time;
        if (timeout < curr_time) return false;
    }

    // Read bytes and return num read
    return Serial.readBytes(rdata, n);
}

// Parse the received data and set hardware accordingly
void PARSE()
{
    switch (KEY)
    {
        case JSON_READ:
            READ_INFO();
            break;
        case JSON_DIO:
            SET_DIO();
            break;
        case JSON_AIO:
            SET_AIO();
            break;
        case JSON_REMOTE_CONN:
            REMOTE_SET();
            break;
        case JSON_FILE:
            RECV_FILE();
            break;
        case JSON_PROGRAM:
            RECV_PROGRAM();
            break;
        case JSON_RESET:
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
    Serial.write(JSON_READ);
    switch (VALUE)
    {
        case JSON_AIO:
        {
            Serial.write(JSON_AIO);
            uint16_t val = 0.0;
            for (uint8_t i = 0; i < num_AIO; i++)
            {
                // Scale value
                val = (uint16_t) (AIO_SCALE * analogRead(i));

                // Send pin num + value to GUI
                Serial.write(i);
                Serial.write((uint8_t) ((val >> 8) & 0xFF));
                Serial.write((uint8_t) (val & 0xFF));
            }
            break;
        }
        case JSON_DIO:
        {
            Serial.write(JSON_DIO);
            uint16_t pos = 0;
            for (uint8_t i = 0; i < num_DIO; i++)
            {
                // Iterate over pins
                switch (DIO_SET[i])
                {
                    case IO_INPUT:
                        pos = (digitalRead(i)) ? IO_ON : IO_OFF;
                        break;
                    case IO_SERVO_US:
                    case IO_SERVO_DEG:
                        pos = DIO_SERVO[i].read();
                        break;
                    case IO_PWM:
                    case IO_OUTPUT:
                        pos = DIO_VAL[i];
                        break;
                }

                // Send pin num + value to GUI
                Serial.write(i);
                Serial.write((uint8_t) ((pos >> 8) & 0xFF));
                Serial.write((uint8_t) (pos & 0xFF));
            }
            break;
        }
        default:
            RETURN_RECEIVED();
            break;
    }
    Serial.write(JSON_READ);
    Serial.write(JSON_END);
}

// Set the DIO as per the command
void SET_DIO()
{
    // Set current value to PIN
    uint8_t PIN = VALUE;

    // Read follow up packets (blocks for up to 1 second)
    if (READ_NEXT(data, 3, 1000) != 3) return;

    // Set follow up packets to IO and pin val
    uint8_t IO = (uint8_t) data[0];
    uint16_t PIN_VAL = (uint16_t) ((((uint16_t) data[1]) << 8) | data[2]);

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
                RETURN_RECEIVED();
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
            RETURN_RECEIVED();
            return;
    }

    DIO_VAL[PIN] = PIN_VAL;
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
    Serial.write(KEY);
    Serial.write(VALUE);
}

void REMOTE_SET()
{
    switch (VALUE)
    {
        case REMOTE_CONN_SET_TX:
        case REMOTE_CONN_SET_RX:
        case REMOTE_CONN_SET_TX_RX:
        case REMOTE_CONN_SET_NONE:
            // Set action here
            break;
        case REMOTE_CONN_DISCONNECT:
            REMOTE_DISCONNECT();
            break;
        case REMOTE_CONN_SEND:
            REMOTE_SEND();
            break;
        default:
            RETURN_RECEIVED();
            break;
    }
}

void REMOTE_CONNECT()
{
    switch (VALUE)
    {
        case REMOTE_CONN_UART:
            // UART Stuff
            break;
        case REMOTE_CONN_SPI:
            // SPI Stuff
            break;
        case REMOTE_CONN_I2C:
            // I2C Stuff
            break;
        default:
            RETURN_RECEIVED();
            break;
    }
}

void REMOTE_SEND()
{
    // Send stuff across connection (must already have connected)
}

void REMOTE_DISCONNECT()
{
    switch (VALUE)
    {
        case REMOTE_CONN_UART:
            // UART Stuff
            break;
        case REMOTE_CONN_SPI:
            // SPI Stuff
            break;
        case REMOTE_CONN_I2C:
            // I2C Stuff
            break;
        default:
            RETURN_RECEIVED();
            break;
    }
}

void RECV_FILE()
{
    // Set buffer back to 0 after done parsing
    memset(data, 0, sizeof(data));
    uint8_t rdLen = READ_NEXT (data, len, 1000);

    // Parse data and read next set until FILE END or RESET set
    // TO DO: add custom code defining what to do with received data
    while (rdLen != 0)
    {
        for (uint8_t i = 0; i < (rdLen - 1); i++)
        {
            if ((data[i] == JSON_FILE) && (data[i+1] == JSON_END))
              return;
            else if ((data[i] == JSON_RESET)
              && ((data[i+1] == JSON_START) || (data[i+1] == JSON_END)))
              return;
        }

        rdLen = READ_NEXT (data, len, 1000);
    }
}

void RECV_PROGRAM()
{
    if (VALUE != PROGRAMNING_INFO_START)
    {
        PROG_MODE = PROGRAMMING_INFO_MODE;
        return;
    }

    uint8_t dataLen;
    while (VALUE != PROGRAMNING_INFO_END)
    {
        // Read next key set (or data if prescribed)
        memset(data, 0, sizeof(data));
        if (READ_NEXT(data, 2, 1000) == 2)
        {
            KEY = data[0];
            VALUE = data[1];

            // If we don't receive JSON_PROGRAM as key error out
            if (KEY != JSON_PROGRAM)
            {
                Serial.write((uint8_t) JSON_COPY);
                Serial.write((uint8_t) JSON_FAILURE);
                Serial.flush();
                RESET_VALUES();
                return;
            }

            // Switch on the value received
            switch(VALUE)
            {
                case PROGRAMNING_INFO_ADDRESS:
                case PROGRAMNING_INFO_DATA:
                    // Read size argument
                    memset(data, 0, sizeof(data));
                    if (READ_NEXT(data, 1, 1000) != 1) break;
                    dataLen = data[0];
                    
                    // Read next data
                    memset(data, 0, sizeof(data));
                    if (READ_NEXT(data, dataLen, 1000) != dataLen) break;

                    // Act on next data (program to device)

                    // Respond to data
                    Serial.write((uint8_t) JSON_COPY);
                    Serial.write((uint8_t) JSON_SUCCESS);
                    Serial.flush();
                    break;
            }
        }
    }
}

