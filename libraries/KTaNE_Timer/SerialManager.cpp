#include <stdint.h>
#include <Arduino.h>
#include "SerialManager.h"
#include "ES_Configure.h"
#include "SerialCommandProcessor.h"

#define UART_RX_BUF_SIZE 64
#define SYNC_BYTE 0xA5

typedef enum 
{
    SearchingForSync1,
    SearchingForSync2,
    ReadingLength,
    AcquiringDataBytes
}
RxState_t;
RxState_t rxState = SearchingForSync1;
uint8_t packetLength = 0;

uint8_t buf[UART_RX_BUF_SIZE] = {0};
uint8_t iBuf = 0;

uint8_t CheckSerial()
{
    ServiceSerial();
    return 0;
}

void ServiceSerial()
{   
    static int newData = 0;    

    while((newData = Serial.read()) > -1)
    {
        // Print for debug purposes
        //Serial.print(newData);
        //Serial.print(' ');

        switch (rxState)
        {
        case SearchingForSync1:
            iBuf = 0;
            if(newData == SYNC_BYTE)
            {
                rxState = SearchingForSync2;
            }
            break;
        case SearchingForSync2:
            if(newData == SYNC_BYTE)
            {
                rxState = ReadingLength;
            }
            else
            {
                rxState = SearchingForSync1;
            }
            break;
        case ReadingLength:
            packetLength = (uint8_t)newData;
            if(packetLength > UART_RX_BUF_SIZE)
                rxState = SearchingForSync1;
            else
                rxState = AcquiringDataBytes;
            break;        
        default:
            break;
        }

        buf[iBuf++] = (uint8_t)newData;

        if(iBuf == packetLength)
        {
            rxState = SearchingForSync1;
            
            // Check CRC

            ProcessSerialCommand(buf);
        }
    }
}

void SendUARTCommand(uint8_t address, uint8_t *bytes, uint8_t nBytes)
{
    // Sync 
    Serial.write(SYNC_BYTE);
    Serial.write(SYNC_BYTE);
    // Length
    Serial.write(12 + nBytes);
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
    Serial.write(0x34);
    Serial.write(0x56);
}

void SendUARTResponse(uint8_t address, uint8_t *bytes, uint8_t nBytes)
{
    // Use highest bit in address to indicate a response
    SendUARTCommand(address | (1 << 7), bytes, nBytes);
}

