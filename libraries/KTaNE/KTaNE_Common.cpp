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

void I2C_request() 
{
    I2C_CommandPacket* pkt = &LastCommand;    
    ServiceI2CRequest(pkt);   
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