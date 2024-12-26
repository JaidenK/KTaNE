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
                // ChatGPT Suggestions:
                    //    uint16_t value;
                    //    memcpy(&value, &pkt->data[0], sizeof(value)); 
                // Or
                    //union PacketData {
                    //    uint8_t bytes[2];
                    //    uint16_t value;
                    //};
                    //
                    //union PacketData *p = (union PacketData *)&pkt->data[0];
                    //uint16_t value = p->value;
                // Or
                    //struct __attribute__((packed)) Packet {
                    //    uint16_t value;
                    //};
                    //
                    //struct Packet *p = (struct Packet *)&pkt->data[0];
                    //uint16_t value = p->value;
                // Or
                    // uint16_t value = (pkt->data[0] << 8) | pkt->data[1];
                
                uint16_t param = *(uint16_t *)(&pkt->data[0]);
                ES_PostAll((ES_Event){DIGITS_RECEIVED,param});
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
