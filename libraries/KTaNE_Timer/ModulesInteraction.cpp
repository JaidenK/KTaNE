#include <Arduino.h>
#include <Wire.h>

// sus
#include <string.h>

#include "ES_Framework.h"
#include "ES_Configure.h"

#include "KTaNE.h"
#include "KTaNE_Constants.h"
#include "ModulesInteraction.h"
#include "EEPROM.h"

//#define VERBOSE_BUS_SCAN

ModuleInfo ModList[N_MAX_MODULES] = {0};

uint8_t SendSyncRequest(uint8_t address)
{
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    //Wire.write(i2c_address);
    Wire.write(REG_SYNC); // Signal to a potential device that we want the sync word.
    Wire.write(N_SYNC_BYTES); // Indicate we want n copies
    return Wire.endTransmission();
}

void print2digithex(uint8_t value)
{
    Serial.print(F("x"));
    if (value < 16)
        Serial.print(F("0"));
    Serial.print(value, HEX);
}

uint8_t ValidateSyncBytes(uint8_t address)
{    
    // Request the module to send the sync word
    //Serial.print(F("Scanning for sync word: "));
    Wire.requestFrom((uint8_t)address, (uint8_t)(2*N_SYNC_BYTES)); // Request the sync word to be written 16 times.
    //Wire.requestFrom((uint8_t)address, (uint8_t)2);
    uint8_t isValid = 1;
    uint8_t nBytesRead = 0;
    while(Wire.available())
    {
        uint8_t c1 = Wire.read();
        uint8_t c2 = Wire.read();
        uint8_t r = c1 | c2;
        if(r != 0xFF)
        {
            isValid = 0;
        }
        
        nBytesRead += 2;
    }
    return (!isValid || (nBytesRead != 2*N_SYNC_BYTES));
}

uint8_t doesModuleAlreadyExist(uint8_t address)
{    
    uint8_t isExistingModule = 0;
    for(uint8_t i = 0; i < N_MAX_MODULES; i++)
    {
        // I2C address of 0 means empty slot
        if(!ModList[i].i2c_address)
            continue;

        //Serial.println(ModList[i].model_id);
        if(address == ModList[i].i2c_address)
        {
            // Existing module
            isExistingModule = 1;
            break;
        }
    }
    return isExistingModule;
}


void flashModuleViaI2C(uint8_t address)
{
    Wire.beginTransmission(address);
    Wire.write(REG_CTRL);
    Wire.write(_BV(CTRL_LED1));
    Wire.endTransmission();
    delay(5);

    Wire.beginTransmission(address);
    Wire.write(REG_CTRL);
    Wire.write(_BV(CTRL_LED2));
    Wire.endTransmission();
    delay(5);

    Wire.beginTransmission(address);
    Wire.write(REG_CTRL);
    Wire.write(_BV(CTRL_LED1));
    Wire.endTransmission();
    delay(5);

    Wire.beginTransmission(address);
    Wire.write(REG_CTRL);
    Wire.write(_BV(CTRL_LED2));
    Wire.endTransmission();
    delay(50);
}

uint8_t GetStatusAllModules()
{
    uint8_t wire_result = 6;
    for(uint8_t i = 0; i < N_MAX_MODULES; i++)
    {
        if(ModList[i].i2c_address > 0)
        {
            Wire.beginTransmission(ModList[i].i2c_address);            
            Wire.write(REG_STATUS); // Signal to a potential device that we want the sync word.
            wire_result = Wire.endTransmission();
            if(wire_result != 0)
            {
                // Missing module
                //Serial.println("Missing module.");
                ES_PostAll((ES_Event){MODULE_DISCONNECTED,ModList[i].i2c_address});
                ModList[i].i2c_address = 0;
            }
            else
            {
                Wire.requestFrom(ModList[i].i2c_address, (uint8_t)1);
                ModList[i].Status = Wire.read();
            }
        }
    }
}

uint8_t toggleLEDs(uint8_t address)
{
    Wire.beginTransmission(address);
    Wire.write(REG_CTRL);
    Wire.write(_BV(CTRL_LED1) | _BV(CTRL_LED2));
    return Wire.endTransmission();
}

void resetAllModules()
{
    broadcastAllModules(REG_CTRL,_BV(CTRL_RESET));
}

void ServiceRequest(uint8_t address)
{
    uint8_t wire_result = 0;
    uint8_t request_code = 0;
    // We assume the request flag is set/already checked
    Wire.beginTransmission(address);
    Wire.write(REG_REQUEST);
    Wire.endTransmission();
    
    Wire.requestFrom(address, (uint8_t)1);
    request_code = Wire.read();

    switch (request_code)
    {
    case REQ_DIGITS:
        Wire.beginTransmission(address);
        Wire.write(REG_REQUEST);
        Wire.write((uint8_t)0x12);
        Wire.write((uint8_t)0x34);
        Wire.endTransmission();
        break;    
    default:
        break;
    }
}

void broadcastAllModules(uint8_t addr, uint8_t value)
{
    uint8_t wire_result = 6;
    for(uint8_t i = 0; i < N_MAX_MODULES; i++)
    {
        if(ModList[i].i2c_address > 0)
        {
            Wire.beginTransmission(ModList[i].i2c_address);            
            Wire.write(addr);
            Wire.write(value);
            wire_result = Wire.endTransmission();
            if(wire_result != 0)
            {
                // Missing module
                ES_PostAll((ES_Event){MODULE_DISCONNECTED,ModList[i].i2c_address});
                ModList[i].i2c_address = 0;
            }
        }
    }
}

