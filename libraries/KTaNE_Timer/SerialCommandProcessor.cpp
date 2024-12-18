#include <Arduino.h>
#include <Wire.h>
#include "SerialCommandProcessor.h"
#include "SerialManager.h"
#include "KTaNE_Common.h"
#include "ES_Framework.h"
#include "Timer_Configure.h"
#include <string.h>
#include "EEPROM.h"

#define OFFSET_LENGTH 2
#define OFFSET_SEQ_CNT 3
#define OFFSET_SEQ_CNT_ECHO 4
#define OFFSET_RESPONSE_LENGTH 9
#define OFFSET_ADDRESS 10
#define OFFSET_CMD_START 11

static uint8_t responseBuf[32];

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
    SendUARTResponse(1, bytes, 4+length);
}

void ServiceCommandLocally(uint8_t *buf)
{   
    uint8_t address = buf[OFFSET_ADDRESS];
    uint8_t *command = &buf[OFFSET_CMD_START];
    uint8_t length = buf[OFFSET_LENGTH] - OFFSET_CMD_START - 2; // extra -2 for CRC
    uint8_t nResponseBytes = buf[OFFSET_RESPONSE_LENGTH];
    uint8_t response[32];

    switch (command[0])
    {
    case FLASH_LED:
        ES_PostAll((ES_Event){FLASH_REQUESTED,0});
        break;    
    case SET_TIME_LIMIT:
        Serial.print(F("New time: "));
        Serial.println(*((uint32_t *)&command[1]));
        SetTimeLimitConfig(*((uint32_t *)&command[1]));
        SendUARTResponse(address,command,5);
        break;
    case SET_N_BATTERIES:
        Serial.print(F("New batteries: "));
        Serial.println(*((uint32_t *)&command[1]));
        SetNumBatteriesConfig(*((uint32_t *)&command[1]));
        response[0] = SET_N_BATTERIES;
        response[1] = nBatteries;
        SendUARTResponse(address,response,2);
        break;
    case SET_SERIAL_NO:
        Serial.print(F("New serial: "));
        Serial.println((char *)&command[1]);
        response[0] = SET_SERIAL_NO;
        SetSerialNoConfig(&command[1]);
        memcpy(&response[1],serial_no,sizeof(serial_no));
        SendUARTResponse(address,response,sizeof(serial_no)+1);
        break;
    case REQUEST_CONFIG:
        switch (command[1])
        {
        case SET_TIME_LIMIT:
            response[0] = SET_TIME_LIMIT;
            memcpy(&response[1],&timeLimit,sizeof(uint32_t));
            SendUARTResponse(address,response,sizeof(uint32_t)+1);
            break;     
        case SET_N_BATTERIES:
            response[0] = SET_N_BATTERIES;
            response[1] = nBatteries;
            SendUARTResponse(address,response,2);
            break;     
        case SET_SERIAL_NO:
            response[0] = SET_SERIAL_NO;
            memcpy(&response[1],serial_no,sizeof(serial_no));
            SendUARTResponse(address,response,sizeof(serial_no)+1);
            break;  
        default:
            break;
        }
        break;
    case GET_EEPROM:
        TestEEPROMDump(command);
        break;
    case SET_EEPROM:
        SetEEPROMCmd(command);
        break;
    default:
        // Error condition
        Serial.println(F("Invalid serial command."));
        break;
    }
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