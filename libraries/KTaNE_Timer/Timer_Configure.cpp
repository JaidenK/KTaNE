
#include <Wire.h>
#include <EEPROM.h>
#include <Arduino.h>
#include <string.h>

#include "KTaNE.h"
#include "Timer_Configure.h"


/* EEPROM Addresses */
//#define EEPROM_TIME_LIMIT 0x20 // uint32_t Stored as seconds
//#define EEPROM_N_BATTERIES 0x24 // uint8_t
//#define EEPROM_SERIAL_NO 0x40 // String
//#define EEPROM_PORTS 0x70 // uint8_t bitfield
//#define EEPROM_INDICATORS 0x80 // uint8_t array of N_MAX_INDICATORS elements

uint32_t timeLimit = 0;
#define DEFAULT_TIME_LIMIT_S ((uint16_t)(60*15))

void LoadAllEepromConfigInfo()
{
    char *moduleName =  "THE TIMER       ";
    char *serialNo =    "TIM001          "; // This should be unique for every individual module. IDK how to ensure that right now.
    char *buildDate =   "12/17/2024 18:16"; // Bless your heart if you actually remember to update this datetime each time the code changes.
    for(uint8_t i=0; i<16; i++)
    {
        EEPROM.update(EEPROM_MODULE_NAME+i,moduleName[i]);
        EEPROM.update(EEPROM_REAL_MODULE_SERIAL+i,serialNo[i]); 
        EEPROM.update(EEPROM_BUILD_DATE+i,buildDate[i]); 
    }        
    if(getTimeLimist_s() == 0xFFFF)
    {
        EEPROM.update(EEPROM_TIMER_TIME_LIMIT_SECONDS, DEFAULT_TIME_LIMIT_S & 0xFF);
        EEPROM.update(EEPROM_TIMER_TIME_LIMIT_SECONDS+1, DEFAULT_TIME_LIMIT_S >> 8);
    }
}

uint16_t getTimeLimist_s()
{
    uint16_t timeLimit_s = 0;
    EEPROM.get(EEPROM_TIMER_TIME_LIMIT_SECONDS, timeLimit_s);
    return timeLimit_s;
}
