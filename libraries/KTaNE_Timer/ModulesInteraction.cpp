#include "ModulesList.h"

#include <Arduino.h>
#include <Wire.h>

// sus
#include <string.h>

#include "KTaNE.h"
#include "ModulesInteraction.h"

//#define VERBOSE_BUS_SCAN

ModuleInfo ModList[N_MAX_MODULES] = {0};

uint8_t SendSyncRequest(uint8_t address)
{
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    Wire.write(i2c_address);
    Wire.write(REQUEST_SYNC); // Signal to a potential device that we want the sync word.
    Wire.write(N_SYNC_BYTES); // Indicate we want 16 copies
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


void RequestAndPrintModuleName(uint8_t address)
{
    // Device is a valid module!
    // Ask for its name
    I2C_SendPacket(address, REQUEST_NAME);
    Wire.requestFrom((uint8_t)address, (uint8_t)N_MAX_MODULE_NAME_CHARS); 
    
    // Read the name
    for(uint8_t i = 0; i < N_MAX_MODULE_NAME_CHARS && Wire.available(); i++)
    {
        uint8_t c = (uint8_t)Wire.read();
        
        // I2C interface pads data with 0xFF after slave stops transmitting
        if(c == 0xFF)
            break;

        Serial.print((char)c);
    }
    while(Wire.available()) Wire.read(); // Consume extra characters
}

void GetModuleID(uint8_t address, char *IDbuf, uint8_t length)
{    
    // Device is a valid module!
    // Ask for its name
    Wire.beginTransmission(address);
    Wire.write(i2c_address);
    Wire.write(REQUEST_ID);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)address, length); 
    
    // Read the name
    for(uint8_t i = 0; i < length && Wire.available(); i++)
    {
        uint8_t c = (uint8_t)Wire.read();
        
        // I2C interface pads data with 0xFF after slave stops transmitting
        if(c == 0xFF)
            break;

        IDbuf[i] = (char)c;
        // Serial.print((char)c);
    }
    while(Wire.available()) Wire.read(); // Consume extra characters
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

uint8_t ScanForModules()
{
    uint8_t address = 0;
    uint8_t wire_result = 0;

    #ifdef VERBOSE_BUS_SCAN
    Serial.println(F("Scanning I2C bus:"));
    #endif
    
    for (address = 0; address < 128; address++ )
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
                // Send the command to scramble their addressses
                I2C_SendPacket(address, REASSIGN_I2C_ADDRESS);
            }
            else
            {
                
                #ifdef VERBOSE_BUS_SCAN
                RequestAndPrintModuleName(address);
                #endif
                char IDbuf[N_MAX_MODULE_ID_CHARS] = {0};
                GetModuleID(address,IDbuf,sizeof(IDbuf));     

                if(!doesModuleAlreadyExist(address))
                {                    
                    // New module
                    Serial.print(F(" (New module!): "));
                    print2digithex(address);
                    Serial.println();
                    
                                        
                    // Find first available index (or overwrite the last one)
                    uint8_t i = 0;
                    for(;(i<(N_MAX_MODULES-1))&&(ModList[i].i2c_address);i++)
                        ;

                    ModList[i].i2c_address = address;
                    ModList[i].isDisarmed = 0;
                    memcpy(ModList[i].model_id,IDbuf,N_MAX_MODULE_ID_CHARS);
                    Serial.print("model id: ");
                    Serial.println(ModList[i].model_id);
                }
                #ifdef VERBOSE_BUS_SCAN
                Serial.println();
                #endif
            }
        }
        else if (wire_result == 4)
        {
            Serial.println(F("Unknown error wire_result=4"));
        }
        else if (wire_result == 3)
        {
            Serial.println(F("Error wire_result=3"));
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