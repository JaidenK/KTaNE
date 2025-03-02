
#include <Wire.h>
#include <EEPROM.h>
#include <Arduino.h>
#include <string.h>

#include "KTaNE.h"
#include "Timer_EEPROM.h"

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
void TimerEEPROM_WriteROM(void);
void TimerEEPROM_WriteDefaults(void);

/* EEPROM Addresses */
//#define EEPROM_TIME_LIMIT 0x20 // uint32_t Stored as seconds
//#define EEPROM_N_BATTERIES 0x24 // uint8_t
//#define EEPROM_SERIAL_NO 0x40 // String
//#define EEPROM_PORTS 0x70 // uint8_t bitfield
//#define EEPROM_INDICATORS 0x80 // uint8_t array of N_MAX_INDICATORS elements

uint32_t timeLimit = 0;
#define DEFAULT_TIME_LIMIT_S ((uint16_t)(60*15))

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

// TODO: Refactor this 
void TimerEEPROM_Initialize(void)
{
    TimerEEPROM_WriteROM();
    TimerEEPROM_WriteDefaults();
         
}

uint16_t TimerEEPROM_GetTimeLimit_s(void)
{
    uint16_t timeLimit_s = 0;
    EEPROM.get(EEPROM_TIMER_TIME_LIMIT_SECONDS, timeLimit_s);
    return timeLimit_s;
}

/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/

/**
 * @brief Sets the "read-only" values of the EEPROM
 * 
 * TODO This should be a KTaNE Library call
 */
void TimerEEPROM_WriteROM(void)
{
    // Ensure that "read-only" data is correct    
    KTaNE_InitEEPROM("THE TIMER       ",
                     "TIM001          ",
                     "01/08/2025 12:54");    
}

/**
 * @brief Ensures reasonable values are in the EEPROM on first bootup.
 */
void TimerEEPROM_WriteDefaults(void)
{ 
    if(TimerEEPROM_GetTimeLimit_s() == 0xFFFF)
    {
        EEPROM.update(EEPROM_TIMER_TIME_LIMIT_SECONDS, DEFAULT_TIME_LIMIT_S & 0xFF);
        EEPROM.update(EEPROM_TIMER_TIME_LIMIT_SECONDS+1, DEFAULT_TIME_LIMIT_S >> 8);
    }
}
