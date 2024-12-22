//#include <Wire.h>

#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>

#include <stdint.h>

#include "KTaNE_Constants.h"
#include "KTaNE_Common.h"
#include "KTaNE_CommandIDs.h"

#include "ES_Configure.h"
#include "ES_Framework.h"


volatile uint8_t STATUS = 0;
volatile uint8_t CONTROL = 0;

uint8_t i2c_address = 0;
volatile I2C_CommandPacket LastCommand = 
{
    //.SenderAddress = 0,
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

// address = -1: Join as master
// address = 0:  Get address from EEPROM
// address > 0:  Use parameter value
void I2C_Init(int address)
{    
    if(address < 0)
    {
        i2c_address = 0;
    }
    else if(address == 0)
    {        
        i2c_address = EEPROM.read(EEPROM_I2C_ADDRESS);
        if(i2c_address < 2)
            i2c_address = 2;
        else if(i2c_address > 127)
            i2c_address = 127;
    }
    else 
    {
        i2c_address = address;
    }
    EEPROM.update(EEPROM_I2C_ADDRESS,i2c_address);
    if(i2c_address == 0)
        Wire.begin();
    else
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
        case REG_DEVID:
            Wire.write(DEVID);
            break;
        case REG_STATUS:
            Wire.write(STATUS);
            // Clear strike flag when they read status
            STATUS &= ~_BV(STS_STRIKE);
            break;
        case REG_SYNC:
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
    if(ServiceI2CRequest_Common(pkt)) return; // Don't do custom responses if it was a default packet        
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

uint8_t ReceiveI2CCommand_Common(I2C_CommandPacket *pkt, uint8_t length)
{
    uint16_t event_param = 0;
    switch (pkt->CommandID)
    {
    case REG_STATUS:
        // This code is reached when they've indicated that
        // they _will be_ reading the register. The read
        // hasn't occurred yet, so don't clear anything.
        break;
    case REG_CTRL:
        if(length > 1)
        {
            event_param = CONTROL << 8;
            CONTROL = pkt->data[0];
            event_param |= CONTROL;
            
            if(CONTROL & _BV(CTRL_START))
            {
                CONTROL &= ~_BV(CTRL_START);
                ES_PostAll((ES_Event){EVENT_START,event_param});
            }
            if(CONTROL & _BV(CTRL_RESET))
            {
                CONTROL &= ~_BV(CTRL_RESET);
                ES_PostAll((ES_Event){EVENT_RESET,event_param});
            }
            if(CONTROL & _BV(CTRL_LED1))
            {
                CONTROL &= ~_BV(CTRL_LED1);
                ToggleSolveLED();
            }
            if(CONTROL & _BV(CTRL_LED2))
            {
                CONTROL &= ~_BV(CTRL_LED2);
                ToggleStrikeLED();
            }
            
            
        }
        break;
    case SET_EEPROM:
        ReceiveI2C_SetEEPROM(pkt);
        break;    
    default:
        // Module-specific or invalid command ID
        return 0;
    }
    return 1;
}

// This can be used if LastCommand is manually populated elsewhere
void I2C_receive_test(uint8_t length)
{
    if(ReceiveI2CCommand_Common(&LastCommand,length)) return; // Don't do custom responses if it was a default packet
    ReceiveI2CCommand(&LastCommand);
}

void I2C_receive(int nBytes) 
{
    // Get handle to where we'll store the packet
    I2C_CommandPacket* pkt = &LastCommand;       
    // Clear existing contents of that packet     
    memset(pkt,0,sizeof(I2C_CommandPacket));
    // Read the packet
    uint8_t i = 0;
    for(; i < sizeof(I2C_CommandPacket) && Wire.available(); i++)
    {
        ((uint8_t *)pkt)[i] = Wire.read();
    }    
    if(ReceiveI2CCommand_Common(pkt,i)) return; // Don't do custom responses if it was a default packet
    ReceiveI2CCommand(pkt);
}

uint8_t I2C_SendPacket(uint8_t address, uint8_t command)
{
    Wire.beginTransmission(address);
    //Wire.write(i2c_address);
    Wire.write(command); 
    return Wire.endTransmission();
}

uint8_t I2C_SendPacketEx(uint8_t address, uint8_t *command, uint8_t length)
{
    Wire.beginTransmission(address);
    //Wire.write(i2c_address);
    for(uint8_t i = 0; i < length; i++)
        Wire.write(command[i]); 
    return Wire.endTransmission();
}