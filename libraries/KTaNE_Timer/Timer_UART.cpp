#include <stdint.h>
#include <Arduino.h>
#include "SerialCommandProcessor.h"
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "Timer_UART.h"
#include "KTaNE.h"
#include "Timer_I2C.h"
#include "Speaker.h"


/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
#define SYNC_BYTE 0xA5

uint8_t RunUARTReceiverFSM(uint8_t newData);

uint8_t CheckRx(void);
uint8_t CheckTx(void);
uint8_t CheckConnection(void);

void SendModuleListToPC(void);


/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                    *
 ******************************************************************************/

#define UART_RX_BUF_SIZE 64
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

uint32_t tLastRxPacket = 0;
uint8_t isConnected = 0;
#define CONNECTION_TIMEOUT_MS 1000

uint8_t isTransmittingStatus = 0;
uint32_t tNextTransmission = 0;
#define STATUS_TRANSMISSION_PERIOD_MS 200

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

uint8_t TimerUART_CheckEvents(void)
{
    uint8_t eventOccurred = FALSE;

    eventOccurred |= CheckRx();
    eventOccurred |= CheckTx();
    eventOccurred |= CheckConnection();

    return eventOccurred;
}


void TimerUART_SendPacket(uint8_t address, uint8_t *bytes, uint8_t nBytes)
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

void TimerUART_SendCommandByte(uint8_t address, uint8_t byte)
{
    TimerUART_SendPacket(address, &byte, 1);
}

void TimerUART_SendResponse(uint8_t address, uint8_t *bytes, uint8_t nBytes)
{
    // Use highest bit in address to indicate a response
    TimerUART_SendPacket(address | (1 << 7), bytes, nBytes);
}

uint8_t TimerUART_IsPCConnected(void)
{
    return isConnected;
}

void TimerUART_EnableStatusTransmissions(void)
{
    isTransmittingStatus = 1;
    tNextTransmission = millis();
}

void TimerUART_DisableStatusTransmissions(void)
{
    isTransmittingStatus = 0;
}

/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/

uint8_t RunUARTReceiverFSM(uint8_t newData)
{
    uint8_t packetReceived = 0;

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
        packetLength = newData;
        if(packetLength > UART_RX_BUF_SIZE)
            rxState = SearchingForSync1;
        else
            rxState = AcquiringDataBytes;
        break;        
    default:
        break;
    }

    buf[iBuf++] = newData;

    if(iBuf == packetLength)
    {
        rxState = SearchingForSync1;
        
        // TODO Check CRC

        // if(crc is correct):
        packetReceived = 1;
    }

    return packetReceived;
}

uint8_t CheckRx(void)
{
    static int newData = 0;    
    static uint8_t packetReceived = 0;

    while((newData = Serial.read()) > -1)
    {
        packetReceived = RunUARTReceiverFSM((uint8_t)newData);
        if(packetReceived)
        {
            tLastRxPacket = millis();
            ProcessSerialCommand(buf);
        }

    }

    return 0;
}

uint8_t CheckTx(void)
{
    if(isTransmittingStatus)
    {
        if(millis() >= tNextTransmission)
        {
            tNextTransmission += STATUS_TRANSMISSION_PERIOD_MS;

            // ? Do we also want to check if we're connected?
            // Probably not since we would still like to receive info even
            // if we screw up the PC end of the communication. Maybe once we've
            // tested more thoroughly then we can limit it.
            SendModuleListToPC();
        }
    }
    return 0;
}


uint8_t CheckConnection(void)
{
    uint8_t eventOccurred = FALSE;

    // This is a hack to prevent the connection looking like it exists
    // immediately on booting
    if(millis() <= CONNECTION_TIMEOUT_MS)
        return FALSE;

    // Look for change in connection state.
    if(isConnected)
    {
        if((millis() - tLastRxPacket) > CONNECTION_TIMEOUT_MS)
        {
            isConnected = 0;
            ES_PostAll((ES_Event){PC_CONNECTION_CHANGED,0});
            Speaker_PlaySound(SOUND_CONNECTION_LOST); // TODO Perhaps the speaker should have its own service that can respond to the event?
            eventOccurred = TRUE;
        }
    }
    else if((millis() - tLastRxPacket) < CONNECTION_TIMEOUT_MS)
    {
        isConnected = 1;        
        ES_PostAll((ES_Event){PC_CONNECTION_CHANGED,1});
        Speaker_PlaySound(SOUND_CONNECTION_MADE); // TODO Perhaps the speaker should have its own service that can respond to the event?
        eventOccurred = TRUE;
    }

    return eventOccurred;
}

void SendModuleListToPC()
{  
    uint8_t bytes[N_MAX_MODULES + 1] = {0};
    bytes[0] = LIST_MODULES;
    uint8_t nModules = 0;
    for(uint8_t i = 0; i < N_MAX_MODULES; i++)
    {
        // TODO I don't want to directly reference ModList like this
        if(ModList[i].i2c_address > 0)
        {
            bytes[1+(2*nModules)] = ModList[i].i2c_address;
            bytes[1+(2*nModules)+1] = ModList[i].Status;
            nModules++;
        }
    }

    TimerUART_SendPacket(i2c_address, bytes, 2*nModules+1);
}