#ifndef _TIMER_CONFIGURE_H_
#define _TIMER_CONFIGURE_H_

#include <stdint.h>
#include "KTaNE_Common.h"
#include "KTaNE_Constants.h"

extern uint32_t timeLimit;

void LoadAllEepromConfigInfo();
uint16_t getTimeLimist_s();

#endif /* _TIMER_CONFIGURE_H_ */