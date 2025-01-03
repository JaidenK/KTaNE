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

#define BTN_PIN 10

// RGB LED pins
#define PIN_RED   9
#define PIN_GREEN 6
#define PIN_BLUE  5

/* EEPROM ADDRESSES */
#define EEPROM_BUTTON_LABEL 0x1E0
#define BTN_LABEL_OTHER    0
#define BTN_LABEL_ABORT    1
#define BTN_LABEL_DETONATE 2
#define BTN_LABEL_HOLD     3

#define EEPROM_BUTTON_BTN_COLOR 0x1E1
#define BTN_COLOR_OTHER  0
#define BTN_COLOR_BLUE   1
#define BTN_COLOR_WHITE  2
#define BTN_COLOR_YELLOW 3
#define BTN_COLOR_RED    4

#define EEPROM_BUTTON_STRIP_COLOR 0x1E2
#define STRIP_COLOR_RANDOM 0
#define STRIP_COLOR_RED    1
#define STRIP_COLOR_YELLOW 2
#define STRIP_COLOR_GREEN  3
#define STRIP_COLOR_CYAN   4
#define STRIP_COLOR_BLUE   5
#define STRIP_COLOR_MAGENTA 6
#define STRIP_COLOR_WHITE  7


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

