#include <Arduino.h>
#include <Wire.h>
#include "KTaNE.h"
#include "ES_Framework.h"


const char ModelID[N_MAX_MODULE_ID_CHARS] = "VALK_SKETCH_000";
const char ModuleName[N_MAX_MODULE_NAME_CHARS] = "Sketchpad";

void ServiceI2CRequest(I2C_CommandPacket *pkt)
{
    switch (pkt->CommandID)
    {
    case REQUEST_SYNC:
        writeSyncBytes(pkt->data[0]);
        break;
    case REQUEST_ID:
        Wire.write(ModelID);
        break;
    case REQUEST_NAME:
        Wire.write(ModuleName);
        break;   
    case FLASH_LED:
        ES_PostAll((ES_Event){FLASH_REQUESTED,0});
        break;
    default:
        break;
    }
}

void ReceiveI2CCommand(I2C_CommandPacket *pkt)
{
    // Serial.print(F("I2C: "));
    // for(uint8_t i =0;i < sizeof(pkt->data); i++)
    // {
    //     if(pkt->data[i] > 31) // ASCII starts after 31
    //         Serial.print((char)pkt->data[i]);
    // }
    // Serial.println();
} 
