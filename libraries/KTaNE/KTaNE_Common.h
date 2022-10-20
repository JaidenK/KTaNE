/*

Common.h

02/21/2020

Jaiden King

Common functions used between all KTaNE modules.

*/
#ifndef _KTANE_COMMON_H_
#define _KTANE_COMMON_H_

#include <stdint.h>

#include "KTaNE_CommandIDs.h"

extern uint8_t i2c_address;
//extern uint8_t requestType;
//extern uint8_t nSyncBytes;
//extern char moduleName[MODULE_NAME_LENGTH+1];

void writeSyncBytes(uint8_t nBytes);
void reassignI2C(uint8_t reserved[], uint8_t nReserved);

void I2C_Init(int address);

typedef struct
{
    uint8_t SenderAddress;
    uint8_t CommandID;
    uint8_t data[30];
}
I2C_CommandPacket;

extern I2C_CommandPacket LastCommand;

// The module-specific code must implement these functions.
// These are called inside of the ISRs
void ServiceI2CRequest(I2C_CommandPacket *pkt); 
void ReceiveI2CCommand(I2C_CommandPacket *pkt); 
uint8_t I2C_SendPacket(uint8_t address, uint8_t command);
uint8_t I2C_SendPacketEx(uint8_t address, uint8_t *command, uint8_t length);

#endif
