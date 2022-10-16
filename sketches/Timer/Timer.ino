 /*
 * Timer
 * 
 * 02/21/2020
 * 
 * Jaiden King
 * 
 * This is the code for the Timer module of the Keep Talking and Nobody Explodes device.
 * It will read it's EEPROM for i2c addresses and module types, then scan the i2c bus
 * to make sure the modules all exist. If a module was removed or added, then the user
 * will be prompted to reconfigure. If the cannot reconfigure, the module will be sent
 * into a floating state (led flashing) to indicate that it's not configured.
 * 
 */

#include <KTaNE.h>
#include <ES_Framework.h>
#include <BOARD_Timer.h>

void setup()
{
    ES_Return_t ErrorType;

    BOARD_Init();
  
    Serial.println("Board initialized.");

    // Initialize the framework
    ErrorType = ES_Initialize();
    if (ErrorType == Success) {
        ErrorType = ES_Run();
    }
    
    //if we got to here, there was an error
    switch (ErrorType) {
    case FailedPointer:
        printf("Failed on NULL pointer\n");
        break;
    case FailedInit:
        printf("Failed Initialization\n");
        break;
    default:
        printf("Other Failure: %d\n", ErrorType);
        break;
    }
    for (;;)
        ;
}

void loop()
{
}
