
#ifndef _ES_EVENTCHECKHEADER_H_
#define _ES_EVENTCHECKHEADER_H_

// This file is used to make sure the events listed in EVENT_CHECK_LIST of ES_Configure.h are visible

#include "Timer_I2C.h"  // TimerI2C_CheckEvents:  MODULE_CONNECTED, MODULE_DISCONNECTED, 
#include "Timer_UART.h" // TimerUART_CheckEvents: PC_CONNECTION_CHANGED
#include "Clock.h"      // Clock_CheckEvents:     CLOCK_EXPIRED

#endif /* _ES_EVENTCHECKHEADER_H_ */