
#include <Wire.h>
#include <EEPROM.h>
#include <Arduino.h>
#include <string.h>

#include "KTaNE_Constants.h"
#include "Timer_Configure.h"


/* EEPROM Addresses */
#define EEPROM_TIME_LIMIT 0x20 // uint32_t Stored as seconds
#define EEPROM_N_BATTERIES 0x24 // uint8_t
#define EEPROM_SERIAL_NO 0x40 // String
#define EEPROM_PORTS 0x70 // uint8_t bitfield
#define EEPROM_INDICATORS 0x80 // uint8_t array of N_MAX_INDICATORS elements

uint32_t timeLimit = 0;
char serial_no[SERIAL_NO_MAX_LENGTH] = {0};
uint8_t nBatteries = 0;
Indicator indicators[N_MAX_INDICATORS] = {0};
PortType ports[N_MAX_PORTS] = {0};

void SetTimeLimitConfig(uint32_t newTimeLimit)
{
    timeLimit = newTimeLimit;
    EEPROM.put(EEPROM_TIME_LIMIT, timeLimit);
}
void SetNumBatteriesConfig(uint8_t newNBatteries)
{
    nBatteries = newNBatteries;
    EEPROM.put(EEPROM_N_BATTERIES, nBatteries);
}
void SetSerialNoConfig(char *newSerialNo)
{
    for(int i = 0; i < strnlen(newSerialNo,SERIAL_NO_MAX_LENGTH)+1; i++)
    {
        serial_no[i] = newSerialNo[i];
    }
    EEPROM.put(EEPROM_SERIAL_NO, serial_no);
}

void LoadAllEepromConfigInfo()
{
    char *moduleName =  "THE TIMER       ";
    char *serialNo =    "TIM001          "; // This should be unique for every individual module. IDK how to ensure that right now.
    char *buildDate =   "12/17/2024 18:16"; // Bless your heart if you actually remember to update this datetime each time the code changes.
    for(uint8_t i=0; i<16; i++)
    {
        EEPROM.update(EEPROM_MODULE_NAME+i,moduleName[i]);
        EEPROM.update(EEPROM_REAL_MODULE_SERIAL+i,serialNo[i]); 
        EEPROM.update(EEPROM_BUILD_DATE+i,buildDate[i]); 
    }
        
    
    EEPROM.get(EEPROM_TIME_LIMIT, timeLimit);
    if(!timeLimit)
    {
        SetTimeLimitConfig(60);
    }

    // Would get() work for this?
    for(uint8_t i = 0; i < sizeof(serial_no) - 1; i++)
    {
        serial_no[i] = EEPROM.read(EEPROM_SERIAL_NO+i);
    }
    serial_no[sizeof(serial_no)-1] = '\0'; // For good measure
    Serial.print("Serial: ");
    Serial.println(serial_no);

    EEPROM.get(EEPROM_INDICATORS, indicators);
    EEPROM.get(EEPROM_PORTS, ports);
    EEPROM.get(EEPROM_N_BATTERIES, nBatteries);
}


uint8_t BroadcastAllConfigInfo(void)
{
    /*// Serial Number
    for(uint8_t i = 0; i < SERIAL_NO_LENGTH; i++)
    {
        serialNo[i] = EEPROM.read(EEPROM_SERIAL_NO+i);
    }
    for(uint8_t i = 0; i < nConnectedModules; i++)
    {
        Wire.beginTransmission(ConnectedModules[i].i2c_address);
        Wire.write(REQ_SET_SERIAL_NO);
        Wire.write(serialNo,SERIAL_NO_LENGTH);
        Wire.endTransmission();
    }
    tone(SPEAKER_PIN,G4,90);
    delay(100);

    // N Batteries
    nBatteries = EEPROM.read(EEPROM_N_BATTERIES);            
    for(uint8_t i = 0; i < nConnectedModules; i++)
    {
        Wire.beginTransmission(ConnectedModules[i].i2c_address);
        Wire.write(REQ_SET_N_BATTERIES);
        Wire.write(nBatteries);
        Wire.endTransmission();
    }
    tone(SPEAKER_PIN,G4,90);
    delay(100);

    // Indicators         
    for(uint8_t i = 0; i < nConnectedModules; i++)
    {
        Wire.beginTransmission(ConnectedModules[i].i2c_address);
        Wire.write(REQ_SET_INDICATORS);
        for(uint8_t j = 0; j < N_MAX_INDICATORS; j++)
        {
            Wire.write(EEPROM.read(EEPROM_LIT_INDIC + j));
        }
        Wire.endTransmission();
    }
    tone(SPEAKER_PIN,G4,90);
    delay(100);*/

    return 0;
}