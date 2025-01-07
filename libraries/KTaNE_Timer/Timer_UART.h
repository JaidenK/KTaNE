#ifndef _TIMER_UART_H_
#define _TIMER_UART_H_

#include <stdint.h>

/**
 * @brief Receives and transmits serial data between the PC and Timer.
 * 
 * Abstraction layer for the Serial communication protocol. All functions are
 * blocking (no FIFO in use). Listed in EVENT_CHECK_LIST in ES_Configure.h.
 * 
 * When full Packets are received by this layer, they'll be sent in a blocking
 * way to the SerialCommandProcessor, which starts starts I2C communication.
 * * Idea: We may want to make the SerialCommandProcessor into a service that
 * *       receives events from the UART layer. 
 * 
 * ! Don't Commit
 * * SerialCommandProcessor is the DJ of this party. FSM is the leader.
 * 
 * Posts:
 * {PC_CONNECTION_CHANGED,<new state>} - When the connection status of the PC
 *                                       changes.
 * 
 * @note SerialCommandProcessor may post its own events in response to the UART
 */
uint8_t TimerUART_CheckEvents(void);

// TODO Is this the right way to do this?
// TODO We need an Test Procedure for this
// TODO We need unit tests for this
// TODO Robust software architecture to enhance your play experience. Good for modders

//* Idea: We could have a Service here to "passively" send all Events to the 
//*       PC as packets.

/**
 * @brief Sends data to the PC.
 * 
 * Generates the necessary header and CRC information required to send the
 * passed bytes array to the PC.
 * 
 * @param address The I2C address that this message is originating from or 
 *                in regard to. 
 * @param bytes Arbitrary data, but typically an I2C packet.
 */
void TimerUART_SendPacket(uint8_t address, uint8_t *bytes, uint8_t nBytes);

/**
 * @brief Sends 1 byte to the PC.
 * 
 * Just a wrapper for TimerUART_SendPacket to send a single byte.
 */
void TimerUART_SendCommandByte(uint8_t address, uint8_t byte);

/**
 * @brief Sends data to the PC, indicating a response from the specified 
 *        address.
 * 
 * Just a wrapper for TimerUART_SendPacket to add the response bit.
 */
void TimerUART_SendResponse(uint8_t address, uint8_t *bytes, uint8_t nBytes);

/**
 * @returns The current state of the Serial connection. 
 *          1: Connected
 *          0: Disconnected
 */
uint8_t TimerUART_IsPCConnected(void);

void TimerUART_EnableStatusTransmissions(void);
void TimerUART_DisableStatusTransmissions(void);

#endif /* _TIMER_I2C_H_ */