#ifndef _TIMER_CONFIGURE_H_
#define _TIMER_CONFIGURE_H_

#include <stdint.h>

/* EEPROM Addresses */
#define EEPROM_SERIAL_NO 0x10 // String
#define EEPROM_TIME_LIMIT 0x20 // uint32_t Stored as seconds
#define EEPROM_N_BATTERIES 0x24 // uint8_t
#define EEPROM_LIT_INDIC 0x25 // uint8_t array of N_MAX_INDICATORS elements
#define EEPROM_PORTS 0x35 // uint8_t bitfield

extern uint32_t timeLimit;
void SetTimeLimitConfig(uint32_t newTimeLimit);

void LoadAllEepromConfigInfo();

#endif /* _TIMER_CONFIGURE_H_ */