/*
 * File:   BOARD.c
 * Author: Max
 *
 * Created on December 19, 2012, 2:08 PM
 */


#include <Arduino.h>
#include <Wire.h>

#include "BOARD.h"
#include "KTaNE_Constants.h"
#include "KTaNE_Common.h"

/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/

//#define SYSTEM_CLOCK 80000000L
//#define  PB_CLOCK SYSTEM_CLOCK/2
//#define TurnOffAndClearInterrupt(Name) INTEnable(Name,INT_DISABLED); INTClearFlag(Name)


/**
 * @function BOARD_Init(void)
 * @param None
 * @return None
 * @brief Set the clocks up for the board, initializes the serial port, and turns on the A/D
 *        subsystem for battery monitoring
 * @author Max Dunne, 2013.09.15  */
void BOARD_Init()
{
    randomSeed(analogRead(A3));

    Serial.begin(SERIAL_BAUD_RATE); // The baudrate of Serial monitor is set in 9600
    
    pinMode(STRIKE_BTN,INPUT_PULLUP);
    pinMode(DISARM_BTN,INPUT_PULLUP);
    pinMode(STRIKE_PIN,OUTPUT);
    pinMode(DISARM_PIN,OUTPUT);
    digitalWrite(STRIKE_PIN,1);
    digitalWrite(DISARM_PIN,1);
    
    KTaNE_I2C_Init(0);
}
