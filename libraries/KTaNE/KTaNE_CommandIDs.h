/* 
 * File:   KTaNE_CommandIDs.h
 * Author: Jaiden King
 *
 * https://docs.google.com/document/d/1BFYc2orsP6i7TOAGHv1l20GYD-Ui4hXYQUHcmJRFHF8/edit?usp=sharing 
 *
 * Created on September 24, 2022
 */

#ifndef _KTANE_COMMANDIDS_H_
#define	_KTANE_COMMANDIDS_H_

typedef enum 
{
    INVALID_COMMAND = 0x00,
    
    // Initialization commands
    REQUEST_SYNC = 0x10,        // Ask a module to respond with sync pairs. 
    REASSIGN_I2C_ADDRESS,       // Command a module to randomize its I2C address 
    REQUEST_NAME,               // Ask a module to proivde its ASCII name
    REQUEST_ID,                 // Ask a module to return its model ID. This must be a N_MODULE_ID_CHARS length ASCII string.
    REQUEST_STATE,              // Ask a module what its current state is (idle, unsolved, solved, etc.)
    REQUEST_CONFIG,             // Ask for the state of the following command. (command is transmitted with a second command as its argument)
    RESET,                      // Command a module to return to its initial state

    // Gameplay commands
    START = 0x30,               // Begin the game. Can be sent to the timer, and is also broadcast by the timer.
    DETONATE,                   // Command a module to detonate. This triggers a game over when the timer receives it, and will be broadcast by the timer when the time expires
    DEFUSED,                    // Broadcast from the timer when all modules have been solved.
    STRIKE,                     // Inform the timer that a strike has occured
    SOLVED,                     // Inform the timer that the module has been solved
    UNSOLVED,                   // Inform the timer that the module has been un-solved (needy modules)
    GET_TIME_MS,                // Get number of milliseconds remaining in game
    GET_TIME_DIGITS,            // Get the digits displayed on the timer

    // Configuration commands
    // These are sent from the GUI to the Timer, 
    // then the Timer broadcasts them to the modules
    FLASH_LED = 0x50,           // Make the module flash its LED to signal which module is being controlled
    SET_EEPROM,
    GET_EEPROM,
    SET_SERIAL_NO,              // Inform a module what the serial number is ()
    SET_N_BATTERIES,            // Inform a module how many batteries are on the bomb
    SET_INDICATOR,              // Inform a module of an indicator that is installed, and its state. State of -1 means clear list.
    SET_PORT,                   // Inform a module of a port that is installed. -1 means clear list   
    SET_TIME_LIMIT,

    // UART commands
    ACK = 0x60,                 // Parameter may include response bytes 
    NACK,
    MODULE_ADDED,               // Parameter = Address, model ID
    MODULE_REMOVED,             // 
    REQUEST_MODULE_LIST,        // Triggers the Timer to list out all the currently connected modules (as sequential serial packets)

    // Reserved command IDs
    CUSTOM_COMMANDS = 0x70, 
    LIST_MODULES = 0xA0,
}
I2C_Command_ID;


#define REG_DEVID (0x00)
#define DEVID 0xF5

extern volatile uint8_t STATUS;
#define REG_STATUS  (0x10)
#define STS_STRIKE  (0)
#define STS_SOLVED  (1)
#define STS_RUNNING (2)
#define STS_REQUEST (3)
#define STS_READY   (4)

extern volatile uint8_t CONTROL;
#define REG_CTRL      (0x11)
#define CTRL_START    (0)
#define CTRL_RESET    (1)
#define CTRL_LED1     (2)
#define CTRL_LED2     (3)
#define CTRL_DETONATE (4)

extern volatile uint8_t REQUEST;
#define REG_REQUEST (0x12)
#define REQ_DIGITS  (0x01)

#define REG_REJOIN  (0x13)
#define REG_SYNC    (0x14)


#endif /* _KTANE_COMMANDIDS_H_ */