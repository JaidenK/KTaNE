/*

Common.h

02/21/2020

Jaiden King

Common functions used between all KTaNE modules.

*/
#ifndef _KTANE_COMMON_H_
#define _KTANE_COMMON_H_

#include <stdint.h>

#include "KTaNE_Constants.h"

extern uint8_t i2c_address;

typedef struct __attribute__((__packed__))
{
    uint8_t CommandID; // TODO Rename to Register
    union 
    {
        uint8_t u8[30];
        uint16_t u16[15];
    } 
    data;
    uint8_t unused;
}
I2C_CommandPacket;
extern volatile I2C_CommandPacket LastCommand;

/**
 * @brief Initializes the "read-only" EEPROM memory used to identify the
 *        module on the PC. 
 */
void KTaNE_InitEEPROM(char *moduleName, char *serialNo, char *buildDate);

/**
 * @brief Initializes the I2C bus with the given address.
 * 
 * @param address The I2C address to use. If -1, the module will join as a master.
 *                If 0, the module will use the address stored in EEPROM. If > 0, 
 *                the module will use the given address.
 */
void KTaNE_I2C_Init(int address);

// Only the Timer should need to reference this directly
void KTaNE_I2C_receive_test(uint8_t length); 

/**
 * @brief Sends a single byte to the given I2C address.
 * 
 * @param address The I2C address to send the packet to.
 * @param command The command to send.
 * 
 * @return 0 if the packet was sent successfully, Wire result otherwise.
 */
uint8_t KTaNE_I2C_SendPacket(uint8_t address, uint8_t command);

/**
 * @brief Sends a packet to the given I2C address.
 * 
 * @param address The I2C address to send the packet to.
 * @param command Pointer to the start of the command data.
 * @param length The length of the command.
 * 
 * @return 0 if the packet was sent successfully, Wire result otherwise.
 */
uint8_t KTaNE_I2C_SendPacketEx(uint8_t address, uint8_t *command, uint8_t length);



#endif
