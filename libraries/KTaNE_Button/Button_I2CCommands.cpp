#include <Arduino.h>
#include <Wire.h>
#include "KTaNE.h"
#include "ES_Framework.h"
#include "BOARD_Button.h"

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
        // Check if they're writing to the register
        if(length > 1)
        {
            switch (REQUEST)
            {
            case REQ_DIGITS:
                // TODO
                STATUS |= _BV(STS_SOLVED);                
                ES_PostAll((ES_Event){SOLVED_EVENT,0});
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
