/*
 * File: TemplateFSM.c
 * Author: J. Edward Carryer
 * Modified: Gabriel H Elkaim
 *
 * Template file to set up a Flat State Machine to work with the Events and Services
 * Frameword (ES_Framework) on the Uno32 for the CMPE-118/L class. Note that this file
 * will need to be modified to fit your exact needs, and most of the names will have
 * to be changed to match your code.
 *
 * This is provided as an example and a good place to start.
 *
 *Generally you will just be modifying the statenames and the run function
 *However make sure you do a find and replace to convert every instance of
 *  "Template" to your current state machine's name
 * History
 * When           Who     What/Why
 * -------------- ---     --------
 * 09/13/13 15:17 ghe      added tattletail functionality and recursive calls
 * 01/15/12 11:12 jec      revisions for Gen2 framework
 * 11/07/11 11:26 jec      made the queue static
 * 10/30/11 17:59 jec      fixed references to CurrentEvent in RunTemplateSM()
 * 10/23/11 18:20 jec      began conversion from SMTemplate.c (02/20/07 rev)
 */


/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>

#include "BOARD.h"

#include "ES_Configure.h"
#include "ES_Framework.h"

#include "SketchpadFSM.h"
#include "KTaNE.h"




/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/


/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this machine. They should be functions
   relevant to the behavior of this state machine.*/


/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                    *
 ******************************************************************************/

/* You will need MyPriority and the state variable; you may need others as well.
 * The type of state variable should match that of enum in header file. */

typedef enum {
    InitPState,
    Idle,
    Running,
    Solved,
    N_STATES,
} FSMState_t;

static const char *StateNames[] = {
    "InitPState",
    "Idle",
    "Running",
    "Solved",
    "N_STATES",
};


FSMState_t CurrentState = InitPState; // <- change enum name to match ENUM
static uint8_t MyPriority;

static uint8_t moduleI2Crequest = 0; 

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/
 
uint8_t InitSketchpadFSM(uint8_t Priority)
{
    MyPriority = Priority;
    // put us into the Initial PseudoState
    CurrentState = InitPState;

    // post the initial transition event
    if (ES_PostToService(MyPriority, INIT_EVENT) == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
    
    Serial.println(F("Sketchpad FSM Initialized"));
}

void FlashBlocking()
{
    for (int i = 0; i < 5; i++)
    {
        digitalWrite(STRIKE_PIN,1);
        delay(200);
        digitalWrite(STRIKE_PIN,0);
        delay(200);
    }
    
}

uint8_t PostSketchpadFSM(ES_Event ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event RunSketchpadFSM(ES_Event ThisEvent)
{
    uint8_t makeTransition = FALSE; // use to flag transition
    FSMState_t nextState; // <- need to change enum type here

    if(ThisEvent.EventType == ES_ENTRY)
    {
        Serial.print(F("Entering state: "));
        Serial.println(StateNames[CurrentState]);
    }

    switch (CurrentState) {
    case InitPState: // If current state is initial Psedudo State
        if (ThisEvent.EventType == ES_ENTRY || 
            ThisEvent.EventType == ES_INIT)// only respond to ES_Init
        {
            KTaNE_InitEEPROM("SKETCHPAD       ",
                             "SKC001          ",
                             "12/18/2024 15:30");  

            // now put the machine into the actual initial state
            nextState = Idle;
            makeTransition = TRUE;
            ThisEvent.EventType = ES_NO_EVENT;
        }
        break;
    case Idle:
        if(ThisEvent.EventType == ES_ENTRY)
        {
            digitalWrite(DISARM_PIN, LOW);
            digitalWrite(STRIKE_PIN, LOW);
            STATUS |= _BV(STS_READY);
            STATUS &= ~_BV(STS_RUNNING);   
            STATUS &= ~_BV(STS_SOLVED);    
            STATUS &= ~_BV(STS_STRIKE);    
        }
        if(ThisEvent.EventType == EVENT_START)
        {
            ThisEvent.EventType = ES_NO_EVENT;
            nextState = Running;
            makeTransition = TRUE;
        }
        break;
    case Running:
        if(ThisEvent.EventType == ES_ENTRY)
        {
            STATUS |= _BV(STS_RUNNING);
            //digitalWrite(DISARM_PIN, HIGH);
            //digitalWrite(STRIKE_PIN, HIGH);
            //delay(100);
            //digitalWrite(DISARM_PIN, LOW);
            //digitalWrite(STRIKE_PIN, LOW);
        }
        if(ThisEvent.EventType == BUTTON_EVENT)
        {            
            Serial.print("Param: ");
            Serial.println(ThisEvent.EventParam,HEX);
            uint8_t strikeState = (ThisEvent.EventParam & 0x0001);
            uint8_t disarmState = (ThisEvent.EventParam & 0x0002);
            Serial.print("State: ");
            Serial.println(strikeState);
            Serial.println(disarmState);
            
            // Buttons are active low, LEDs are active high
            digitalWrite(DISARM_PIN,!disarmState);
            digitalWrite(STRIKE_PIN,!strikeState);

            // check which button changed AND its state
            if((ThisEvent.EventParam & 0x0101) == 0x0100)
            {
                //I2C_SendPacket(TIMER_I2C_ADDRESS, STRIKE);
                STATUS |= _BV(STS_STRIKE);
            }
            else if((ThisEvent.EventParam & 0x0202) == 0x0200)
            {
                //I2C_SendPacket(TIMER_I2C_ADDRESS, SOLVED);  
                ThisEvent.EventType = ES_NO_EVENT;
                nextState = Solved;
                makeTransition = TRUE;              
            }                    
        }
        else if(ThisEvent.EventType == FLASH_REQUESTED)
        {
            FlashBlocking();
            ThisEvent.EventType = ES_NO_EVENT;
        }    
        else if(ThisEvent.EventType == EVENT_RESET)
        {
            ThisEvent.EventType = ES_NO_EVENT;
            nextState = Idle;
            makeTransition = TRUE;
        }
        break;
    case Solved:
        if(ThisEvent.EventType == ES_ENTRY)
        {            
            digitalWrite(DISARM_PIN, HIGH);
            digitalWrite(STRIKE_PIN, LOW);
            STATUS |= _BV(STS_SOLVED);
        }
        else if(ThisEvent.EventType == EVENT_RESET)
        {
            ThisEvent.EventType = ES_NO_EVENT;
            nextState = Idle;
            makeTransition = TRUE;
        }
        break;
    default: // all unhandled states fall into here
        break;
    } // end switch on Current State

    // Clear the serial buffer if it's still there
    // while(Serial.available())
    // {
    //     Serial.readString();
    //     Serial.println(F("..."));
    // }

    if (makeTransition == TRUE) { // making a state transition, send EXIT and ENTRY
        // recursively call the current state with an exit event
        RunSketchpadFSM(EXIT_EVENT);
        CurrentState = nextState;
        RunSketchpadFSM(ENTRY_EVENT);
    }
    ES_Tail(); // trace call stack end
    return ThisEvent;
}


void Module_ToggleStrikeLED()
{
    digitalWrite(STRIKE_PIN,!digitalRead(STRIKE_PIN));
}

void Module_ToggleSolveLED()
{
    digitalWrite(DISARM_PIN,!digitalRead(DISARM_PIN));
}

void Module_Detonate()
{
    // TODO
}

void Module_PerformSelfTest()
{
    // TODO
}

/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/
