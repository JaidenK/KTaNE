#include <Arduino.h>
#include <Wire.h>
#include "KTaNE.h"
#include "ES_Framework.h"


const char ModelID[N_MAX_MODULE_ID_CHARS] = "VALK_TIMER_000";
const char ModuleName[N_MAX_MODULE_NAME_CHARS] = "The Timer";

void ServiceI2CRequest(I2C_CommandPacket *pkt)
{
}

void ReceiveI2CCommand(I2C_CommandPacket *pkt, uint8_t length)
{
    Serial.print(F("I2C: "));
    Serial.println(pkt->CommandID,HEX);
    // Adding address and command ID to the event param is more thread-safe than
    // using the global LastCommand. And less coupled to globals.
    //ES_PostAll((ES_Event){I2C_CMD_RECEIVED,(uint16_t)((pkt->SenderAddress << 8) + pkt->CommandID)});
    ES_PostAll((ES_Event){I2C_CMD_RECEIVED,(uint16_t)(pkt->CommandID)});
} 
