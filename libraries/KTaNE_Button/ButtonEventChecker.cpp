#include "ButtonEventChecker.h"
#include "ES_Framework.h"
#include "BOARD_Button.h"
#include "Arduino.h"


uint8_t ButtonCurrentState = 0;
uint8_t BtnStates = 0;


uint8_t CheckButtons(void)
{
    uint8_t returnVal = 0;

    uint16_t eventParam = 0;

    BtnStates = (BtnStates << 1) + digitalRead(BTN_PIN);

    if(ButtonCurrentState)
    {
        if(BtnStates == 0)
        {
            ButtonCurrentState = 0;
            eventParam |= (1 << 8);
            returnVal = 1;
        }
    }
    else
    {
        if(BtnStates == 0xFF)
        {
            ButtonCurrentState = 1;
            eventParam |= (1 << 8);
            returnVal = 1;
        }
    }

    if(eventParam)
    {
        returnVal = 1;
        if(ButtonCurrentState)
            eventParam |= (1 << 0);
        ES_PostAll((ES_Event){BUTTON_EVENT,eventParam});
    }

    return (returnVal);
}