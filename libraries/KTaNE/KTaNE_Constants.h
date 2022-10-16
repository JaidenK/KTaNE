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

#define EEPROM_I2C_ADDRESS 0x00 // Location in EEPROM where the module's address is stored.
#define EEPROM_CONFIGURED_ADDRESS 0x01 // Location in EEPROM where the bit stating whether the module has been configured or not is stored.
#define SERIAL_BAUD_RATE 9600

#define TIMER_I2C_ADDRESS 0x00 // Fixed address of the timer module
#define PROGRAMMER_I2C_ADDRESS 0x7F // Fixed address of the programmer

#define SERIAL_NO_LENGTH 6

#define STRIKE_LED_OCR1A 31250 // How many counts of the 256 prescaled timer that the strike LED should be on after a strike

#define N_CONSOLE_ROWS 24
#define N_CONSOLE_COLUMNS 80

#define I2C_WAITING_LED_PERIOD 70 // The LEDs on the modules should blink while waiting for the i2c to connect

#define N_MAX_MODULE_ID_CHARS 30
#define N_MAX_MODULE_NAME_CHARS 30
#define N_MAX_MODULES 12


#endif /* _KTANE_CONSTANTS_H_ */
