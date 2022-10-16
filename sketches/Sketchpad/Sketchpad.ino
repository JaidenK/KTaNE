 /*
 * 
 * KTaNE
 * Sketchpad
 * 
 * 10/12/2022
 * 
 * Jaiden King
 * 
 * 
 */

#include <KTaNE.h>
#include <ES_Framework.h>
#include <BOARD_Sketchpad.h>

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
