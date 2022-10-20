#include <stdint.h>
#include <Arduino.h>
#include "SerialManager.h"
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

void ServiceSerial()
{   
    static int newData = 0;

    while((newData = Serial.read()) > -1)
    {
        // Print for debug purposes
        Serial.print((char)newData);

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