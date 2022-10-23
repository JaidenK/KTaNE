#ifndef _SERIAL_MANAGER_H_
#define _SERIAL_MANAGER_H_

#include <stdint.h>

#define SYNC_BYTE 0xA5

void ServiceSerial();

void SendUARTCommandByte(uint8_t address, uint8_t byte);
void SendUARTCommand(uint8_t address, uint8_t *bytes, uint8_t nBytes);
void SendUARTResponse(uint8_t address, uint8_t *bytes, uint8_t nBytes);

#endif // _SERIAL_MANAGER_H_
