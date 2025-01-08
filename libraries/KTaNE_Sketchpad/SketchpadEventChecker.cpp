#include "SketchpadEventChecker.h"
#include "ES_Framework.h"
#include "BOARD.h"
#include "Arduino.h"


uint8_t StrikeButtonCurrentState = 0;
uint8_t DisarmButtonCurrentState = 0;
uint8_t BtnStateRegister = 0;
uint8_t DisarmBtnStates = 0;


uint8_t CheckButtons(void)
{
    uint8_t returnVal = 0;

    uint16_t eventParam = 0;

    BtnStateRegister = (BtnStateRegister << 1) + digitalRead(STRIKE_BTN);
    DisarmBtnStates = (DisarmBtnStates << 1) + digitalRead(DISARM_BTN);

    if(StrikeButtonCurrentState)
    {
        if(BtnStateRegister == 0)
        {
            StrikeButtonCurrentState = 0;
            eventParam |= (1 << 8);
            returnVal = 1;
        }
    }
    else
    {
        if(BtnStateRegister == 0xFF)
        {
            StrikeButtonCurrentState = 1;
            eventParam |= (1 << 8);
            returnVal = 1;
        }
    }
    
    if(DisarmButtonCurrentState)
    {
        if(DisarmBtnStates == 0)
        {
            DisarmButtonCurrentState = 0;
            eventParam |= (1 << 9);
            returnVal = 1;
        }
    }
    else
    {
        if(DisarmBtnStates == 0xFF)
        {
            DisarmButtonCurrentState = 1;
            eventParam |= (1 << 9);
            returnVal = 1;
        }
    }

    if(eventParam)
    {
        returnVal = 1;
        if(StrikeButtonCurrentState)
            eventParam |= (1 << 0);
        if(DisarmButtonCurrentState)
            eventParam |= (1 << 1);
        ES_PostAll((ES_Event){BUTTON_EVENT,eventParam});
    }

    return (returnVal);
}