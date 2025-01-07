/*

Constants.h

02/21/2020
Jaiden King

Constants used between all modules for KTaNE.

*/

#ifndef _KTANE_CONSTANTS_H_
#define _KTANE_CONSTANTS_H_

#define TRUE 1
#define FALSE 0

// Max number of modules in the system. There are 12 slots on the physical
// case, but the Timer will always occupy a slot so the practical limit is 11. 
#define N_MAX_MODULES 12

// Number of _pairs_ of sync bytes to send. Within each pair will be a byte 
// and its bitwise complement. An address conflict should show up as a response
// that doesn't have matching bytes. 
#define N_SYNC_BYTES 8

// The baud rate for the serial connection.
#define SERIAL_BAUD_RATE 9600

// The duration of the pulse for the Strike LED when a strike is generated.
#define STRIKE_LED_PULSE_DURATION_MS 1000

// Max length for the serial number edgework 
#define SERIAL_NO_MAX_LENGTH 16

#endif /* _KTANE_CONSTANTS_H_ */
