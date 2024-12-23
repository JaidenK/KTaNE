#include <Arduino.h>
#include <Wire.h>
#include "KTaNE.h"
#include "ES_Framework.h"

void ServiceI2CRequest(I2C_CommandPacket *pkt)
{
    switch (pkt->CommandID)
    {
    default:
        break;
    }
}

void ReceiveI2CCommand(I2C_CommandPacket *pkt, uint8_t length)
{
    switch (pkt->CommandID)
    {
    case REG_REQUEST:
        STATUS |= STS_STRIKE;
        STATUS = STATUS ^ 0b10000000;
        // Check if they're writing to the register
        if(length > 1)
        {
            STATUS = STATUS ^ 0b01000000;
            switch (REQUEST)
            {
            case REQ_DIGITS:
                // TEMPORARY
                // TODO
                STATUS |= STS_SOLVED;
                break;            
            default:
                break;
            }
            // Clear the request flag and register
            STATUS &= ~_BV(STS_REQUEST);
            REQUEST = 0;
        }
        break;
    default:
        break;
    }
} 
