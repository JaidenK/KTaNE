#ifndef _MODULESINTERACTION_H_
#define _MODULESINTERACTION_H_

#include <stdint.h>
#include "KTaNE.h"

typedef struct 
{
    uint8_t i2c_address;
    //char model_id[N_MAX_MODULE_ID_CHARS];    
    //uint8_t isDisarmed;
    uint8_t Status; // Status register
}
ModuleInfo;

extern ModuleInfo ModList[N_MAX_MODULES];

uint8_t ScanForModules(); 
uint8_t GetStatusAllModules();
void print2digithex(uint8_t value);
//uint8_t PingExistingModules();

#endif /* _MODULESINTERACTION_H_ */