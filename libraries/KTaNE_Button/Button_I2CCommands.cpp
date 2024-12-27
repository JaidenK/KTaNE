#include <Arduino.h>
#include <Wire.h>
#include "KTaNE.h"
#include "ES_Framework.h"
#include "BOARD_Button.h"

//
// Posts: DIGITS_RECEIVED
//

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
                ES_PostAll((ES_Event){DIGITS_RECEIVED,pkt->data.u16[0]});
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
