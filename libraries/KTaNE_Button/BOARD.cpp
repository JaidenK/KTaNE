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

/**
 * @function BOARD_Init(void)
 * @param None
 * @return None
 * @brief Set the clocks up for the board, initializes the serial port, and turns on the A/D
 *        subsystem for battery monitoring
 * @author Max Dunne, 2013.09.15  */
void BOARD_Init()
{
    // Configure pin directions
    pinMode(BTN_PIN,INPUT_PULLUP);
    pinMode(STRIKE_PIN,OUTPUT);
    pinMode(DISARM_PIN,OUTPUT);
    pinMode(PIN_RED,OUTPUT);
    pinMode(PIN_GREEN,OUTPUT);
    pinMode(PIN_BLUE,OUTPUT);

    // Make sure LEDs and such are off
    digitalWrite(STRIKE_PIN,0);
    digitalWrite(DISARM_PIN,0);
    digitalWrite(PIN_RED,0);
    digitalWrite(PIN_GREEN,0);
    digitalWrite(PIN_BLUE,0);
    
    // Read a floating analog pin to seed random number generator
    randomSeed(analogRead(A3));

    // Talk to PC
    Serial.begin(SERIAL_BAUD_RATE); // The baudrate of Serial monitor is set in 9600    

    // Join the bus
    KTaNE_I2C_Init(0);
}
