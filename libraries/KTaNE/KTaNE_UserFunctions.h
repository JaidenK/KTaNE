#ifndef _KTANE_USERFUNCTIONS_H_
#define _KTANE_USERFUNCTIONS_H_

// Functions defined in this header file must be 
// implemented by the module-specific code. This is to 
// allow for hardware-dependent actions.

// Note that even if the implementation ends up being 
// identical for all modules, it may still worth keeping
// certain features for potential modders.

// Called in the interrupt handler that runs when I2C data 
// is requested by a master only if the request is not 
// automatically handled by the KTaNE Library.
void Module_ServiceI2CRequest(I2C_CommandPacket *pkt); 

// Called in the interrupt handler that receives I2C data from a 
// master if the data is not automatically handled by the KTaNE Library.
void Module_ReceiveI2CCommand(I2C_CommandPacket *pkt, uint8_t length); 

// The module should toggle the current state of its Strike LED.
// This should NOT cause a strike.
// Called in the I2C Receive interrupt.
void Module_ToggleStrikeLED(void);

// The module should toggle the current state of its Solved LED.
// This should NOT cause a Solve.
// Called in the I2C Receive interrupt.
void Module_ToggleSolveLED(void);

// Module-specific detonation behavior. 
// Typically turning on the Strike LED.
// Called in the I2C Receive interrupt.
void Module_Detonate(void);

// Module-specific hardware test.
// Called in the I2C Receive interrupt.
void Module_PerformSelfTest(void); // If they don't set the STS_RESULT_READY flag, you'll have to account for a timeout.


#endif 
