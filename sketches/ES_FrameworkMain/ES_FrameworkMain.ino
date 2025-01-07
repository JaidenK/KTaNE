 /*
 * Generic ES Framework Main Sketch File
 * 
 * 01/06/2025
 * 
 * Jaiden King
 * 
 * This code is used to run any project using the standard Events and Services 
 * architecture. You must put the ES Framework and your code in your Arduino library path.
 */

#include <ES_Framework.h>
#include <BOARD.h>

void setup()
{
    ES_Return_t ErrorType;

    BOARD_Init(); // TODO This could be in the state machine init function or something and then this .ino file would be the same for all modules
  
    Serial.println(F("Board initialized."));

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
