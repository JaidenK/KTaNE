

/*****************************************************************************
 * MODULE #INCLUDE                                                           *
 ****************************************************************************/
#include <Arduino.h>
#include <Wire.h>
#include "Timer_I2C.h"
#include "Utilities.h"
#include "ES_Framework.h"
#include "ES_Configure.h"
#include "KTaNE.h"
#include "KTaNE_Constants.h"
#include "ModulesInteraction.h"
#include "EEPROM.h"
#include "Clock.h"
#include "Speaker.h" // TODO: Get rid of this reference

/*****************************************************************************
 * MODULE #DEFINES                                                           *
 ****************************************************************************/

/*****************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                               *
 ****************************************************************************/

uint8_t MonitorMode_CheckNextModule(void);

void ServiceRequest(uint8_t module_i2c_address);
uint8_t doesModuleAlreadyExist(uint8_t address); // ? Should this be public ?

void ScanError(uint8_t scanmode_address, uint8_t wire_result);

// These inline functions are called exactly once in one place each
inline uint8_t ScanMode_ScanNextAddress(void); 
inline uint8_t ScanMode_SendSyncRequest(uint8_t module_i2c_address);
inline uint8_t ScanMode_ValidateSyncBytes(uint8_t address);
inline uint8_t ScanMode_ValidModule(uint8_t scanmode_address);
inline uint8_t ScanMode_AddressConflict(uint8_t scanmode_address);
inline uint8_t ScanMode_InvalidSyncBytes(uint8_t scanmode_address);

/*****************************************************************************
 * PRIVATE MODULE VARIABLES                                                  *
 ****************************************************************************/


I2CMode_t I2CMode = I2C_IDLE;

ModuleInfo ModList[N_MAX_MODULES] = {0};

uint8_t isBusScanningEnabled = 0;
uint8_t isStatusMonitoringEnabled = 0;
uint8_t scanInProgress = 0;

uint8_t scanmode_address = 0; 

/*****************************************************************************
 * PUBLIC FUNCTIONS                                                          *
 ****************************************************************************/


uint8_t TimerI2C_CheckEvents(void)
{    
    uint8_t eventOccured = 0;

    if(isBusScanningEnabled)
        eventOccured |= ScanMode_ScanNextAddress();
    if(isStatusMonitoringEnabled)
        eventOccured |= MonitorMode_CheckNextModule();

    return eventOccured;
}

void TimerI2C_SetMode(I2CMode_t mode)
{
    // TODO
}

// TODO Needed?
I2CMode_t TimerI2C_GetMode(void)
{
    return I2CMode;
}

// TODO Needed?
void TimerI2C_ScanAllAddressesNow_Blocking(void)
{

}

void TimerI2C_FlashLEDs_Blocking(uint8_t module_i2c_address)
{
    TimerI2C_SetRegister(module_i2c_address,REG_CTRL,_BV(CTRL_LED1));
    delay(5);
    TimerI2C_SetRegister(module_i2c_address,REG_CTRL,_BV(CTRL_LED2));
    delay(5);
    TimerI2C_SetRegister(module_i2c_address,REG_CTRL,_BV(CTRL_LED1));
    delay(5);
    TimerI2C_SetRegister(module_i2c_address,REG_CTRL,_BV(CTRL_LED2));
    delay(5);
}

void TimerI2C_ToggleLEDs(uint8_t module_i2c_address)
{
    TimerI2C_SetRegister(module_i2c_address, REG_CTRL, _BV(CTRL_LED1) | _BV(CTRL_LED2));
}

void TimerI2C_BroadcastAllModules(uint8_t register_address, uint8_t value)
{
    for(uint8_t i = 0; i < N_MAX_MODULES; i++)
    {
        if(ModList[i].i2c_address > 0)
        {
            TimerI2C_SetRegister(ModList[i].i2c_address, register_address, value);
        }
    }
}

void TimerI2C_ResetAllModules(void)
{
    TimerI2C_BroadcastAllModules(REG_CTRL,_BV(CTRL_RESET));
}

void TimerI2C_StartAllModules(void)
{
    TimerI2C_BroadcastAllModules(REG_CTRL,_BV(CTRL_START));
}

uint8_t TimerI2C_SetRegister(uint8_t module_i2c_address, uint8_t register_address, uint8_t value)
{
    uint8_t wire_result = 0;
    Wire.beginTransmission(module_i2c_address);
    Wire.write(register_address);
    Wire.write(value); 
    wire_result = Wire.endTransmission();
    
    if(wire_result != 0)
    {
        // TODO This error could be handled differently
        // Missing module
        //ES_PostAll((ES_Event){MODULE_DISCONNECTED,module_i2c_address});
        // Don't post event here because it's also called just when we're 
        // scanning the bus.
    }

    return wire_result;
}

uint8_t TimerI2C_ReadRegister(uint8_t module_i2c_address, uint8_t register_address, uint8_t *out)
{
    uint8_t retval = 0;
    // TODO: Error checking. 
    // Need to come up with a good error checking system, even if it's just
    // a mostly placeholder interface.
    Wire.beginTransmission(module_i2c_address);
    Wire.write(register_address);
    retval = Wire.endTransmission();
    Wire.requestFrom(module_i2c_address,(uint8_t)1);
    *out = Wire.read();
    return retval;
}


