#include <Arduino.h>
#include <Wire.h>
#include "SerialCommandProcessor.h"
#include "KTaNE_Common.h"

#define OFFSET_LENGTH 2
#define OFFSET_SEQ_CNT 3
#define OFFSET_SEQ_CNT_ECHO 4
#define OFFSET_RESPONSE_LENGTH 9
#define OFFSET_ADDRESS 10
#define OFFSET_CMD_START 11

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

    I2C_SendPacketEx(address,command,length);

    if(nResponseBytes > 0)
    {
        // TODO: forward the response to the GUI as a packet?

        for(uint8_t i = 0; i < nResponseBytes && Wire.available(); i++)
        {
            uint8_t c = (uint8_t)Wire.read();
            
            // I2C interface pads data with 0xFF after slave stops transmitting
            if(c == 0xFF)
                continue;

            Serial.print((char)c);
        }
        while(Wire.available()) Wire.read(); // Consume extra characters

        Serial.println();
    }
}