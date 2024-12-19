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

#include "BOARD_Timer.h"
#include "Timer_Configure.h"
#include "ModulesInteraction.h"

#include "ES_Configure.h"
#include "ES_Framework.h"

#include "TimerFSM.h"
#include "KTaNE.h"
#include "ClockEventChecker.h"
#include "SerialManager.h"

#include "frequencies.h"


/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/


/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this machine. They should be functions
   relevant to the behavior of this state machine.*/
void initModules(void);

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                    *
 ******************************************************************************/

/* You will need MyPriority and the state variable; you may need others as well.
 * The type of state variable should match that of enum in header file. */

typedef enum {
    InitPState,
    Idle,
    Counting,
    Exploding,
    N_STATES,
} TimerFSMState_t;

static const char *StateNames[] = {
    "InitPState",
    "Idle",
    "Counting",
    "Exploding",
    "N_STATES",
};


TimerFSMState_t CurrentState = InitPState; // <- change enum name to match ENUM
static uint8_t MyPriority;

static uint8_t nStrikes = 0;
static uint8_t allModulesDisarmed = 0;

static uint8_t moduleI2Crequest = 0; 

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

void FlashBlocking()
{
    for (int i = 0; i < 5; i++)
    {
        digitalWrite(STRIKE1_PIN,1);
        delay(200);
        digitalWrite(STRIKE1_PIN,0);
        delay(200);
    }
    
}

uint8_t InitTimerFSM(uint8_t Priority)
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
}


uint8_t PostTimerFSM(ES_Event ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event RunTimerFSM(ES_Event ThisEvent)
{
    uint8_t makeTransition = FALSE; // use to flag transition
    TimerFSMState_t nextState; // <- need to change enum type here

    if(ThisEvent.EventType == ES_ENTRY)
    {
        Serial.print("Entering state: ");
        Serial.println(StateNames[CurrentState]);
    }

    switch (CurrentState) {
    case InitPState: // If current state is initial Psedudo State
        if (ThisEvent.EventType == ES_ENTRY || 
            ThisEvent.EventType == ES_INIT)// only respond to ES_Init
        {            
            LoadAllEepromConfigInfo();
            SendUARTCommandByte(i2c_address,RESET);

            // now put the machine into the actual initial state
            nextState = Idle;
            makeTransition = TRUE;
            ThisEvent.EventType = ES_NO_EVENT;
        }
        break;
    case Idle:
        switch (ThisEvent.EventType)
        {
        case I2C_CMD_RECEIVED:
            if(LastCommand.CommandID == SOLVED)
            {
                Serial.print(F("SOLVED signal received from address "));
                print2digithex(LastCommand.SenderAddress);
                Serial.println();
                ThisEvent.EventType = ES_NO_EVENT;
            }
            else if(LastCommand.CommandID == STRIKE)
            {
                Serial.print(F("STRIKE signal received from address "));
                print2digithex(LastCommand.SenderAddress);
                Serial.println();
                ThisEvent.EventType = ES_NO_EVENT;
            }
            else
            {
                Serial.print(F("Unhandled I2C command."));
            }
            break;
        case ES_TIMEOUT:
        case ES_ENTRY:
            StartPseudoTimer(0, 3000);
            ScanForModules();
            ThisEvent.EventType = ES_NO_EVENT;
            break;
        case FLASH_REQUESTED:
            FlashBlocking();
            ThisEvent.EventType = ES_NO_EVENT;
            break;
        default:
            break;
        }
        break;
    default: // all unhandled states fall into here
        break;
    } // end switch on Current State

    ServiceSerial();

    if(ThisEvent.EventType != ES_NO_EVENT)
    {
        Serial.print(F("Unhandled event: "));
        print2digithex(ThisEvent.EventType);
        Serial.println();
    }

    if (makeTransition == TRUE) { // making a state transition, send EXIT and ENTRY
        // recursively call the current state with an exit event
        RunTimerFSM(EXIT_EVENT);
        CurrentState = nextState;
        RunTimerFSM(ENTRY_EVENT);
    }
    ES_Tail(); // trace call stack end
    return ThisEvent;
}


/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/