void TimerI2C_StartBusScan(void)
{
    TimerI2C_EnableBusScanning();
    scanmode_address = 0;
    scanInProgress = 1;
}

void TimerI2C_EnableBusScanning(void)
{
    isBusScanningEnabled = 1;
}
void TimerI2C_DisableBusScanning(void)
{
    isBusScanningEnabled = 0;
}
void TimerI2C_EnableStatusMonitoring(void)
{
    isStatusMonitoringEnabled = 1;
}
void TimerI2C_DisableStatusMonitoring(void)
{
    isStatusMonitoringEnabled = 0;
}

void TimerI2C_BroadcastGameConfigInfo(void)
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

/*****************************************************************************
 * PRIVATE FUNCTIONS                                                         *
 ****************************************************************************/

/**
 * @brief Checks an i2c address to see if there is a KTaNE module present.
 * 
 * The timer will try to request sync bytes from an i2c address
 * 
 * The old method for scanning the bus was a blocking call that read all the
 * addresses in one go. This version instead only scans 1 address per call 
 * and automatically increments the address that will be scanned next time.
 * This allows us to put it in the event checker that runs repeatedly while
 * still allowing for frequent breaks to allow other systems CPU time. 
 */
uint8_t ScanMode_ScanNextAddress(void)
{   
    static uint8_t wire_result = 0;
    uint8_t eventOccurred = FALSE;

    //* Debug Tip: Check the I2C bus with the logic analyzer instead of 
    //*            adding a Serial.println();

    // TODO record millis() for throttling

    // Increment address to scan through the i2c address space
    if(scanmode_address++ > 127) 
    {
        scanmode_address = 0;
        if(scanInProgress)
        {
            scanInProgress = 0;
            ES_PostAll((ES_Event){I2C_BUS_SCAN_COMPLETE,0});
        }
    }

    // Timer has no i2c_address currently, but this is here for potential
    // future compatibility. Prevents trying to sync with ourselves.
    if(scanmode_address == i2c_address)
        return FALSE;
    
    // Tell the module to begin generating the sync bytes.
    wire_result = ScanMode_SendSyncRequest(scanmode_address);

    if(wire_result == 0) // Successful request, not yet sure it's a valid module
    {
        // TODO Print device address

        uint8_t isNotValid = ScanMode_ValidateSyncBytes(scanmode_address);
        if(isNotValid)
        {
            eventOccurred = ScanMode_InvalidSyncBytes(scanmode_address);
        }
        else
        {
            eventOccurred = ScanMode_ValidModule(scanmode_address);
        }

    }
    else if(wire_result == 2) // 2: received NACK on transmit of address. 
    {   
        // This is the expected case for most addresses.  
        // Do nothing. 
    }
    else
    {
        ScanError(scanmode_address, wire_result);
    }

    return eventOccurred;
}

//* Idea: TimerI2C_StartScan() switches to scan mode and then posts a 
//*       SCAN_COMPLETE event when all addresses have been read. If it was 
//*       already in scan mode then it will perform a new scan.

// TODO Move to private functions and doc comment
void ScanError(uint8_t scanmode_address, uint8_t wire_result)
{
    Serial.print(F("Error wire_result="));
    Serial.print(wire_result);
    Serial.print(F(" at address: "));
    print2digithex(scanmode_address);
    Serial.println();
}
// TODO Move to private functions and doc comment
uint8_t ScanMode_InvalidSyncBytes(uint8_t scanmode_address)
{
    uint8_t eventOccurred = FALSE;
    Serial.println(F("Multiple devices detected (invalid sync bytes) at address: 0x"));
    print2digithex(scanmode_address);
    Serial.println();

    // Confirm device ID indicates a valid KTaNE module
    uint8_t their_id = 0;
    TimerI2C_ReadRegister(scanmode_address, REG_DEVID, &their_id);
    if(their_id != DEVID)
    {
        Serial.println(F("Invalid device ID: "));
        print2digithex(their_id);
        Serial.println();
    }
    else
    {
        eventOccurred = ScanMode_AddressConflict(scanmode_address);
    }  
    return eventOccurred;
}

// TODO Move to private functions and doc comment
uint8_t ScanMode_AddressConflict(uint8_t scanmode_address)
{
    uint8_t eventOccurred = FALSE;

    Serial.println(F("Device ID valid. Re-assigning address."));
    Wire.beginTransmission(scanmode_address);
    Wire.write(REG_REJOIN); // Writing to this register triggers a rejoin
    // Follow register address with values that are not allowed to be used
    for(uint8_t i = 0; i < N_MAX_MODULES; i++)
    {
        // ? Should this be replaced with a get() function?
        if(ModList[i].i2c_address > 0)
        {
            Wire.write(ModList[i].i2c_address);
        }
    }
    Wire.endTransmission();  

    //* We could maybe post an Address Conflict event? for serial service

    return eventOccurred;
}
// TODO Move to private functions and doc comment
uint8_t ScanMode_ValidModule(uint8_t scanmode_address)
{
    uint8_t eventOccurred = FALSE;
    
    if(!doesModuleAlreadyExist(scanmode_address))
    {                
        ES_PostAll((ES_Event){MODULE_CONNECTED,scanmode_address});   
        Speaker_PlaySound(SOUND_CONNECTION_MADE); // TODO Perhaps the speaker should have its own service that can respond to the event?
        eventOccurred = TRUE; 

        // Find first available index (or overwrite the last one)
        uint8_t i = 0;
        for(;(i<(N_MAX_MODULES-1))&&(ModList[i].i2c_address);i++)
            ;

        ModList[i].i2c_address = scanmode_address;
    }

    return eventOccurred;
}

