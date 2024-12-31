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

#include "BOARD_ComplicatedWires.h"

#include "ES_Configure.h"
#include "ES_Framework.h"

#include "ComplicatedWiresFSM.h"
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
    InitPState, // 0
    Idle,       // 1
    Running,    // 2
    Solved,     // 3
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


uint8_t get_isDigitEven()
{
    uint8_t lastChar = 0;
    uint8_t lastDigit = 0;

    for(uint8_t i = 0; i < SERIAL_NO_MAX_LENGTH; i++)
    {
        lastChar = EEPROM[EEPROM_TIMER_BOMB_SERIAL_NO + i];
        // Check if value is ASCII
        if(lastChar < 48 || lastChar > 90) // '0' and 'Z'
        {
            if(lastChar < 58)
            {
                lastDigit = lastChar - 48;
            }
        }
        else
        {
            break;
        }
    }

    return (lastDigit & 1) == 0;
}
uint8_t get_hasParallelPort()
{
    // Syntax matches format of TheTimer.xaml.cs
    return (EEPROM[EEPROM_TIMER_PORTS] & (1 << 1)) > 0;
}
uint8_t get_has2Batteries()
{
    return (EEPROM[EEPROM_TIMER_N_AA_BATTERIES] + EEPROM[EEPROM_TIMER_N_D_BATTERIES]) >= 2;
}

uint8_t calculateRule()
{
    // Refer to the truth table available here:
    // https://docs.google.com/spreadsheets/d/1A45_IfDyaQUIMZf9_n4Ueu2QJsHTjETd2lycnWZJMxM/edit?usp=sharing

    // Input Declaration
    uint8_t isWireRed = 0;
    uint8_t isWireBlue = 0;
    uint8_t hasStar = 0;
    uint8_t isLEDOn = 0;
    uint8_t isDigitEven = 0;
    uint8_t hasParallelPort = 0;
    uint8_t has2Batteries = 0; 
    // Outputs
    uint8_t cut = 0;    
    uint8_t letter = 'X'; // X is an invalid letter

    // Wire Selector
    uint8_t whichWire = 0;
    uint8_t mask = 1 << whichWire;

    // Input definition
    isWireRed =  (EEPROM[EEPROM_REDWIRES]  & mask) > 0;
    isWireBlue = (EEPROM[EEPROM_BLUEWIRES] & mask) > 0;
    hasStar =    (EEPROM[EEPROM_STARS]     & mask) > 0;
    isLEDOn =    (EEPROM[EEPROM_LEDS]      & mask) > 0;
    isDigitEven = get_isDigitEven();
    hasParallelPort = get_hasParallelPort();
    has2Batteries = get_has2Batteries();

    uint8_t letter_bitfield = (isWireRed  << 3) 
                            | (isWireBlue << 2)
                            | (hasStar    << 1)
                            | (isLEDOn    << 0);
                            

    // See truth table
    switch (letter_bitfield)
    {
    case 0b0011:
    case 0b1001:
    case 0b1010:
    case 0b1011:
        letter = 'B';
        break;
    case 0b0000:
    case 0b0010:
        letter = 'C';
        break;
    case 0b0001:
    case 0b0110:
    case 0b1111:
        letter = 'D';
        break;
    case 0b0101:
    case 0b0111:
    case 0b1110:
        letter = 'P';
        break;
    case 0b0100:
    case 0b1000:
    case 0b1100:
    case 0b1101:
        letter = 'S';
        break;
    default:
        letter = 'Y'; // Set to a different invalid character to know a case was missed. That shouldn't be possible.
        break;
    }

    switch (letter)
    {
    case 'C': cut = 1; break;
    case 'D': cut = 0; break;
    case 'S': cut = isDigitEven; break;
    case 'P': cut = hasParallelPort; break;
    case 'B': cut = has2Batteries; break;
    default:
        letter = 'Z'; // Set to a different invalid character to know a case was missed. That shouldn't be possible.
        break;
    }

    return cut;
}

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/
 
