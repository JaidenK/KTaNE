/*

Constants.h

02/21/2020
Jaiden King

Constants used between all modules for KTaNE.

*/

#ifndef _KTANE_CONSTANTS_H_
#define _KTANE_CONSTANTS_H_

#define TRUE 1
#define FALSE 0

#define SYNC_BYTE_A 0xA5 // Byte used to identify if an address is used by multiple modules 
#define SYNC_BYTE_B 0xF0
#define N_SYNC_BYTES 8 // How many sync bytes to send

#define EEPROM_MODULE_NAME        0x00 // Name of module. 16 ASCII chars. "readonly"
#define EEPROM_REAL_MODULE_SERIAL 0x10 // Unique identifier of this module. 16 ASCII chars. "readonly"
#define EEPROM_BUILD_DATE         0x20 // 16 ASCII chars. "readonly" format "MM/DD/YYYY HH:MM"
#define END_OF_PROTECTED_EEPROM   0x30 // EEPROM prior to this point cannot be modified by commands from the PC
#define EEPROM_CONFIG_DATE        0x30 // Set by the PC when the user reconfigures the bomb.
#define EEPROM_I2C_ADDRESS        0x40 // This module's I2C address. Used so that the system is predictable when it's power cycled.

#define EEPROM_TIMER_BOMB_SERIAL_NO     0x50
#define EEPROM_TIMER_TIME_LIMIT_SECONDS 0x60 // uint16_t
#define EEPROM_TIMER_N_AA_BATTERIES     0x62 // uint8_t 
#define EEPROM_TIMER_N_D_BATTERIES      0x63 // uint8_t 
#define EEPROM_TIMER_PORTS              0x64 // bitfield (6 bits) order matches defusal manual appendix LSB: DVI-D, MSB: Stero RCA
#define EEPROM_TIMER_INDICATORS         0x70 // null-terminated string

#define SERIAL_BAUD_RATE 9600

#define GENERAL_CALL_I2C_ADDRESS 0x00
#define TIMER_I2C_ADDRESS 0x01 // Fixed address of the timer module

#define SERIAL_NO_MAX_LENGTH 16

#define STRIKE_LED_OCR1A 31250 // How many counts of the 256 prescaled timer that the strike LED should be on after a strike

#define N_CONSOLE_ROWS 24
#define N_CONSOLE_COLUMNS 80

#define I2C_WAITING_LED_PERIOD 70 // The LEDs on the modules should blink while waiting for the i2c to connect

#define N_MAX_MODULE_ID_CHARS 30
#define N_MAX_MODULE_NAME_CHARS 30
#define N_MAX_MODULES 12

#define INDICATOR_MAX_LENGTH 3 // Maximum number of characters in an indicator
#define N_MAX_INDICATORS 12 // Maximum number of indicators supported in a system
#define N_MAX_PORTS 12

typedef enum
{
    INVALID_PORT = 0,
    DVI_D,
    PARALLEL,
    PS_2,
    RJ_45,
    SERIAL,
    STEREO_RCA
}
Port_t;

#endif /* _KTANE_CONSTANTS_H_ */
