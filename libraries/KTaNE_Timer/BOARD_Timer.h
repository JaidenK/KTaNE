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
#include "TM1637Display.h"

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
//#define FALSE ((int8_t) 0)
//#define TRUE ((int8_t) 1)
#endif
#define ERROR ((int8_t) -1)
#define SUCCESS ((int8_t) 1)

/* EEPROM Addresses */
#define EEPROM_SERIAL_NO 0x10 // String
#define EEPROM_TIME_LIMIT 0x20 // uint8_t Stored as minutes
#define EEPROM_N_BATTERIES 0x24 // uint8_t
#define EEPROM_LIT_INDIC 0x25 // uint8_t array of N_MAX_INDICATORS elements
#define EEPROM_PORTS 0x35 // uint8_t bitfield


/* IO PINS */
#define SPEAKER_PIN 9
#define START_BTN 12

#define TM1637_CLK 8
#define TM1637_DIO 7

#define LIGHT_PIN A3

#define STRIKE1_PIN 2
#define STRIKE2_PIN 3


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

