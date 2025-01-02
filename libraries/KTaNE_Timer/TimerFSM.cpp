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
#include "Speaker.h"
//#include "pitches.h"

#include "frequencies.h"


/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/


#define STRIKE_LIMIT 3


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
    Booting,
    Idle,
    Setup,
    Running,
    Exploding,
    Solved,
    N_STATES,
} FSMState_t;

static const char *StateNames[] = {
    "InitPState",
    "Booting",
    "Idle",
    "Setup",
    "Running",
    "Exploding",
    "Solved",
    "N_STATES",
};

FSMState_t CurrentState = InitPState; // <- change enum name to match ENUM
static uint8_t MyPriority;

static uint8_t nStrikes = 0;
static uint8_t allModulesDisarmed = 0;

static uint8_t moduleI2Crequest = 0; 

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/



void SendModuleListToPC()
{  

    uint8_t bytes[N_MAX_MODULES + 1] = {0};
    bytes[0] = LIST_MODULES;
    uint8_t nModules = 0;
    for(uint8_t i = 0; i < N_MAX_MODULES; i++)
    {
        if(ModList[i].i2c_address > 0)
        {
            bytes[1+(2*nModules)] = ModList[i].i2c_address;
            bytes[1+(2*nModules)+1] = ModList[i].Status;
            nModules++;
        }
    }

    SendUARTCommand(i2c_address, bytes, 2*nModules+1);
}

uint8_t InitTimerFSM(uint8_t Priority)
{
    MyPriority = Priority;
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
            LoadAllEepromConfigInfo();
            SendUARTCommandByte(i2c_address,RESET);
            display.setBrightness(5);
            display.clear();
            
            Speaker_Tone(NOTE_C7, 100);

            // now put the machine into the actual initial state
            nextState = Booting;
            makeTransition = TRUE;
            ThisEvent.EventType = ES_NO_EVENT;
        }
        if(ThisEvent.EventType == ES_EXIT)
        {
            ThisEvent.EventType = ES_NO_EVENT;
        }
        break;
    case Booting:
        switch (ThisEvent.EventType)
        {
        case ES_ENTRY:
            StopAllPseudoTimers();
            StartPseudoTimer(0, 5000);
            SpinClock();
            ThisEvent.EventType = ES_NO_EVENT;
            break;
        case PC_CONNECTION_CHANGED:
            if(ThisEvent.EventParam == 1)
            {
                // Computer connection detected. Go to idle state.
                nextState = Idle;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
            }
            break;
        case ES_TIMEOUT:
            // No response from computer. Start game.
            nextState = Setup;
            makeTransition = TRUE;
            ThisEvent.EventType = ES_NO_EVENT;
            break;
        case ES_EXIT:
            StopSpinClock();
            ThisEvent.EventType = ES_NO_EVENT;
            break;
        default:
            break;
        }
        break;
    case Idle:
        switch (ThisEvent.EventType)
        {
        case ES_TIMEOUT:
        case ES_ENTRY:
            digitalWrite(STRIKE1_PIN,LOW);
            digitalWrite(STRIKE2_PIN,LOW);
            digitalWrite(CONSEQUENCE_PIN,LOW); 
            StopAllPseudoTimers(); 
            StartPseudoTimer(0, 200);
            ScanForModules(); // Detects new modules
            GetStatusAllModules(); // Detects disconnected modules
            SendModuleListToPC();
            showTime(((uint32_t)getTimeLimist_s()) * 1000);
            ThisEvent.EventType = ES_NO_EVENT;
            break;
        case EVENT_START:
            nextState = Setup;
            makeTransition = TRUE;
            ThisEvent.EventType = ES_NO_EVENT;
            break;
        case MODULE_CONNECTED:
            Serial.print(F("Module connected at address: "));
            Serial.println(ThisEvent.EventParam);
            ThisEvent.EventType = ES_NO_EVENT;
            break;
        case MODULE_DISCONNECTED:
            Serial.print(F("Module disconnected at address: "));
            Serial.println(ThisEvent.EventParam);
            ThisEvent.EventType = ES_NO_EVENT;
            break;
        case ES_EXIT:
            ThisEvent.EventType = ES_NO_EVENT;
            break;
        default:
            break;
        }
        break;
    case Setup:        
        switch (ThisEvent.EventType)
        {
        case ES_ENTRY:
            nStrikes = 0;
            SetTimeLimit(getTimeLimist_s());
            StartPseudoTimer(0, 100);  // Polling rate
            StartPseudoTimer(1, 2000); // Safety timeout
            ThisEvent.EventType = ES_NO_EVENT;
            ReplicateConfigInfo();
            resetAllModules();
            break;
        case ES_TIMEOUT:
            if(ThisEvent.EventParam == 1)
            {
                nextState = Idle;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
            }
            else
            {
                ThisEvent.EventType = ES_NO_EVENT;
                if(checkAllModulesReady())
                {
                    nextState = Running;
                    makeTransition = TRUE;
                }     
                else
                {
                    StartPseudoTimer(0, 100);
                }   
            }
            break;
        case ES_EXIT:
            ThisEvent.EventType = ES_NO_EVENT;
            break;
        default:
            break;
        }
        break;
    case Running:
        switch (ThisEvent.EventType)
        {
        case ES_ENTRY:
            nStrikes = 0;            
            broadcastAllModules(REG_CTRL, _BV(CTRL_START));
            StartPseudoTimer(0, 50); // Polling rate
            StopPseudoTimer(1);
            StartClock();
            ThisEvent.EventType = ES_NO_EVENT;
            break;
        case ES_TIMEOUT:
            if(ThisEvent.EventParam == 1)
            {
                nextState = Idle;
                makeTransition = TRUE;
            }
            else
            {
                StartPseudoTimer(0, 50);
                GetStatusAllModules();
                SendModuleListToPC();
                uint8_t allSolved = 1;
                for(uint8_t i = 0; i < N_MAX_MODULES; i++)
                {
                    if(ModList[i].i2c_address > 0)
                    {
                        if(!(ModList[i].Status & _BV(STS_SOLVED)))
                        {
                            allSolved = 0;
                        }
                        if(ModList[i].Status & _BV(STS_STRIKE))
                        {
                            Serial.println(F("Strike!"));
                            nStrikes++;
                            if(nStrikes == 1)
                            {
                                digitalWrite(STRIKE1_PIN,HIGH);
                            }     
                            else if(nStrikes == 2)
                            {
                                digitalWrite(STRIKE1_PIN,HIGH);
                                digitalWrite(STRIKE2_PIN,HIGH);
                            }                       
                            else if(nStrikes >= STRIKE_LIMIT)
                            {
                                nextState = Exploding;
                                makeTransition = TRUE;
                                ThisEvent.EventType = ES_NO_EVENT;
                            }
                        }
                        if(ModList[i].Status & _BV(STS_REQUEST))
                        {
                            ServiceRequest(ModList[i].i2c_address);
                        }
                        if(!(ModList[i].Status & _BV(STS_RUNNING)))
                        {
                            Serial.println(F("Not running?"));
                        }
                    }
                }
                if(allSolved)
                {
                    Serial.println(F("All modules solved!"));
                    nextState = Idle;
                    makeTransition = TRUE;
                }
            }
            ThisEvent.EventType = ES_NO_EVENT;
            break;
        case CLOCK_EXPIRED:
            nextState = Exploding;
            makeTransition = TRUE;
            ThisEvent.EventType = ES_NO_EVENT;
            break;
        case ES_EXIT:
            StopClock();
            resetAllModules();
            ThisEvent.EventType = ES_NO_EVENT;
            break;
        default:
            break;
        }        
        break;
    case Exploding:
        switch (ThisEvent.EventType)
        {
        case ES_ENTRY:
            digitalWrite(STRIKE1_PIN,HIGH);
            digitalWrite(STRIKE2_PIN,HIGH);
            digitalWrite(CONSEQUENCE_PIN,HIGH);  
            ThisEvent.EventType = ES_NO_EVENT;
            StopPseudoTimer(0);
            StopPseudoTimer(1);
            StartPseudoTimer(0, 3000);
            break;        
        case ES_TIMEOUT:
            nextState = Idle;
            makeTransition = TRUE;
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
    //ES_Tail(); // trace call stack end
    return ThisEvent;
}


