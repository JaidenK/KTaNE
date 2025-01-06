#ifndef _TIMER_CONFIGURE_H_
#define _TIMER_CONFIGURE_H_

#include <stdint.h>
#include "KTaNE_Common.h"
#include "KTaNE_Constants.h"

// HAL for the EEPROM

/**
 * @brief Initializes the EEPROM memory.
 * 
 * In the final product, this will only run
 * once ever: when the device is first powered on.
 * From then on it will have no effect because the 
 * values either don't change or aren't overwritten.
 * 
 * @note This does not read out anything from EEPROM.
 */
void TimerEEPROM_Initialize(void);

/**
 * @brief Reads the time limit value stored in EEPROM.
 * 
 * This is a player-defined value. Shown on the clock 
 * during the Idle state.
 * 
 * @returns Time limit (in seconds)
 */
uint16_t TimerEEPROM_GetTimeLimit_s(void);

#endif /* _TIMER_CONFIGURE_H_ */