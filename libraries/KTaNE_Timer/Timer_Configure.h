#ifndef _TIMER_CONFIGURE_H_
#define _TIMER_CONFIGURE_H_

#include <stdint.h>
#include "KTaNE_Common.h"
#include "KTaNE_Constants.h"

extern uint32_t timeLimit;
// extern char serial_no[SERIAL_NO_MAX_LENGTH];
// extern uint8_t nBatteries;
// extern Indicator indicators[N_MAX_INDICATORS];
// extern PortType ports[N_MAX_PORTS];

void SetTimeLimitConfig(uint32_t newTimeLimit);
void SetNumBatteriesConfig(uint8_t newNBatteries);
void SetSerialNoConfig(char *newSerialNo);

void LoadAllEepromConfigInfo();

#endif /* _TIMER_CONFIGURE_H_ */