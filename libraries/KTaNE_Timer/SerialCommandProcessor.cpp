#include <Arduino.h>
#include <Wire.h>
#include "SerialCommandProcessor.h"
#include "SerialManager.h"
#include "KTaNE_Common.h"
#include "ES_Framework.h"

#define OFFSET_LENGTH 2
#define OFFSET_SEQ_CNT 3
#define OFFSET_SEQ_CNT_ECHO 4
#define OFFSET_RESPONSE_LENGTH 9
#define OFFSET_ADDRESS 10
#define OFFSET_CMD_START 11

static uint8_t responseBuf[32];

void ServiceCommandLocally(uint8_t *buf)
{   
    uint8_t address = buf[OFFSET_ADDRESS];
    uint8_t *command = &buf[OFFSET_CMD_START];
    uint8_t length = buf[OFFSET_LENGTH] - OFFSET_CMD_START - 1; // extra -1 for CRC
    uint8_t nResponseBytes = buf[OFFSET_RESPONSE_LENGTH];

    switch (command[0])
    {
    case FLASH_LED:
        ES_PostAll((ES_Event){FLASH_REQUESTED,0});
        break;    
    default:
        // Error condition
        break;
    }
}

void ProcessSerialCommand(uint8_t *buf)
{
    if(buf[OFFSET_LENGTH] <= OFFSET_CMD_START + 1)
    {
        // The packet length is somehow too low
        // to contain a command
        return;
    }

    uint8_t address = buf[OFFSET_ADDRESS];
    uint8_t *command = &buf[OFFSET_CMD_START];
    uint8_t length = buf[OFFSET_LENGTH] - OFFSET_CMD_START - 1; // extra -1 for CRC
    uint8_t nResponseBytes = buf[OFFSET_RESPONSE_LENGTH];

    if(address == i2c_address)
    {
        ServiceCommandLocally(buf);
        return;
    }

    I2C_SendPacketEx(address,command,length);

    if(nResponseBytes > 0)
    {
        // TODO: forward the response to the GUI as a packet?        
        
        Wire.requestFrom((uint8_t)address, nResponseBytes); 

        for(uint8_t i = 0; i < nResponseBytes && Wire.available(); i++)
        {
            uint8_t c = (uint8_t)Wire.read();
            responseBuf[i] = c;
            
            // I2C interface pads data with 0xFF after slave stops transmitting
            if(c == 0xFF)
                continue;

            Serial.print((char)c);
        }
        while(Wire.available()) Wire.read(); // Consume extra characters

        Serial.println();
        
        SendUARTResponse(address, responseBuf, nResponseBytes);
    }
}