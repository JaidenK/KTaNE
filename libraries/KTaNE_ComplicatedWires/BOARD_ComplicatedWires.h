/* 
 * File:   BOARD_ComplicatedWires.h
 * Author: Jaiden King
 *
 * Created on December 30, 2024, 7:21 PM
 */

#ifndef BOARD_H
#define	BOARD_H

#include <stdint.h>
#include "KTaNE_Constants.h"

/*******************************************************************************
 * PUBLIC #DEFINES                                                             *
 ******************************************************************************/
#define ERROR ((int8_t) -1)
#define SUCCESS ((int8_t) 1)

/* IO PINS */
// KTaNE Standard Pins
#define STRIKE_PIN 12
#define DISARM_PIN 13

// Inputs 
#define WIRE1_PIN 7
#define WIRE2_PIN 6
#define WIRE3_PIN 5
#define WIRE4_PIN 4
#define WIRE5_PIN 3
#define WIRE6_PIN 2

// LED pins
#define LED1_PIN 8
#define LED2_PIN 9
#define LED3_PIN 10
#define LED4_PIN 15
#define LED5_PIN 16
#define LED6_PIN 17

/* EEPROM ADDRESSES */
#define EEPROM_REDWIRES 0x1E0
#define EEPROM_BLUEWIRES 0x1E1
#define EEPROM_STARS 0x1E2
#define EEPROM_LEDS 0x1E3
// See also:
// EEPROM_TIMER_BOMB_SERIAL_NO
// EEPROM_TIMER_PORTS
// EEPROM_TIMER_N_AA_BATTERIES
// EEPROM_TIMER_N_D_BATTERIES

/**
 * @function BOARD_Init(void)
 * @param None
 * @return None
 * @brief Sets up the pins and peripherals. 
 */
void BOARD_Init(void);

#endif	/* BOARD_H */

