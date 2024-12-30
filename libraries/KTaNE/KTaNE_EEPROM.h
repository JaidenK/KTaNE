
#ifndef _KTANE_EEPROM_H_
#define _KTANE_EEPROM_H_

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
//#define EEPROM_N_MODULES                0x65 // Number of modules (automatically determined by timer at runtime)
#define EEPROM_TIMER_INDICATORS         0x70 // null-terminated string. 

// This block of the Timer's EEPROM will be replicated on
// all modules to provide bomb config info.
#define EEPROM_REPEATED_MEMORY_START 0x50
#define REPEATED_MEMORY_LENGTH 0x40

#endif