void ReplicateConfigInfo()
{
    uint8_t wire_result = 0;
    for(uint8_t i = 0; i < N_MAX_MODULES; i++)
    {
        if(ModList[i].i2c_address > 0)
        {
            uint16_t eeprom_addr = EEPROM_REPEATED_MEMORY_START;
            uint8_t bytesRemaining = REPEATED_MEMORY_LENGTH;

            while(bytesRemaining > 0)
            {
                Wire.beginTransmission(ModList[i].i2c_address);   
                Wire.write(SET_EEPROM);         
                Wire.write(((uint8_t *)&eeprom_addr)[0]);      
                Wire.write(((uint8_t *)&eeprom_addr)[1]);    
                Wire.write((uint8_t)16);
                for(uint8_t j = 0; j < 16; j++)
                {
                    Wire.write(EEPROM.read(eeprom_addr));
                    eeprom_addr++;
                    if(bytesRemaining > 0)
                        bytesRemaining--;
                }
                wire_result = Wire.endTransmission();
            }
        }
    }
}

uint8_t checkAllModulesReady()
{
    uint8_t wire_result = 6;
    uint8_t allModulesReady = 1;
    for(uint8_t i = 0; i < N_MAX_MODULES; i++)
    {
        if(ModList[i].i2c_address > 0)
        {
            toggleLEDs(ModList[i].i2c_address);

            
            Wire.beginTransmission(ModList[i].i2c_address);            
            Wire.write(REG_STATUS); // Signal to a potential device that we want the sync word.
            wire_result = Wire.endTransmission();
            if(wire_result != 0)
            {
                // Missing module
                ES_PostAll((ES_Event){MODULE_DISCONNECTED,ModList[i].i2c_address});
                ModList[i].i2c_address = 0;
            }
            else
            {
                Wire.requestFrom(ModList[i].i2c_address, (uint8_t)1);
                ModList[i].Status = Wire.read();
                if(!(ModList[i].Status & _BV(STS_READY)))
                    allModulesReady = 0;
            }

            delay(20); // Just to see the LEDs blink
            toggleLEDs(ModList[i].i2c_address);
        }
    } 
    return allModulesReady;
}

uint8_t ScanForModules()
{
    uint8_t address = 0;
    uint8_t wire_result = 0;

    #ifdef VERBOSE_BUS_SCAN
    Serial.println(F("Scanning I2C bus:"));
    #endif
    
    for (address = 1; address < 128; address++ )
    {        
        if(address == i2c_address)
            continue;   
        
        wire_result = SendSyncRequest(address);
  
        if (wire_result == 0) // 0 = success
        {
            // Print the device address            
            #ifdef VERBOSE_BUS_SCAN
            print2digithex(address);
            Serial.print(F(": "));
            #endif

            uint8_t isNotValid = ValidateSyncBytes(address);
            if(isNotValid)
            {
                Serial.println(F("Multiple devices detected (invalid sync bytes) at address: 0x"));
                print2digithex(address);
                Serial.println();

                // Confirm device ID
                Wire.beginTransmission(address);
                Wire.write(REG_DEVID);
                Wire.endTransmission();
                Wire.requestFrom(address,(uint8_t)1);
                uint8_t their_id = Wire.read();

                if(their_id != DEVID)
                {
                    Serial.println(F("Invalid device ID: "));
                    print2digithex(their_id);
                    Serial.println();
                }
                else
                {
                    Serial.println(F("Device ID valid. Re-assigning address."));
                    Wire.beginTransmission(address);
                    Wire.write(REG_REJOIN); // Writing to this register triggers a rejoin
                    // Follow register with values that are not allowed to be used
                    for(uint8_t i = 0; i < N_MAX_MODULES; i++)
                    {
                        if(ModList[i].i2c_address > 0)
                        {
                            Wire.write(ModList[i].i2c_address);
                        }
                    }
                    Wire.endTransmission();  
                }              
            }
            else
            {
                
                #ifdef VERBOSE_BUS_SCAN
                RequestAndPrintModuleName(address);
                #endif

                if(!doesModuleAlreadyExist(address))
                {                
                    ES_PostAll((ES_Event){MODULE_CONNECTED,address});    

                    // Find first available index (or overwrite the last one)
                    uint8_t i = 0;
                    for(;(i<(N_MAX_MODULES-1))&&(ModList[i].i2c_address);i++)
                        ;

                    ModList[i].i2c_address = address;
                }
                #ifdef VERBOSE_BUS_SCAN
                Serial.println();
                #endif
            }
        }
        else if (wire_result == 4)
        {
            Serial.print(F("Unknown error wire_result=4 at address "));
            print2digithex(address);
            Serial.println();
        }
        else if (wire_result == 3)
        {
            Serial.println(F("Error wire_result=3 at address"));
            print2digithex(address);
            Serial.println();
        }
        else
        {
            // Silence
        }
    }
    #ifdef VERBOSE_BUS_SCAN
    Serial.println();
    #endif
    return 0;
}