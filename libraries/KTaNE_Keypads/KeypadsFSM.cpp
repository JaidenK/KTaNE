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

#include "BOARD_Keypads.h"

#include "ES_Configure.h"
#include "ES_Framework.h"

#include "KeypadsFSM.h"
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

//static uint8_t moduleI2Crequest = 0; 

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/
 
uint8_t InitKeypadsFSM(uint8_t Priority)
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

    Serial.println(F("Keypads FSM Initialized"));
}

// Startup flash Red-Green-Red-Green-Blue
void StartupFlash()
{
    digitalWrite(STRIKE_PIN,HIGH);     
    delay(200);
    digitalWrite(STRIKE_PIN,LOW);  

    digitalWrite(DISARM_PIN,HIGH);     
    delay(200);
    digitalWrite(DISARM_PIN,LOW);  

    digitalWrite(LED1_PIN,HIGH);     
    delay(200);
    digitalWrite(LED1_PIN,LOW); 

    digitalWrite(LED2_PIN,HIGH);     
    delay(200);
    digitalWrite(LED2_PIN,LOW);  

    digitalWrite(LED3_PIN,HIGH);     
    delay(200);
    digitalWrite(LED3_PIN,LOW); 

    digitalWrite(LED4_PIN,HIGH);     
    delay(200);
    digitalWrite(LED4_PIN,LOW); 
      
}

uint8_t PostKeypadsFSM(ES_Event ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event RunKeypadsFSM(ES_Event ThisEvent)
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
            // now put the machine into the actual initial state
            nextState = Idle;
            makeTransition = TRUE;
            ThisEvent.EventType = ES_NO_EVENT;   
    
            KTaNE_InitEEPROM("KEYPADS         ",
                             "KEY001          ",
                             "12/26/2024 19:30");   
            
            StartupFlash();
        }
        break;
    case Idle:
        if(ThisEvent.EventType == ES_ENTRY)
        {
            // Turn off LEDs
            digitalWrite(DISARM_PIN, LOW);
            digitalWrite(STRIKE_PIN, LOW);
            digitalWrite(LED1_PIN,LOW);  
            digitalWrite(LED2_PIN,LOW);  
            digitalWrite(LED3_PIN,LOW);  
            digitalWrite(LED4_PIN,LOW);  

            // Reset status
            STATUS |= _BV(STS_READY);
            STATUS &= ~_BV(STS_RUNNING);   
            STATUS &= ~_BV(STS_SOLVED);    
            STATUS &= ~_BV(STS_STRIKE);    
            STATUS &= ~_BV(STS_REQUEST);

            // Clear pending request
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
            // TODO turn on each keypad's LED if its button is pressed
            //digitalWrite(STRIKE_PIN,!(ThisEvent.EventParam & 1));
            //digitalWrite(DISARM_PIN,!(ThisEvent.EventParam & 1));
            digitalWrite(LED1_PIN,ThisEvent.EventParam & 0x01);
            digitalWrite(LED2_PIN,ThisEvent.EventParam & 0x02);
            digitalWrite(LED3_PIN,ThisEvent.EventParam & 0x04);
            digitalWrite(LED4_PIN,ThisEvent.EventParam & 0x08);
        }
        break;
    case Running:
        switch(ThisEvent.EventType)
        {
            case ES_ENTRY:
                // TODO calculate rule
                //btn_rule = calculateRule();
                STATUS |= _BV(STS_RUNNING);
                digitalWrite(DISARM_PIN, LOW);
                digitalWrite(STRIKE_PIN, LOW);
                digitalWrite(LED1_PIN,LOW);  
                digitalWrite(LED2_PIN,LOW);  
                digitalWrite(LED3_PIN,LOW);  
                digitalWrite(LED4_PIN,LOW);  
                break;
            case BUTTON_EVENT:  
                // TODO check if they pressed the correct key
                ThisEvent.EventType = ES_NO_EVENT;  
                break;                        
            case EVENT_RESET:
                ThisEvent.EventType = ES_NO_EVENT;
                nextState = Idle;
                makeTransition = TRUE;
                break;
            case SOLVED_EVENT:
                Serial.print(F("ZZ1NULL"));
                ThisEvent.EventType = ES_NO_EVENT;
                nextState = Solved;
                makeTransition = TRUE;
                break;
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
        RunKeypadsFSM(EXIT_EVENT);
        CurrentState = nextState;
        RunKeypadsFSM(ENTRY_EVENT);
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
