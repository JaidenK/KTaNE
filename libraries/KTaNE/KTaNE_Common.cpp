//#include <Wire.h>

#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>

#include <stdint.h>

#include "KTaNE_Constants.h"
#include "KTaNE_Common.h"

uint8_t i2c_address = 0;
I2C_CommandPacket LastCommand = 
{
    .SenderAddress = 0,
    .CommandID = 0,
    .data = {0}
};

void I2C_request();
void I2C_receive(int nBytes);

// Generates a random number and its complement 
// and writes them on the I2C bus. If multiple
// modules are on the bus talking over each other,
// then there should be a collision 
// and this should fail
void writeSyncBytes(uint8_t nBytePairs)
{
    randomSeed(analogRead(0));
    for(uint8_t i = 0; i<nBytePairs; i++) 
    {
        uint8_t sync = (uint8_t)(random(256)%256);
        Wire.write(sync);
        Wire.write(~sync);
    }
}

void reassignI2C(uint8_t reserved[], uint8_t nReserved)
{
    i2c_address = (uint8_t)(random()&0b01111111);
    EEPROM.write(EEPROM_I2C_ADDRESS,i2c_address);
    Wire.begin(i2c_address);
}

void I2C_Init(int address)
{    
    if(address < 0)
        i2c_address = EEPROM.read(EEPROM_I2C_ADDRESS);
    else 
        i2c_address = address;
    Wire.begin(i2c_address); 
    Wire.onRequest(I2C_request);
    Wire.onReceive(I2C_receive);
}



void ServiceI2C_GetEEPROM(I2C_CommandPacket *pkt)
{
    Wire.write(pkt->CommandID); // GET_EEPROM
    Wire.write(pkt->data[0]);   // EEPROM Address Lo
    Wire.write(pkt->data[1]);   // EEPROM Address Hi
    Wire.write(pkt->data[2]);   // Length
    uint16_t eeprom_addr = *(uint16_t *)(&pkt->data[0]);
    uint8_t length = pkt->data[2];
    uint8_t byte = 0xFF;
    for(uint8_t i = 0; i < length; i++)
    {
        Wire.write(EEPROM.get(eeprom_addr+i,byte));
    }
}

// Returns 1 if it was a common command. 0 otherwise.
uint8_t ServiceI2CRequest_Common(I2C_CommandPacket *pkt)
{
    switch (pkt->CommandID)
    {
        case REQUEST_SYNC:
            writeSyncBytes(pkt->data[0]);
            break;
        case GET_EEPROM:
            ServiceI2C_GetEEPROM(pkt);
            break;
        default:
            // Module-specific or invalid command ID
            return 0;
    }
    return 1;
}

void I2C_request() 
{
    I2C_CommandPacket* pkt = &LastCommand;   
    ServiceI2CRequest_Common(pkt); 
    ServiceI2CRequest(pkt);   
}

void ReceiveI2C_SetEEPROM(I2C_CommandPacket *pkt)
{
    uint16_t eeprom_addr = *(uint16_t *)(&pkt->data[0]);
    uint8_t length = pkt->data[2];

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
        EEPROM.update(eeprom_addr+i, pkt->data[3+i]);
    }
}

uint8_t ReceiveI2CCommand_Common(I2C_CommandPacket *pkt)
{
    switch (pkt->CommandID)
    {
    case SET_EEPROM:
        ReceiveI2C_SetEEPROM(pkt);
        break;    
    default:
        // Module-specific or invalid command ID
        return 0;
    }
    return 1;
}

void I2C_receive(int nBytes) 
{
    // Get handle to where we'll store the packet
    I2C_CommandPacket* pkt = &LastCommand;       
    // Clear existing contents of that packet     
    memset(pkt,0,sizeof(I2C_CommandPacket));
    // Read the packet
    for(uint8_t i = 0; i < sizeof(I2C_CommandPacket) && Wire.available(); i++)
    {
        ((uint8_t *)pkt)[i] = Wire.read();
    }
    ReceiveI2CCommand_Common(pkt);
    ReceiveI2CCommand(pkt);
}

uint8_t I2C_SendPacket(uint8_t address, uint8_t command)
{
    Wire.beginTransmission(address);
    Wire.write(i2c_address);
    Wire.write(command); 
    return Wire.endTransmission();
}

uint8_t I2C_SendPacketEx(uint8_t address, uint8_t *command, uint8_t length)
{
    Wire.beginTransmission(address);
    Wire.write(i2c_address);
    for(uint8_t i = 0; i < length; i++)
        Wire.write(command[i]); 
    return Wire.endTransmission();
}