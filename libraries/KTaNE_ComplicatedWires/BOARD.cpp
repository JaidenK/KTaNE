/* 
 * File:   BOARD_ComplicatedWires.cpp
 * Author: Jaiden King
 *
 * Created on December 30, 2024, 7:21 PM
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
    pinMode(WIRE1_PIN,INPUT_PULLUP);
    pinMode(WIRE2_PIN,INPUT_PULLUP);
    pinMode(WIRE3_PIN,INPUT_PULLUP);
    pinMode(WIRE4_PIN,INPUT_PULLUP);
    pinMode(WIRE5_PIN,INPUT_PULLUP);
    pinMode(WIRE6_PIN,INPUT_PULLUP);

    pinMode(STRIKE_PIN,OUTPUT);
    pinMode(DISARM_PIN,OUTPUT);
    
    pinMode(LED1_PIN,OUTPUT);
    pinMode(LED2_PIN,OUTPUT);
    pinMode(LED3_PIN,OUTPUT);
    pinMode(LED4_PIN,OUTPUT);
    pinMode(LED5_PIN,OUTPUT);
    pinMode(LED6_PIN,OUTPUT);

    // Make sure LEDs and such are off
    digitalWrite(STRIKE_PIN,0);
    digitalWrite(DISARM_PIN,0);    
    digitalWrite(LED1_PIN,0);
    digitalWrite(LED2_PIN,0);
    digitalWrite(LED3_PIN,0);
    digitalWrite(LED4_PIN,0);
    digitalWrite(LED5_PIN,0);
    digitalWrite(LED6_PIN,0);
    
    // Read a floating analog pin to seed random number generator
    randomSeed(analogRead(A6));

    // Talk to PC
    Serial.begin(SERIAL_BAUD_RATE); // The baudrate of Serial monitor is set in 9600    

    // Join the bus
    KTaNE_I2C_Init(0);
}
