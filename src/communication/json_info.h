#ifndef JSON_INFO_H
#define JSON_INFO_H

// Keys enum
typedef enum {
    // Reset the state to start values
    JSON_RESET = 0,

    // Confirm actions
    JSON_COPY,
    JSON_SUCCESS,
    JSON_FAILURE,

    // Read information from device
    JSON_READ,

    // Remote pin settings/commands
    JSON_DIO,
    JSON_AIO,

    // Remote UART/I2C connection settings
    JSON_REMOTE_CONN,

    // Send file
    JSON_FILE,

    // Programming
    JSON_PROGRAM,

    // Start and end for efficiently grouping data
    JSON_START,
    JSON_END
} JSON_KEYS;

// IO settings
typedef enum {
    IO_OFF = 0,
    IO_ON = 1,
    IO_INPUT,
    IO_OUTPUT,
    IO_PWM,
    IO_SERVO_US,
    IO_SERVO_DEG
} IO_INFO;

// Remote connection settings
typedef enum {
    REMOTE_CONN_REMOTE = 0,

    REMOTE_CONN_CONNECT,
    REMOTE_CONN_DISCONNECT,
    REMOTE_CONN_SEND,

    REMOTE_CONN_UART,
    REMOTE_CONN_SPI,
    REMOTE_CONN_I2C,

    REMOTE_CONN_SET_TX,
    REMOTE_CONN_SET_RX,
    REMOTE_CONN_SET_TX_RX,
    REMOTE_CONN_SET_NONE
} REMOTE_CONN;

// Programming settings
typedef enum {
    // Programming helper keys
    PROGRAMNING_INFO_START = 0,
    PROGRAMNING_INFO_END,
    PROGRAMNING_INFO_ADDRESS,
    PROGRAMNING_INFO_DATA,
    PROGRAMNING_INFO_READ,

    // Various programming modes
    /** MUST STAY HERE **/
    PROGRAMMING_INFO_MODE,
    /** MUST STAY HERE **/
    PROGRAMMING_INFO_ICSP
} PROGRAMMING_INFO;

#endif // JSON_INFO_H
