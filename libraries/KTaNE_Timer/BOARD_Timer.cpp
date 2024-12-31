/*
 * File:   BOARD.c
 * Author: Max
 *
 * Created on December 19, 2012, 2:08 PM
 */


#include <Arduino.h>
#include <Wire.h>

#include "BOARD_Timer.h"
#include "KTaNE.h"
#include "TM1637Display.h"
#include "ClockEventChecker.h"

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
    //I2C_Init(TIMER_I2C_ADDRESS);
    KTaNE_I2C_Init(-1);
    Serial.begin(SERIAL_BAUD_RATE); // The baudrate of Serial monitor is set in 9600
    
    pinMode(SPEAKER_PIN,OUTPUT);
    pinMode(STRIKE1_PIN,OUTPUT);
    pinMode(STRIKE2_PIN,OUTPUT);
    pinMode(CONSEQUENCE_PIN,OUTPUT);
    digitalWrite(SPEAKER_PIN,0);
    digitalWrite(STRIKE1_PIN,0);
    digitalWrite(STRIKE2_PIN,0);
    digitalWrite(CONSEQUENCE_PIN,0);

    ClearClock();
    showTime(0);
}
