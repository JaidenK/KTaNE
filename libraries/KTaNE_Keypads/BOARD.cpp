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
 * @brief Sets up the pins and peripherals. 
 */
void BOARD_Init(void)
{
    // Configure pin directions
    pinMode(BTN1_PIN,INPUT_PULLUP);
    pinMode(BTN2_PIN,INPUT_PULLUP);
    pinMode(BTN3_PIN,INPUT_PULLUP);
    pinMode(BTN4_PIN,INPUT_PULLUP);

    pinMode(STRIKE_PIN,OUTPUT);
    pinMode(DISARM_PIN,OUTPUT);
    
    pinMode(LED1_PIN,OUTPUT);
    pinMode(LED2_PIN,OUTPUT);
    pinMode(LED3_PIN,OUTPUT);
    pinMode(LED4_PIN,OUTPUT);

    // Make sure LEDs and such are off
    digitalWrite(STRIKE_PIN,0);
    digitalWrite(DISARM_PIN,0);    
    digitalWrite(LED1_PIN,0);
    digitalWrite(LED2_PIN,0);
    digitalWrite(LED3_PIN,0);
    digitalWrite(LED4_PIN,0);
    
    // Read a floating analog pin to seed random number generator
    randomSeed(analogRead(A3));

    // Talk to PC
    Serial.begin(SERIAL_BAUD_RATE); // The baudrate of Serial monitor is set in 9600    

    // Join the bus
    KTaNE_I2C_Init(0);
}
