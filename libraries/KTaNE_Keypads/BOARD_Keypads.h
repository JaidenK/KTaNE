/* 
 * File:   BOARD.h
 * Author: Max Dunne
 *
 * Created on December 19, 2012, 2:08 PM
 */

#ifndef BOARD_H
#define	BOARD_H

#include <stdint.h>
#include "KTaNE_Constants.h"

/*******************************************************************************
 * PUBLIC #DEFINES                                                             *
 ******************************************************************************/

//suppresses various warnings that we don't need to worry about for CMPE13
#ifndef _SUPPRESS_PLIB_WARNING
#define _SUPPRESS_PLIB_WARNING
#endif

#ifndef _DISABLE_OPENADC10_CONFIGPORT_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING
#endif

/*****************************************************************************/
// Boolean defines for TRUE, FALSE, SUCCESS and ERROR
#ifndef FALSE
#define FALSE ((int8_t) 0)
#define TRUE ((int8_t) 1)
#endif
#define ERROR ((int8_t) -1)
#define SUCCESS ((int8_t) 1)

/* IO PINS */
#define STRIKE_PIN 12
#define DISARM_PIN 13

// Numbering is from left-to-right, top-to-bottom:
//  
//   1    2
//  
//   3    4
//

#define BTN1_PIN 2
#define BTN2_PIN 3
#define BTN3_PIN 5
#define BTN4_PIN 6

// LED pins
#define LED1_PIN 9
#define LED2_PIN 10
#define LED3_PIN 8
#define LED4_PIN 7

/* EEPROM ADDRESSES */
#define EEPROM_KEY1 0x1E0
#define EEPROM_KEY2 0x1E1
#define EEPROM_KEY3 0x1E2
#define EEPROM_KEY4 0x1E3


/**
 * @function BOARD_Init(void)
 * @param None
 * @return None
 * @brief Set the clocks up for the board, initializes the serial port, and turns on the A/D
 *        subsystem for battery monitoring
 * @author Max Dunne, 2013.09.15  */
void BOARD_Init();


/**
 * @function BOARD_End(void)
 * @param None
 * @return None
 * @brief Shuts down all peripherals except for serial and A/D. Turns all pins into input
 * @author Max Dunne, 2013.09.20  */
void BOARD_End();

/**
 * @function BOARD_GetPBClock(void)
 * @param None
 * @return PB_CLOCK - Speed the peripheral clock is running in hertz
 * @brief Returns the speed of the peripheral clock.  Nominally at 40Mhz
 * @author Max Dunne, 2013.09.01  */
unsigned int BOARD_GetPBClock();

#endif	/* BOARD_H */

