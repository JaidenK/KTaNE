#ifndef _KTANE_REGISTERS_H_
#define _KTANE_REGISTERS_H_

#include <stdint.h>

// Registers used for communication on the I2C bus.
// Refer to ICD for detailed explanation.
// https://docs.google.com/document/d/1BFYc2orsP6i7TOAGHv1l20GYD-Ui4hXYQUHcmJRFHF8/edit?usp=sharing

#define REG_DEVID (0x00)
#define DEVID 0xF5

// TODO Consider checksums/CRCs on I2C comms? 

// TODO Consider making a function API for the registers
// That would allow us to improve the underlying implementation

// Status information for game state and health monitoring. 
// (Read-Only by master)
extern volatile uint8_t STATUS;
#define REG_STATUS  (0x10)
#define STS_STRIKE  (0) // Set when a strike occurs. Automatically clears when the STATUS register is read.
#define STS_SOLVED  (1) // Set when the module is solved.
#define STS_RUNNING (2) // Set when the module is running in game mode (i.e. can now generate strikes or be solved)
#define STS_REQUEST (3) // Indicates the module requires additional info. Read REG_REQUEST.
#define STS_READY   (4) // Indicates the module is ready to start.
#define STS_FAULT   (5) // Indicates some fault has occured. Read REG_FAULT
#define STS_RESULT_READY (6) // Indicates that the results of the self-test are ready. Read REG_TEST_RESULTS

// Allows the master to control the module by setting bits.
// Bits left at 0 will be ignored.
// Write-Only by master. Value is undefined if read.
extern volatile uint8_t CONTROL;
#define REG_CTRL      (0x11)
#define CTRL_START    (0) // Starts the module (enable it to generate strikes/solve)
#define CTRL_RESET    (1) // Resets the module to an Idle state
#define CTRL_LED1     (2) // Toggles the Solve LED
#define CTRL_LED2     (3) // Toggles the Strike LED
#define CTRL_DETONATE (4) // Tells the module that the game was lost
#define CTRL_SEFLTEST (5) // Instructs the module to begin a self-test

//* Idea: We could reserve a control bit or two for user-defined functions, 
//*       then make GUI buttons to trigger them. That would allow for some
//*       level of development without any custom GUI code.
//#define CTRL_UDF1     (6) 

// Indicates to the Timer what information needs to be provided 
// or action needs to be taken.
// The timer provides the required informatin by writing it back into
// the REQUEST register. (over-writting the request ID)
// If the module's request is satisfied, it must clear the request bit
// of the STATUS register.
extern volatile uint8_t REQUEST;
#define REG_REQUEST (0x12)
#define REQ_DIGITS  (0x01) // Request the digits currently being displayed on the Clock

// These registers store no value. They're more commands than single-byte registers.
#define REG_REJOIN  (0x13)
#define REG_SYNC    (0x14)

//#define REG_MODULE_ADDRESSES (0x16)
//* Idea: Change the REJOIN register to an ADDRESSES register that can
//*       be used to store the addresses of the other modules. That would
//*       facilitate moving to a multi-master bus quite well.
//        Then we can have a command register with a bit to trigger a rejoin,
//        and when it rejoins it will avoid using whatever was written into
//        that register.

//* Idea: Extended control register instead of register per command
// #define REG_CTRL      (0x11)
// #define REG_CTRL_EXT  (0x16)
// #define CONTROL_REJOIN (1)

//* Idea: REG_CONFIG a configuration register that essentially acts as a 
//*       wrapper for SET_EEPROM. That could alleviate some of the code
//*       from needing to know exact EEPROM memory addresses.

// TODO Consider the following:
// If we're always broadcasting config info at game start, it doesn't
// really need to be stored in EEPROM. That's more of an error-checking thing
// so that we know exactly what the module is working with.

//* Idea: Input Control register
// A single register that can be used to simulate/overwrite digital inputs
//* Idea: Output Register
// A single register that can read the states of the outputs.
// Input and Output registers combined could be used to automate testing on
// the PC.

//* Idea: Indicate a problem with fault codes
//extern volatile uint8_t FAULT;
//#define REG_FAULT (0x17)
// Implement if we think of useful fault codes.

extern volatile uint8_t TEST_RESULTS;
#define REG_TEST_RESULTS (0x15)
#define SELFTEST_NOT_PERFORMED (0)
#define SELFTEST_SUCCESS       (1)
// Additional self test result values
// reserved for module-specific result
// codes

#define SET_EEPROM 0x51
#define GET_EEPROM 0x52

// UART Command/Packet IDs
#define LIST_MODULES 0xA0

#endif
