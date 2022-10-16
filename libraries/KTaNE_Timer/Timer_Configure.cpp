
#include <Wire.h>

#include "Timer_Configure.h"


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