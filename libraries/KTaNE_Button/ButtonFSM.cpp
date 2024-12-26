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
 
typedef enum 
{
    RELEASE_IMMEDIATELY,
    RELEASE_DIGIT_5,
    RELEASE_DIGIT_4,
    RELEASE_DIGIT_1,
}
ButtonRule_t;

ButtonRule_t btn_rule = 0;

uint8_t strip_red_value = 0;
uint8_t strip_green_value = 0;
uint8_t strip_blue_value = 0;

#define BUTTON_RELEASE_TIME_WINDOW_MS 500
#define STRIKE_LED_PULSE_DURATION_MS 1000
uint32_t tPress = 0;

void SetStripValuesFromColor(uint8_t strip_color)
{
    switch (strip_color)
    {
    case STRIP_COLOR_RED:     strip_red_value = 255; strip_green_value =   0; strip_blue_value =   0; break;
    case STRIP_COLOR_YELLOW:  strip_red_value = 255; strip_green_value = 255; strip_blue_value =   0; break;
    case STRIP_COLOR_GREEN:   strip_red_value =   0; strip_green_value = 255; strip_blue_value =   0; break;
    case STRIP_COLOR_CYAN:    strip_red_value =   0; strip_green_value = 255; strip_blue_value = 255; break;
    case STRIP_COLOR_BLUE:    strip_red_value =   0; strip_green_value =   0; strip_blue_value = 255; break;
    case STRIP_COLOR_MAGENTA: strip_red_value = 255; strip_green_value =   0; strip_blue_value = 255; break;
    default: strip_red_value = 255; strip_green_value = 255; strip_blue_value = 255; break;
    }
}

ButtonRule_t releasingAHeldButton(uint8_t strip_color)
{
    if(strip_color == STRIP_COLOR_BLUE)
        return RELEASE_DIGIT_4;
    if(strip_color == STRIP_COLOR_YELLOW)
        return RELEASE_DIGIT_5;
    return RELEASE_DIGIT_1;
}

ButtonRule_t calculateRule()
{
    uint8_t nBatteries = EEPROM.read(EEPROM_TIMER_N_AA_BATTERIES) + EEPROM.read(EEPROM_TIMER_N_D_BATTERIES);
    uint8_t indicator_CAR = 0;
    uint8_t indicator_FRK = 0;
    char buf[] = {0,0,0};
    for(uint8_t i = 0; i < 16; i++) // No consistent max # of indicators has been set yet TODO 
    {
        uint16_t addr = EEPROM_TIMER_INDICATORS + 4*i;
        char c = EEPROM.read(addr);
        if(c == '+')
        {
            // Lit indicator
            buf[0] = EEPROM.read(addr + 1);
            buf[1] = EEPROM.read(addr + 2);
            buf[2] = EEPROM.read(addr + 3);
            if(buf[0] == 'F' && buf[1] == 'R' && buf[2] == 'K')
                indicator_FRK = 1;
            if(buf[0] == 'C' && buf[1] == 'A' && buf[2] == 'R')
                indicator_CAR = 1;
        }
        else if(c == '-')
        {
            // Unlit indicator
        }
        else
        {
            // Garbage memory. End of indicators.
            break;
        }
    }

    uint8_t label = EEPROM.read(EEPROM_BUTTON_LABEL);
    uint8_t btn_color = EEPROM.read(EEPROM_BUTTON_BTN_COLOR);
    uint8_t strip_color = EEPROM.read(EEPROM_BUTTON_STRIP_COLOR);

    if(strip_color == STRIP_COLOR_RANDOM || strip_color > STRIP_COLOR_WHITE)
    {
        strip_color = random(6) + 1;        
    }
    SetStripValuesFromColor(strip_color);

    ButtonRule_t rule = 0;

    // 1. If the button is blue and the button says "Abort", hold the button and refer to "Releasing a Held Button".
    if(btn_color == BTN_COLOR_BLUE && label == BTN_LABEL_ABORT)
    {
        rule = releasingAHeldButton(strip_color);
    }
    // 2. If there is more than 1 battery on the bomb and the button says "Detonate", press and immediately release the button.
    else if(nBatteries > 1 && label == BTN_LABEL_DETONATE)
    {
        rule = RELEASE_IMMEDIATELY;
    }
    // 3. If the button is white and there is a lit indicator with label CAR, hold the button and refer to "Releasing a Held Button".
    else if(btn_color == BTN_COLOR_WHITE && indicator_CAR)
    {
        rule = releasingAHeldButton(strip_color);
    }
    // 4. If there are more than 2 batteries on the bomb and there is a lit indicator with label FRK, press and immediately release the button.
    else if(nBatteries > 2 && indicator_FRK)
    {
        rule = RELEASE_IMMEDIATELY;
    }
    // 5. If the button is yellow, hold the button and refer to "Releasing a Held Button"
    else if(btn_color == BTN_COLOR_YELLOW)
    {
        rule = releasingAHeldButton(strip_color);
    }
    // 6. If the button is red and the button says "Hold", press and immediately release the button.
    else if(btn_color == BTN_COLOR_RED && label == BTN_LABEL_HOLD)
    {
        rule = RELEASE_IMMEDIATELY;
    }
    // 7. If none of the above apply, hold the button and refer to "Releasing a Held Button".
    else
    {
        rule = releasingAHeldButton(strip_color);
    }
    

    
    Serial.print('Z'); // ZZ for triggering logic analzyer
    Serial.print('Z');
    Serial.write(nBatteries);
    Serial.write(indicator_CAR);
    Serial.write(indicator_FRK);
    Serial.write(label);
    Serial.write(btn_color);
    Serial.write(strip_color);
    Serial.write(rule);
    Serial.print('Z');

    return rule;
}

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

