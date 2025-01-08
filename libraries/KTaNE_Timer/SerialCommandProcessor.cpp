#include <Arduino.h>
#include <Wire.h>
#include "SerialCommandProcessor.h"
#include "Timer_UART.h"
#include "KTaNE.h"
#include "ES_Framework.h"
#include "Timer_EEPROM.h"
#include <string.h>
#include "EEPROM.h"

#define OFFSET_LENGTH 2
#define OFFSET_SEQ_CNT 3
#define OFFSET_SEQ_CNT_ECHO 4
#define OFFSET_RESPONSE_LENGTH 9
#define OFFSET_ADDRESS 10
#define OFFSET_CMD_START 11

static uint8_t responseBuf[32] = {0};
uint8_t hasFirstPacketBeenReceived = 0;

void SetEEPROMCmd(uint8_t *command)
{
    uint16_t eeprom_addr = *(uint16_t *)(&command[1]);
    uint8_t length = command[3];

    if(eeprom_addr < END_OF_PROTECTED_EEPROM)
    {
        Serial.println(F("Protected EEPROM"));
        return;
    }

    // Maybe in the future we can remove this cap, but for now
    // it's a "safety" feature.
    if(length > 16)
        length = 16;

    for(uint8_t i = 0; i < length; i++)
    {
        EEPROM.update(eeprom_addr+i, command[4+i]);
    }
}

void TestEEPROMDump(uint8_t *command)
{
    uint16_t eeprom_addr = *(uint16_t *)(&command[1]);
    uint8_t length = command[3];
    uint8_t bytes[4+length] = {0};
    bytes[0] = command[0];
    bytes[1] = command[1];
    bytes[2] = command[2];
    bytes[3] = command[3];
    for(uint8_t i = 0; i < length; i++)
    {
        EEPROM.get(eeprom_addr+i,bytes[i+4]);
    }
    TimerUART_SendResponse(i2c_address, bytes, 4+length);
}

uint8_t ServiceCommandLocally(uint8_t *buf)
{   
    //uint8_t address = buf[OFFSET_ADDRESS];
    uint8_t *command = &buf[OFFSET_CMD_START];
    //uint8_t length = buf[OFFSET_LENGTH] - OFFSET_CMD_START - 2; // extra -2 for CRC
    //uint8_t nResponseBytes = buf[OFFSET_RESPONSE_LENGTH];
    //uint8_t response[32];

    switch (command[0])
    {   
    case GET_EEPROM:
        TestEEPROMDump(command);
        break;
    case SET_EEPROM:
        SetEEPROMCmd(command);
        break;
    default:
        // Error condition
        //Serial.println(F("Invalid serial command."));
        return 0;
        break;
    }
    return 1;
}

void ProcessSerialCommand(uint8_t *buf)
{
    if(buf[OFFSET_LENGTH] <= OFFSET_CMD_START + 2)
    {
        // The packet length is somehow too low
        // to contain a command
        return;
    }

    uint8_t address = buf[OFFSET_ADDRESS];
    uint8_t *command = &buf[OFFSET_CMD_START];
    uint8_t length = buf[OFFSET_LENGTH] - OFFSET_CMD_START - 2; // extra -1 for CRC
    uint8_t nResponseBytes = buf[OFFSET_RESPONSE_LENGTH];

    if(!hasFirstPacketBeenReceived)
    {
        hasFirstPacketBeenReceived = 1;
        ES_PostAll((ES_Event){PC_CONNECTION_CHANGED,1});
    }

    // A command has been received from the PC. If it's addressed to
    // us, the Timer, then service it locally. If it's not for us,
    // then just send it along via I2C.
    if(address == i2c_address)
    {
        // If the command is meant for us, service it locally.
        if(!ServiceCommandLocally(buf))
        {
            // If the command wasn't handled, it's a "normal" 
            // I2C command rather than a special Serial command
            for(uint8_t i = 0; i < length; i++)
                ((uint8_t *)&LastCommand)[i] = command[i];
            KTaNE_I2C_receive_test(length);
        }        
    }
    else
    {
        // TODO Refactor. This is hard to follow.
        
        KTaNE_I2C_SendPacketEx(address,command,length);

        if(nResponseBytes > 0)
        {        
            Wire.requestFrom((uint8_t)address, nResponseBytes); 

            for(uint8_t i = 0; i < nResponseBytes && Wire.available(); i++)
            {
                uint8_t c = (uint8_t)Wire.read();
                responseBuf[i] = c;            
            }
            while(Wire.available()) Wire.read(); // Consume extra characters if any where sent

            TimerUART_SendResponse(address, responseBuf, nResponseBytes);
        }
    }
}