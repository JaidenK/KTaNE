 /*
 * 
 * KTaNE
 * Complicated Wires
 * 
 * 12/30/2024
 * 
 * Jaiden King
 * 
 * 
 */

#include <KTaNE.h>
#include <ES_Framework.h>
#include <BOARD_ComplicatedWires.h>

void setup()
{
    ES_Return_t ErrorType;

    BOARD_Init();
  
    //Serial.println("Board initialized.");

    // Initialize the framework
    ErrorType = ES_Initialize();
    if (ErrorType == Success) {
        ErrorType = ES_Run();
    }
    
    //if we got to here, there was an error
    switch (ErrorType) {
    case FailedPointer:
        Serial.println(F("Failed on NULL pointer"));
        break;
    case FailedInit:
        Serial.println(F("Failed Initialization"));
        break;
    default:
        Serial.print(F("Other Failure: "));
        Serial.print(ErrorType);
        Serial.println();
        break;
    }    
    for (;;)
        ;
}

void loop()
{
}