/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/

void Module_ToggleStrikeLED()
{
    digitalWrite(STRIKE1_PIN,!digitalRead(STRIKE1_PIN));
}

void Module_ToggleSolveLED()
{
    digitalWrite(STRIKE2_PIN,!digitalRead(STRIKE2_PIN));
}

void Module_Detonate()
{
    digitalWrite(STRIKE1_PIN,HIGH);
    digitalWrite(STRIKE2_PIN,HIGH);
}

void Module_PerformSelfTest()
{
    display.setBrightness(5);
    display.clear();
    digitalWrite(STRIKE1_PIN,LOW);  
    digitalWrite(STRIKE2_PIN,LOW); 
    digitalWrite(CONSEQUENCE_PIN,0);

    for(uint8_t pos = 0; pos < 4; pos++)
    {
        for(uint8_t i = 0; i < 10; i++)
        {
            display.showNumberDec(i, 0, 1, pos);
            delay(50);
        }
    }

    showTime(754000); // 1234
    
    digitalWrite(STRIKE1_PIN,HIGH);   
    digitalWrite(CONSEQUENCE_PIN,HIGH);    
    delay(500);
    digitalWrite(STRIKE1_PIN,LOW);  

    digitalWrite(STRIKE2_PIN,HIGH);     
    delay(500);
    digitalWrite(STRIKE2_PIN,LOW);  
    digitalWrite(CONSEQUENCE_PIN,LOW);  

    Speaker_BeepBlocking();
       

    Serial.println(F("Self test performed."));

    TEST_RESULTS = SELFTEST_SUCCESS;
    STATUS |= _BV(STS_RESULT_READY);
}

