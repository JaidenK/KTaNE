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
    case REQUEST_ID:
        Wire.write(ModelID);
        break;
    case REQUEST_NAME:
        Wire.write(ModuleName);
        break;
    case GET_TIME_MS:    
        uint32_t t = (uint32_t)millis();            
        Wire.write(((uint8_t *)&t)[0]);             
        Wire.write(((uint8_t *)&t)[1]);             
        Wire.write(((uint8_t *)&t)[2]);             
        Wire.write(((uint8_t *)&t)[3]);      
    case FLASH_LED:
        ES_PostAll((ES_Event){FLASH_REQUESTED,0});
        break;
    default:
        break;
    }
}

void ReceiveI2CCommand(I2C_CommandPacket *pkt)
{
    Serial.print(F("I2C: "));
    Serial.println(pkt->CommandID,HEX);
    // Adding address and command ID to the event param is more thread-safe than
    // using the global LastCommand. And less coupled to globals.
    //ES_PostAll((ES_Event){I2C_CMD_RECEIVED,(uint16_t)((pkt->SenderAddress << 8) + pkt->CommandID)});
    ES_PostAll((ES_Event){I2C_CMD_RECEIVED,(uint16_t)(pkt->CommandID)});
} 