uint8_t nibbleContainsDigit(uint16_t nibbles, uint8_t digit)
{
    // Original code:
    //if(((nibbles >> 12) & 0x0F) == digit) return 1;
    //if(((nibbles >> 8)  & 0x0F) == digit) return 1;
    //if(((nibbles >> 4)  & 0x0F) == digit) return 1;
    //if(((nibbles >> 0)  & 0x0F) == digit) return 1;
    //return 0;

    // ChatGPT re-write:
    for (uint8_t i = 0; i < 4; i++) 
    {
        if (((nibbles >> (i * 4)) & 0x0F) == digit) 
        {
            Serial.print(F("ZZNib"));
            Serial.write(':');
            Serial.write(nibbles >> 8);
            Serial.write(nibbles);
            Serial.write(':');
            Serial.write(digit);
            return 1;
        }
    }
    return 0;
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
            digitalWrite(PIN_RED,LOW);  
            digitalWrite(PIN_GREEN,LOW);  
            digitalWrite(PIN_BLUE,LOW);  
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
        switch(ThisEvent.EventType)
        {
            case ES_ENTRY:
                btn_rule = calculateRule();
                STATUS |= _BV(STS_RUNNING);
                digitalWrite(PIN_RED,LOW);  
                digitalWrite(PIN_GREEN,LOW);  
                digitalWrite(PIN_BLUE,LOW); 
                break;
            case BUTTON_EVENT:  
                if(ThisEvent.EventParam & 1) // On release
                {        
                    if(btn_rule == RELEASE_IMMEDIATELY)
                    {
                        if(millis() - tPress > BUTTON_RELEASE_TIME_WINDOW_MS)
                        {
                            // They were supposed to release immediately, but they didn't. Strike!
                            STATUS |= _BV(STS_STRIKE);
                            StartPseudoTimer(1,STRIKE_LED_PULSE_DURATION_MS);
                            digitalWrite(STRIKE_PIN,HIGH); 
                        }
                        else
                        {
                            Serial.print(F("ZZ2NULL"));
                            nextState = Solved;
                            makeTransition = TRUE;
                        }
                    }
                    else
                    {
                        // Request digits
                        REQUEST = REQ_DIGITS;
                        STATUS |= _BV(STS_REQUEST);
                    }

                    digitalWrite(PIN_RED,LOW);  
                    digitalWrite(PIN_GREEN,LOW);  
                    digitalWrite(PIN_BLUE,LOW); 

                    StopPseudoTimer(0);
                }
                else
                {
                    // On press
                    tPress = millis();
                    StartPseudoTimer(0,BUTTON_RELEASE_TIME_WINDOW_MS);
                } 
                ThisEvent.EventType = ES_NO_EVENT;  
                break;
            case DIGITS_RECEIVED:        
                if(    ((btn_rule == RELEASE_DIGIT_1) && (nibbleContainsDigit(ThisEvent.EventParam,(uint8_t)1)))
                    || ((btn_rule == RELEASE_DIGIT_4) && (nibbleContainsDigit(ThisEvent.EventParam,(uint8_t)4)))
                    || ((btn_rule == RELEASE_DIGIT_5) && (nibbleContainsDigit(ThisEvent.EventParam,(uint8_t)5))))
                {
                    //Serial.write('Z');
                    //Serial.write('Z');
                    //Serial.print(F("Digits: "));
                    //Serial.print((uint16_t)ThisEvent.EventParam);
                    
                    Serial.print(F("ZZDigSolve"));
                    nextState = Solved;
                    makeTransition = TRUE;
                }
                else
                {
                    STATUS |= _BV(STS_STRIKE);
                    StartPseudoTimer(1,STRIKE_LED_PULSE_DURATION_MS);
                    digitalWrite(STRIKE_PIN,HIGH); 
                }
                ThisEvent.EventType = ES_NO_EVENT;
                break;
            case ES_TIMEOUT:
                if(ThisEvent.EventParam == 1)
                {
                    digitalWrite(STRIKE_PIN,LOW); 
                }
                else
                {
                    analogWrite(PIN_RED,strip_red_value);  
                    analogWrite(PIN_GREEN,strip_green_value);  
                    analogWrite(PIN_BLUE,strip_blue_value);  
                }
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
