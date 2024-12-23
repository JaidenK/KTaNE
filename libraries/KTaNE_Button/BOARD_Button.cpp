/*
 * File:   BOARD.c
 * Author: Max
 *
 * Created on December 19, 2012, 2:08 PM
 */


#include <Arduino.h>
#include <Wire.h>

#include "BOARD_Button.h"
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
    pinMode(BTN_PIN,INPUT_PULLUP);
    pinMode(STRIKE_PIN,OUTPUT);
    pinMode(DISARM_PIN,OUTPUT);
    pinMode(PIN_RED,OUTPUT);
    pinMode(PIN_GREEN,OUTPUT);
    pinMode(PIN_BLUE,OUTPUT);
    digitalWrite(STRIKE_PIN,0);
    digitalWrite(DISARM_PIN,0);
    digitalWrite(PIN_RED,0);
    digitalWrite(PIN_GREEN,0);
    digitalWrite(PIN_BLUE,0);
    
    randomSeed(analogRead(A3));
    I2C_Init(0);
    Serial.begin(SERIAL_BAUD_RATE); // The baudrate of Serial monitor is set in 9600    
}

/**
 * @function BOARD_End(void)
 * @param None
 * @return None
 * @brief Shuts down all peripherals except for serial and A/D. Turns all pins into input
 * @author Max Dunne, 2013.09.20  */
void BOARD_End()
{

}

/**
 * @function BOARD_GetPBClock(void)
 * @param None
 * @return PB_CLOCK - Speed the peripheral clock is running in hertz
 * @brief Returns the speed of the peripheral clock.  Nominally at 40Mhz
 * @author Max Dunne, 2013.09.01  */
unsigned int BOARD_GetPBClock()
{
    //return PB_CLOCK;
    return 0;
}
