#include <Arduino.h>
#include <Wire.h>
#include "KTaNE.h"
#include "ES_Framework.h"

void Module_ServiceI2CRequest(I2C_CommandPacket *pkt)
{
    switch (pkt->CommandID)
    {
    default:
        break;
    }
}

void Module_ReceiveI2CCommand(I2C_CommandPacket *pkt, uint8_t length)
{
    // Serial.print(F("I2C: "));
    // for(uint8_t i =0;i < sizeof(pkt->data); i++)
    // {
    //     if(pkt->data[i] > 31) // ASCII starts after 31
    //         Serial.print((char)pkt->data[i]);
    // }
    // Serial.println();
} 
