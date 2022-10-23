#include <stdint.h>
#include <Arduino.h>
#include "SerialManager.h"
#include "ES_Configure.h"
#include "SerialCommandProcessor.h"


void SendUARTCommandByte(uint8_t address, uint8_t byte)
{
    SendUARTCommand(address, &byte, 1);
}

void SendUARTCommand(uint8_t address, uint8_t *bytes, uint8_t nBytes)
{
    // Sync 
    Serial.write(SYNC_BYTE);
    Serial.write(SYNC_BYTE);
    // Length
    Serial.write(13 + nBytes);
    // Sequence Counts
    Serial.write(0x00);
    Serial.write(0x00);
    // Reserved
    Serial.write(0x01);
    Serial.write(0x02);
    Serial.write(0x03);
    Serial.write(0x04);
    // Response length
    Serial.write(0x00);
    // Destination/Sender 
    Serial.write(address);
    // Data
    for(uint8_t i = 0; i < nBytes; i++)
    {
        Serial.write(bytes[i]);
    }
    // CRC
    Serial.write(0x75);
    Serial.write(0x75);
}

void SendUARTResponse(uint8_t address, uint8_t *bytes, uint8_t nBytes)
{
    // Use highest bit in address to indicate a response
    SendUARTCommand(address | (1 << 7), bytes, nBytes);
}

