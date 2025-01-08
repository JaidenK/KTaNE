#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>

#include <stdint.h>

#include "KTaNE.h"

#include "ES_Configure.h"
#include "ES_Framework.h"

//* Idea: The configuration should totally be done via bluetooth webpage.
//*       QR code to the web page found on back of the Timer module.
//*       Webpage entirely in javascript, so anyone can host it locally
//*       if they want.

// Registers declared in KTaNE_Regsiters.h
volatile uint8_t STATUS = 0;
volatile uint8_t CONTROL = 0;
volatile uint8_t REQUEST = 0;
volatile uint8_t TEST_RESULTS = 0;

uint8_t i2c_address = 0;

volatile I2C_CommandPacket LastCommand = 
{
    .CommandID = 0,
    .data = {0}
};

void I2C_request();
void I2C_receive(int nBytes);


void writeSyncBytes(uint8_t nBytes);

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

// address = -1: Join as master
// address = 0:  Get address from EEPROM
// address > 0:  Use parameter value
void KTaNE_I2C_Init(int address)
{    
    if(address < 0)
    {
        i2c_address = 0;
    }
    else if(address == 0)
    {        
        i2c_address = EEPROM.read(EEPROM_I2C_ADDRESS);
        // TODO: Check what the valid I2C address are and if we want to comply
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
    Wire.write(pkt->data.u8[0]);   // EEPROM Address Lo
    Wire.write(pkt->data.u8[1]);   // EEPROM Address Hi
    Wire.write(pkt->data.u8[2]);   // Length
    uint16_t eeprom_addr = pkt->data.u16[0];
    uint8_t length = pkt->data.u8[2];
    uint8_t byte = 0xFF;
    for(uint8_t i = 0; i < length; i++)
    {
        Wire.write(EEPROM.get(eeprom_addr+i,byte));
    }
}

void KTaNE_InitEEPROM(char *moduleName, char *serialNo, char *buildDate)
{    
    // TODO This needs to be safer
    // TODO safety check the string lengths
    for(uint8_t i = 0; i < 16; i++)
    {
        EEPROM.update(EEPROM_MODULE_NAME+i,moduleName[i]);
        EEPROM.update(EEPROM_REAL_MODULE_SERIAL+i,serialNo[i]); 
        EEPROM.update(EEPROM_BUILD_DATE+i,buildDate[i]); 
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
            writeSyncBytes(pkt->data.u8[0]);
            break;
        case REG_REQUEST:
            Wire.write(REQUEST);
            break;
        case GET_EEPROM:
            ServiceI2C_GetEEPROM(pkt);
            break;
        case REG_TEST_RESULTS:
            Wire.write(TEST_RESULTS);
            // Clear results ready flag when they read results
            STATUS &= ~_BV(STS_RESULT_READY);
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
    Module_ServiceI2CRequest(pkt);   
}

void ReceiveI2C_SetEEPROM(I2C_CommandPacket *pkt)
{
    uint16_t eeprom_addr = pkt->data.u16[0];
    uint8_t length = pkt->data.u8[2];

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
        EEPROM.update(eeprom_addr+i, pkt->data.u8[3+i]);
    }
}

void RejoinI2C(I2C_CommandPacket *pkt, uint8_t length)
{
    uint8_t isValid = 0;
    while (!isValid)
    {
        isValid = 1;
        i2c_address = (uint8_t)(random()&0b01111111);
        if(i2c_address > 127)
            i2c_address = 127;
        if(i2c_address < 10) // Arbitrary lower limit
            i2c_address = 10;
        for(uint8_t i = 0; i < length-1; i++)
        {
            if(i2c_address == pkt->data.u8[i])
            {
                isValid = 0;
                break;
            }
        }
    } 
        
    EEPROM.write(EEPROM_I2C_ADDRESS,i2c_address);
    Wire.begin(i2c_address);
}

uint8_t ReceiveI2CCommand_Common(I2C_CommandPacket *pkt, uint8_t length)
{
    uint16_t event_param = 0;
    switch (pkt->CommandID)
    {
    case REG_DEVID:
        // No reaction to device ID. Just wait for the request.
        break;
    case REG_SYNC:
        // No reaction to sync command. Just wait for the request.
        break;
    case REG_STATUS:
        // This code is reached when they've indicated that
        // they _will be_ reading the register. The read
        // hasn't occurred yet, so don't clear anything.
        break;
    case REG_CTRL:
        if(length > 1)
        {
            event_param = CONTROL << 8;
            CONTROL = pkt->data.u8[0];
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
                Module_ToggleSolveLED();
            }
            if(CONTROL & _BV(CTRL_LED2))
            {
                CONTROL &= ~_BV(CTRL_LED2);
                Module_ToggleStrikeLED();
            }       
            if(CONTROL & _BV(CTRL_DETONATE))
            {
                CONTROL &= ~_BV(CTRL_DETONATE);
                Module_Detonate();
            }     
            if(CONTROL & _BV(CTRL_SEFLTEST))
            {
                CONTROL &= ~_BV(CTRL_SEFLTEST);
                TEST_RESULTS = SELFTEST_NOT_PERFORMED;
                STATUS &= ~_BV(STS_RESULT_READY);
                Module_PerformSelfTest();                
            }      
        }
        break;
    case REG_REJOIN:
        RejoinI2C(pkt,length);
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

// This can be used if LastCommand is manually populated elsewhere.
// Used by the Timer.
void KTaNE_I2C_receive_test(uint8_t length)
{
    if(ReceiveI2CCommand_Common(&LastCommand,length)) return; // Don't do custom responses if it was a default packet
    Module_ReceiveI2CCommand(&LastCommand,length);
}

void I2C_receive(int nBytes) 
{
    (void)nBytes; // Unused

    // Get handle to where we'll store the packet
    volatile I2C_CommandPacket* pkt = &LastCommand; // Not sure if we need volatile keyword here but the compiler doesn't like excluding it.

    // Clear existing contents of that packet     
    memset(pkt,0,sizeof(I2C_CommandPacket));
    // Read the packet
    uint8_t i = 0;
    for(; i < sizeof(I2C_CommandPacket) && Wire.available(); i++)
    {
        ((uint8_t *)pkt)[i] = Wire.read();
    }    
    if(ReceiveI2CCommand_Common(pkt,i)) return; // Don't do custom responses if it was a default packet
    Module_ReceiveI2CCommand(pkt,i);
}

// TODO The timer should be using this function
uint8_t KTaNE_I2C_SendPacket(uint8_t address, uint8_t command)
{
    Wire.beginTransmission(address);
    Wire.write(command); 
    return Wire.endTransmission();
}

// TODO The timer should be using this function
uint8_t KTaNE_I2C_SendPacketEx(uint8_t address, uint8_t *command, uint8_t length)
{
    Wire.beginTransmission(address);
    for(uint8_t i = 0; i < length; i++)
        Wire.write(command[i]); 
    return Wire.endTransmission();
}