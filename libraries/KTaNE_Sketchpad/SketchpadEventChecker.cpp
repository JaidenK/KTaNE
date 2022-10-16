#include "SketchpadEventChecker.h"
#include "ES_Framework.h"
#include "BOARD_Sketchpad.h"
#include "Arduino.h"


uint8_t StrikeButtonCurrentState = 0;
uint8_t DisarmButtonCurrentState = 0;
uint8_t StrikeBtnStates = 0;
uint8_t DisarmBtnStates = 0;


uint8_t CheckButtons(void)
{
    uint8_t returnVal = 0;

    uint16_t eventParam = 0;

    StrikeBtnStates = (StrikeBtnStates << 1) + digitalRead(STRIKE_BTN);
    DisarmBtnStates = (DisarmBtnStates << 1) + digitalRead(DISARM_BTN);

    if(StrikeButtonCurrentState)
    {
        if(StrikeBtnStates == 0)
        {
            StrikeButtonCurrentState = 0;
            eventParam |= (1 << 8);
        }
    }
    else
    {
        if(StrikeBtnStates == 0xFF)
        {
            StrikeButtonCurrentState = 1;
            eventParam |= (1 << 8) + (1 << 0);
        }
    }
    
    if(DisarmButtonCurrentState)
    {
        if(DisarmBtnStates == 0)
        {
            DisarmButtonCurrentState = 0;
            eventParam |= (1 << 9);
        }
    }
    else
    {
        if(DisarmBtnStates == 0xFF)
        {
            DisarmButtonCurrentState = 1;
            eventParam |= (1 << 9) + (1 << 1);
        }
    }

    if(eventParam)
    {
        returnVal = 1;
        ES_PostAll((ES_Event){BUTTON_EVENT,eventParam});
    }

    return (returnVal);
}