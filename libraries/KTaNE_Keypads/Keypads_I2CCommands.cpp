#include <Arduino.h>
#include <Wire.h>
#include "KTaNE.h"
#include "ES_Framework.h"
#include "BOARD_Keypads.h"

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
    default:
        break;
    }
} 