// TODO Rename this function to something more meaningful
uint8_t ScanMode_ValidateSyncBytes(uint8_t address)
{
    // * Important: The sync request register must have already been sent
    
    // TODO Refactor to use API
    //* sync bytes command could be simpler by making it always return 
    //* exactly 1 byte and just repeatedly calling it. It would be slower
    //* potentially, but would better conform to the way we use the control 
    //* and status registers. 

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

// TODO Doc comment
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

uint8_t MonitorMode_CheckNextModule(void)
{
    static uint8_t iModList = 0;
    uint8_t eventOccurred = FALSE;

    // Compute next index to read from
    iModList++;
    if(iModList >= N_MAX_MODULES)
        iModList = 0;

    // Only read if the module is connected
    if(ModList[iModList].i2c_address == 0)
        return FALSE; 

    // Get status
    uint8_t newStatus = 0;
    uint8_t wire_result = TimerI2C_ReadRegister(ModList[iModList].i2c_address, REG_STATUS, &newStatus);
    if(wire_result) // Should be 0 on success
    {
        ES_PostAll((ES_Event){MODULE_DISCONNECTED,ModList[iModList].i2c_address});
        ModList[iModList].i2c_address = 0; // This "removes" it from the array
        Speaker_PlaySound(SOUND_CONNECTION_LOST); // TODO Perhaps the speaker should have its own service that can respond to the event?
        return TRUE;
    }

    // Check if it has become solved
    if((newStatus & _BV(STS_SOLVED)) && !(ModList[iModList].Status & _BV(STS_SOLVED)))
    {
        ES_PostAll((ES_Event){MODULE_DISARMED,ModList[iModList].i2c_address});
        eventOccurred = TRUE;
        ModList[iModList].Status = newStatus;  // Having this call here but also needing it outside the if{} feels dumb. But we need it for the following loop.
        // Check if all are disarmed.
        uint8_t allSolved = 1;
        for(uint8_t i = 0; i < N_MAX_MODULES; i++)
        {
            if((ModList[i].i2c_address > 0) && !(ModList[i].Status & _BV(STS_SOLVED)))
            {
                allSolved = 0;
            }
        }
        if(allSolved)
            ES_PostAll((ES_Event){ALL_MODULES_SOLVED,0});
    }
    
    ModList[iModList].Status = newStatus;     
    
    // Check for a strike    
    if(ModList[iModList].Status & _BV(STS_STRIKE))
    {
        ES_PostAll((ES_Event){MODULE_STRIKE,ModList[iModList].i2c_address});
        eventOccurred = TRUE;
    }

    // Check for a pending request
    if(ModList[iModList].Status & _BV(STS_REQUEST))
    {
        ServiceRequest(ModList[iModList].i2c_address);
    }

    // Check etc. conditions
    if(!(ModList[iModList].Status & _BV(STS_RUNNING)))
    {
        //Serial.println(F("Not running?"));
    }

    return eventOccurred;
}

uint8_t ScanMode_SendSyncRequest(uint8_t module_i2c_address)
{    
    return TimerI2C_SetRegister(module_i2c_address, REG_SYNC, N_SYNC_BYTES);
}

// This function is only useful if you have status monitoring enabled
uint8_t TimerI2C_AreAllModulesRead(void)
{
    uint8_t allModulesReady = 1;
    for(uint8_t i = 0; i < N_MAX_MODULES; i++)
    {
        if(ModList[i].i2c_address > 0)
        {
            if(!(ModList[i].Status & _BV(STS_READY)))
                allModulesReady = 0;
        }
    } 
    return allModulesReady;
}

void ServiceRequest(uint8_t module_i2c_address)
{
    uint8_t wire_result = 0;
    uint8_t request_code = 0;
    uint16_t digits = 0;
    // We assume the request flag is set/already checked
    Wire.beginTransmission(module_i2c_address);
    Wire.write(REG_REQUEST);
    Wire.endTransmission();
    
    Wire.requestFrom(module_i2c_address, (uint8_t)1);
    request_code = Wire.read();

    switch (request_code)
    {
    case REQ_DIGITS:
        digits = Clock_GetDisplayedDigits();
        Wire.beginTransmission(module_i2c_address);
        Wire.write(REG_REQUEST);
        Wire.write(((uint8_t *)&digits)[0]);
        Wire.write(((uint8_t *)&digits)[1]);
        Wire.endTransmission();
        break;    
    default:
        break;
    }
}
