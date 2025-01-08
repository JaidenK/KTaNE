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
 * @brief Sets up the pins and peripherals.
 * @author Max Dunne, 2013.09.15  */
void BOARD_Init(void);


#endif	/* BOARD_H */

