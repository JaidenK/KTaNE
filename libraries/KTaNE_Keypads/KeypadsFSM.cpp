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

#define STRIKE_LED_PULSE_DURATION_MS 1000

FSMState_t CurrentState = InitPState; // <- change enum name to match ENUM
static uint8_t MyPriority;

struct keystruct
{
    uint8_t param_mask;
    uint8_t symbol;
    uint8_t row_index;
    uint8_t original_index;
};

#define N_ROWS 7
#define N_COLS 6

uint8_t whichColumn = 0;
uint8_t currentStep = 0;
struct keystruct keys[4] = {0};

uint8_t symbols[N_COLS][N_ROWS] =
{
    {28, 13, 30, 12, 7, 9, 23},
    {16, 28, 23, 26, 3, 9, 20},
    {1, 8, 26, 5, 15, 30, 3},
    {11, 21, 31, 7, 5, 20, 4},
    {24, 4, 31, 22, 21, 19, 2},
    {11, 16, 27, 14, 24, 18, 6}
};
uint8_t LED_pins[4] = {LED1_PIN, LED2_PIN, LED3_PIN, LED4_PIN};


uint8_t columnHasSymbol(uint8_t col, struct keystruct *key)
{
    for(uint8_t r = 0; r < N_ROWS; r++)
    {
        if(symbols[col][r] == key->symbol)
        {
            key->row_index = r;
            return 1;
        }
    }
    return 0;
}

void bubbleSort(struct keystruct *arr) {
    uint8_t swapped;
    uint8_t tmp;
  
    for (uint8_t i = 0; i < 4 - 1; i++) {
        swapped = 0;
        for (uint8_t j = 0; j < 4 - i - 1; j++) {
            if ((arr[j].row_index) > (arr[j + 1].row_index)) {
                tmp = arr[j].row_index;
                arr[j].row_index = arr[j + 1].row_index;
                arr[j + 1].row_index = tmp;
                tmp = arr[j].symbol;
                arr[j].symbol = arr[j + 1].symbol;
                arr[j + 1].symbol = tmp;
                tmp = arr[j].param_mask;
                arr[j].param_mask = arr[j + 1].param_mask;
                arr[j + 1].param_mask = tmp;
                tmp = arr[j].original_index;
                arr[j].original_index = arr[j + 1].original_index;
                arr[j + 1].original_index = tmp;
                swapped = 1;
            }
        }
      
        // If no two elements were swapped, then break
        if (!swapped)
            break;
    }
}

void calculateRule()
{
    for(uint8_t i = 0; i < 4; i++)
    {
        keys[i].symbol = EEPROM.read(EEPROM_KEY1 + i);
        keys[i].param_mask = 1 << i;
        keys[i].original_index = i;
    }

    for(whichColumn = 0; whichColumn < N_COLS; whichColumn++)
    {
        if(columnHasSymbol(whichColumn,&keys[0])
        && columnHasSymbol(whichColumn,&keys[1])
        && columnHasSymbol(whichColumn,&keys[2])
        && columnHasSymbol(whichColumn,&keys[3]))
        {
            break;
        }
    }

    bubbleSort(keys);

    // Combine the masks so that repeated button presses don't cause problems
    keys[1].param_mask |= keys[0].param_mask;
    keys[2].param_mask |= keys[1].param_mask;
    keys[3].param_mask |= keys[2].param_mask;

    // Check for no valid column found
    if(whichColumn >= N_COLS)
    {
        digitalWrite(STRIKE_PIN,HIGH);
        delay(2500);
        ES_PostAll((ES_Event){EVENT_RESET,0});
    }
}

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

    //Serial.println(F("Keypads FSM Initialized"));
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
    uint8_t pressed_buttons = 0;
    uint8_t target_mask = 0;
    static uint8_t runningMask = 0;

    uint8_t makeTransition = FALSE; // use to flag transition
    FSMState_t nextState; // <- need to change enum type here

    if(ThisEvent.EventType == ES_ENTRY)
    {
        //Serial.print(F("Entering state: "));
        //Serial.println(StateNames[CurrentState]);
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
                digitalWrite(DISARM_PIN, LOW);
                digitalWrite(STRIKE_PIN, LOW);
                digitalWrite(LED1_PIN,LOW);  
                digitalWrite(LED2_PIN,LOW);  
                digitalWrite(LED3_PIN,LOW);  
                digitalWrite(LED4_PIN,LOW);  
                currentStep = 0;
                calculateRule();
                runningMask = 0;
                STATUS |= _BV(STS_RUNNING);
                break;
            case BUTTON_EVENT:  
                // Check if event is a PRESS. Nothing happens on release.
                pressed_buttons = (ThisEvent.EventParam >> 8) & (ThisEvent.EventParam & 0xFF);
                if(pressed_buttons)
                {                    
                    target_mask = keys[currentStep].param_mask;

                    // Check if the correct button was pressed
                    if((runningMask | pressed_buttons) == target_mask)
                    {
                        runningMask |= pressed_buttons;
                        digitalWrite(LED_pins[keys[currentStep].original_index],HIGH);
                        currentStep++;

                        if(currentStep >= 4)
                        {
                            // Solved!
                            nextState = Solved;
                            makeTransition = TRUE;
                        }
                    }
                    // Check if the WRONG button was pressed. Can happen simultaneously.
                    if(pressed_buttons & ~target_mask)
                    {
                        STATUS |= _BV(STS_STRIKE); 
                        digitalWrite(STRIKE_PIN,HIGH);                         
                        digitalWrite(LED1_PIN,LOW);  
                        digitalWrite(LED2_PIN,LOW);  
                        digitalWrite(LED3_PIN,LOW);  
                        digitalWrite(LED4_PIN,LOW);  
                        currentStep = 0;
                        runningMask = 0;
                        StartPseudoTimer(0,STRIKE_LED_PULSE_DURATION_MS);
                    }
                }               

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
    //ES_Tail(); // trace call stack end
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
