#ifndef _TIMER_I2C_H_
#define _TIMER_I2C_H_

#include <stdint.h>
#include "KTaNE_Constants.h"

typedef enum
{
    I2C_IDLE,
    I2C_SCANNING,
    I2C_MONITORING,
}
I2CMode_t;

// TODO doc comment this struct and make it private
typedef struct 
{
    uint8_t i2c_address;
    uint8_t Status; // Status register
}
ModuleInfo;

// TODO Refactor to remove this extern variable
// Currently used in: Timer_UART
extern ModuleInfo ModList[N_MAX_MODULES];

/**
 * @brief Controls the I2C bus and posts events based on the mode.
 * 
 * The main loop function for all the I2C hardware abstraction. 
 * Listed in EVENT_CHECK_LIST in ES_Configure.h. This software module also
 * encapsulates all behavior required to respond to requests made
 * by other Modules on the I2C bus. 
 * 
 * @note The serial command processor also interacts with the I2C bus 
 *       in order to pass commands from the PC.
 * *Idea: Maybe we change that. Have the serial command processor call
 * *      a function from this module.
 * 
 * Posts:
 * {MODULE_CONNECTED,<i2c address>}    - When a new valid KTaNE module is found
 *                                       on the I2C bus.
 * {MODULE_DISCONNECTED,<i2c address>} - When a module stops responding. 
 * {MODULE_DISARMED,<i2c address>}     - When the status bit indicates the 
 *                                       module is solved.
 * {MODULE_STRIKE,<i2c address>}       - When the status bit indicates a strike
 * 
 */
uint8_t TimerI2C_CheckEvents(void);

/**
 * The timer is the I2C bus master and has 2 different "steady state" 
 * behaviors. It can either be scanning all addresses to see if any modules
 * are being added or removed. This is a slow process and used only around 
 * startup/configuration. The second mode is a faster "monitoring" mode where
 * it gathers the status bytes from the connected modules. The behaviour is
 * implemented in TimerI2C_CheckEvents and this function simply changes the
 * flags.
 * 
 * ! Likely to be deprecated
 * 
 * ? Should this method be exposed in the public API?
 */
void TimerI2C_SetMode(I2CMode_t mode);

//* Idea: Fully encapsulate the module list inside TimerI2C and just rely 
//*       on the events it posts
//void TimerI2C_StartScanning(void);
//void TimerI2C_StartMonitoring(void);
//* Idea: uint8_t TimerI2C_StartConfiguring()

// TODO Evaluate if these functions are needed
I2CMode_t TimerI2C_GetMode(void);
void TimerI2C_ScanAllAddressesNow_Blocking(void);

//* Idea: uint8_t TimerI2C_AllModulesSolved(void);
// This would help FSM

/**
 * @brief Causes a module on the bus to toggle its LEDs by sending 
 *        multiple commands sequentially with delays between them. 
 * 
 * The LEDs will end back in their original state.
 */
void TimerI2C_FlashLEDs_Blocking(uint8_t module_i2c_address);

/**
 * @brief Toggles both the Strike and Disarm LED of the specified module.
 */
void TimerI2C_ToggleLEDs(uint8_t module_i2c_address);

/**
 * @brief Writes a value to the specified register of all connected modules.
 * 
 * @note May become deprecated in favor of ResetAll, StartAll, etc.
 * 
 * Posts:
 * {MODULE_DISCONNECTED,<i2c address>} - If there's an issue with the 
 *                                       transmission.
 */
void TimerI2C_BroadcastAllModules(uint8_t register_address, uint8_t value);

/**
 * @brief Sets the Reset bit of all modules' Control register.
 */
void TimerI2C_ResetAllModules(void);

/**
 * @brief Sets the Reset bit of all modules' Control register.
 */
void TimerI2C_StartAllModules(void);

/**
 * @brief Sets the value of a single register for a single module.
 * 
 * @returns Wire.endTransmission()
 */
uint8_t TimerI2C_SetRegister(uint8_t module_i2c_address, uint8_t register_address, uint8_t value);

//TODO Doc comment
uint8_t TimerI2C_ReadRegister(uint8_t module_i2c_address, uint8_t register_address, uint8_t *out);


void TimerI2C_StartBusScan(void);

/**
 * @brief Enables the sanning behavior of the event checker to see if new
 *        modules are being added.
 */
void TimerI2C_EnableBusScanning(void);
void TimerI2C_DisableBusScanning(void);
void TimerI2C_EnableStatusMonitoring(void);
void TimerI2C_DisableStatusMonitoring(void);

void TimerI2C_BroadcastGameConfigInfo(void);
uint8_t TimerI2C_AreAllModulesRead(void);

#endif /* _TIMER_I2C_H_ */