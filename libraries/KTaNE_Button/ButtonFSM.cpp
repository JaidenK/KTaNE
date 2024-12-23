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

#include "BOARD_Button.h"

#include "ES_Configure.h"
#include "ES_Framework.h"

#include "ButtonFSM.h"
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
} TimerFSMState_t;

static const char *StateNames[] = {
    "InitPState",
    "Idle",
    "Running",
    "Solved",
    "N_STATES",
};


TimerFSMState_t CurrentState = InitPState; // <- change enum name to match ENUM
static uint8_t MyPriority;

static uint8_t moduleI2Crequest = 0; 

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/
 
uint8_t InitButtonFSM(uint8_t Priority)
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
    
    Serial.println(F("Button FSM Initialized"));
}

void LoadEEPROMConfig()
{
    char *moduleName =  "THE BUTTON      ";
    char *serialNo =    "BTN001          "; // This should be unique for every individual module. IDK how to ensure that right now.
    char *buildDate =   "12/22/2024 17:30"; // Bless your heart if you actually remember to update this datetime each time the code changes.
    for(uint8_t i=0; i<16; i++)
    {
        EEPROM.update(EEPROM_MODULE_NAME+i,moduleName[i]);
        EEPROM.update(EEPROM_REAL_MODULE_SERIAL+i,serialNo[i]); 
        EEPROM.update(EEPROM_BUILD_DATE+i,buildDate[i]); 
    }
}

void StartupFlash()
{
    digitalWrite(STRIKE_PIN,HIGH);     
    delay(200);
    digitalWrite(STRIKE_PIN,LOW);  
    digitalWrite(DISARM_PIN,HIGH);     
    delay(200);
    digitalWrite(DISARM_PIN,LOW);  
    digitalWrite(PIN_RED,HIGH);     
    delay(200);
    digitalWrite(PIN_RED,LOW);  
    digitalWrite(PIN_GREEN,HIGH);      
    delay(200);
    digitalWrite(PIN_GREEN,LOW);  
    digitalWrite(PIN_BLUE,HIGH);     
    delay(200);
    digitalWrite(PIN_BLUE,LOW);  
}

uint8_t PostButtonFSM(ES_Event ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event RunButtonFSM(ES_Event ThisEvent)
{
    uint8_t makeTransition = FALSE; // use to flag transition
    TimerFSMState_t nextState; // <- need to change enum type here

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
            LoadEEPROMConfig();
            // now put the machine into the actual initial state
            nextState = Idle;
            makeTransition = TRUE;
            ThisEvent.EventType = ES_NO_EVENT;      
            
            StartupFlash();
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
            STATUS &= ~_BV(STS_REQUEST);
            REQUEST = 0;
        }
        if(ThisEvent.EventType == EVENT_START)
        {
            ThisEvent.EventType = ES_NO_EVENT;
            nextState = Running;
            makeTransition = TRUE;
        }
        if(ThisEvent.EventType == BUTTON_EVENT)
        {
            digitalWrite(STRIKE_PIN,!(ThisEvent.EventParam & 1));
            digitalWrite(DISARM_PIN,!(ThisEvent.EventParam & 1));
        }
        break;
    case Running:
        if(ThisEvent.EventType == ES_ENTRY)
        {
            STATUS |= _BV(STS_RUNNING);
        }
        if(ThisEvent.EventType == BUTTON_EVENT)
        {       
            if(ThisEvent.EventParam & 1) // On release
            {
                // Request digits
                STATUS |= _BV(STS_REQUEST);
                REQUEST = REQ_DIGITS;
            }
            // Todo
            //STATUS |= _BV(STS_STRIKE);    
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
        RunButtonFSM(EXIT_EVENT);
        CurrentState = nextState;
        RunButtonFSM(ENTRY_EVENT);
    }
    ES_Tail(); // trace call stack end
    return ThisEvent;
}

void ToggleStrikeLED()
{
    digitalWrite(STRIKE_PIN,!digitalRead(STRIKE_PIN));
}

void ToggleSolveLED()
{
    digitalWrite(DISARM_PIN,!digitalRead(DISARM_PIN));
}

/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/
