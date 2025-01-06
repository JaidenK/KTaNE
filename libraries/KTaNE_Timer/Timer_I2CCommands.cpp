#include <Arduino.h>
#include <Wire.h>
#include "KTaNE.h"
#include "ES_Framework.h"

// These are unused functions that require an implementation from 
// the KTaNE library.

// This is the function that's being called on the other module
// when the Timer makes a request on the I2C bus.
void Module_ServiceI2CRequest(I2C_CommandPacket *pkt)
{
    (void)pkt; // Acknowledge unused parameter
}

// This is called when the timer is sent a message on the I2C bus from
// another I2C master. This should never happen.
//* Idea: this could be where we display and error on the display?
//*       "Error another I2C bus master has been found. Continue? Y/N"
//*       Useful for people who want to make their own modules?
//* Idea: Write abstraction layer for providing user feedback. We could
//*       have a DisplayError() function that enapsulates interacting
//*       with the Clock, and facilitate expanding to more sophisticated
//*       logging in the future.
void Module_ReceiveI2CCommand(I2C_CommandPacket *pkt, uint8_t length)
{
    (void)length; // Acknowledge unused parameter
    
    // This function should be dead code.
    Serial.print(F("I2C: "));
    Serial.println(pkt->CommandID,HEX);
} 
