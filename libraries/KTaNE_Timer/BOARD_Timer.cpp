#include <Arduino.h>
#include <Wire.h>

#include "BOARD_Timer.h"
#include "KTaNE.h"
#include "TM1637Display.h"
#include "Clock.h"

/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/

/**
 * @brief Configures all required board peripherals and GPIO.
 */
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

    Clock_Reset();
}
