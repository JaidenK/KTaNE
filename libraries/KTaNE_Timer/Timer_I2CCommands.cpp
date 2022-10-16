#include <Arduino.h>
#include <Wire.h>
#include "KTaNE.h"
#include "ES_Framework.h"


const char ModelID[N_MAX_MODULE_ID_CHARS] = "VALK_TIMER_000";
const char ModuleName[N_MAX_MODULE_NAME_CHARS] = "The Timer";

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
    default:
        break;
    }
}

void ReceiveI2CCommand(I2C_CommandPacket *pkt)
{
    ES_PostAll((ES_Event){I2C_CMD_RECEIVED,0});

    Serial.print(F("I2C: "));
    Serial.println(pkt->CommandID,HEX);
    // Adding address and command ID to the event param is more thread-safe than
    // using the global LastCommand. And less coupled to globals.
    ES_PostAll((ES_Event){I2C_CMD_RECEIVED,(uint16_t)((pkt->SenderAddress << 8) + pkt->CommandID)});
    // for(uint8_t i =0;i < sizeof(pkt->data); i++)
    // {
    //     if(pkt->data[i] > 31) // ASCII starts after 31
    //         Serial.print((char)pkt->data[i]);
    // }
    // Serial.println();
} 