uint8_t InitComplicatedWiresFSM(uint8_t Priority)
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

    digitalWrite(LED5_PIN,HIGH);     
    delay(200);
    digitalWrite(LED5_PIN,LOW); 

    digitalWrite(LED6_PIN,HIGH);     
    delay(200);
    digitalWrite(LED6_PIN,LOW); 
      
}


void ResetAllOutputs()
{
    digitalWrite(DISARM_PIN, LOW);
    digitalWrite(STRIKE_PIN, LOW);
    digitalWrite(LED1_PIN,LOW);  
    digitalWrite(LED2_PIN,LOW);  
    digitalWrite(LED3_PIN,LOW);  
    digitalWrite(LED4_PIN,LOW);  
    digitalWrite(LED5_PIN,LOW);  
    digitalWrite(LED6_PIN,LOW);  
}

uint8_t PostComplicatedWiresFSM(ES_Event ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event RunComplicatedWiresFSM(ES_Event ThisEvent)
{
    uint8_t makeTransition = FALSE; // use to flag transition
    FSMState_t nextState; // <- need to change enum type here

    //if(ThisEvent.EventType == ES_ENTRY)
    //{
    //    Serial.print(F("Entering state: "));
    //    Serial.println(StateNames[CurrentState]);
    //}

    switch (CurrentState) {
    case InitPState: // If current state is initial Psedudo State
        if (ThisEvent.EventType == ES_ENTRY || 
            ThisEvent.EventType == ES_INIT)// only respond to ES_Init
        {
            // now put the machine into the actual initial state
            nextState = Idle;
            makeTransition = TRUE;
            ThisEvent.EventType = ES_NO_EVENT;   
    
            KTaNE_InitEEPROM("COMP. WIRES     ",
                             "CMPWR001        ",
                             "12/30/2024 19:30");               
            StartupFlash();
        }
        break;
    case Idle:
        if(ThisEvent.EventType == ES_ENTRY)
        {
            // Turn off LEDs
            ResetAllOutputs();

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
        //if(ThisEvent.EventType == DIGITAL_INPUT_EVENT)
        //{
        //    digitalWrite(LED1_PIN,ThisEvent.EventParam & 0x01);
        //    digitalWrite(LED2_PIN,ThisEvent.EventParam & 0x02);
        //    digitalWrite(LED3_PIN,ThisEvent.EventParam & 0x04);
        //    digitalWrite(LED4_PIN,ThisEvent.EventParam & 0x08);
        //    digitalWrite(LED5_PIN,ThisEvent.EventParam & 0x10);
        //    digitalWrite(LED6_PIN,ThisEvent.EventParam & 0x20);
        //}
        break;
    case Running:
        switch(ThisEvent.EventType)
        {
            case ES_ENTRY:
                ResetAllOutputs(); 
                calculateRule();
                STATUS |= _BV(STS_RUNNING);
                break;
            case DIGITAL_INPUT_EVENT:  

                // TODO              

                ThisEvent.EventType = ES_NO_EVENT;  
                break;          
            case ES_TIMEOUT:
                digitalWrite(STRIKE_PIN,LOW); 
                ThisEvent.EventType = ES_NO_EVENT;
                break;              
            case EVENT_RESET:
                ThisEvent.EventType = ES_NO_EVENT;
                nextState = Idle;
                makeTransition = TRUE;
                break;
            case SOLVED_EVENT:
                ThisEvent.EventType = ES_NO_EVENT;
                nextState = Solved;
                makeTransition = TRUE;
                break;
            default:
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

    if (makeTransition == TRUE) 
    { 
        // making a state transition, send EXIT and ENTRY
        // recursively call the current state with an exit event
        RunComplicatedWiresFSM(EXIT_EVENT);
        CurrentState = nextState;
        RunComplicatedWiresFSM(ENTRY_EVENT);
    }
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
    StartupFlash();

    TEST_RESULTS = SELFTEST_SUCCESS;
    STATUS |= _BV(STS_RESULT_READY);
}

/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/
