/**
 * @file TimerFSM.cpp
 * @author Jaiden King
 * @date 1/4/2025
 * 
 * Game Behavior Finite State Machine
 * 
 * Device boots up and looks for a PC connection. If it finds a connection, 
 * then it waits Idle until the player starts the game. It's presumed that the
 * player will be using the PC to send packets to the other modules during this
 * time. 
 * If there's no PC connection found at startup, the game will automatically 
 * start. The clock will start counting down until all modules are solved, the
 * time runs out, or 3 strikes occur. 
 * Before starting the game, the game configuration info will be sent to all
 * connected modules.
 */


/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

#include <Arduino.h>

#include "BOARD.h" // TODO Should the Timer FSM know about the board at all?
#include "Timer_EEPROM.h"

#include "ES_Configure.h"
#include "ES_Framework.h"

#include "TimerFSM.h"
#include "KTaNE.h"
#include "Timer_UART.h"
#include "Timer_I2C.h"
#include "Speaker.h"
#include "Clock.h"
#include "Utilities.h"
#include "Consequences.h"

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/

// *Idea: Nuget package to talk to my device?

#define STRIKE_LIMIT 3

typedef enum 
{
    InitPState,
    Booting,
    Idle,
    Setup,
    Running,
    Exploding,
    Solved,
    N_STATES,
}
FSMState_t;

static const char *StateNames[] = 
{
    "InitPState",
    "Booting",
    "Idle",
    "Setup",
    "Running",
    "Exploding",
    "Solved",
    "N_STATES",
};

FSMState_t CurrentState = InitPState;
static uint8_t MyPriority; // For ES Framework. Not important.

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
// State functions. 
// These functions exist only to clean up RunFSM function and do not get called
// elsewhere, therefore they're all inline.
inline void RunState_InitialPseudoState(ES_Event *ThisEvent, FSMState_t *nextState, uint8_t *makeTransition) __attribute__((always_inline));
inline void RunState_Booting(ES_Event *ThisEvent, FSMState_t *nextState, uint8_t *makeTransition) __attribute__((always_inline));
inline void RunState_Idle(ES_Event *ThisEvent, FSMState_t *nextState, uint8_t *makeTransition) __attribute__((always_inline));
inline void RunState_Setup(ES_Event *ThisEvent, FSMState_t *nextState, uint8_t *makeTransition) __attribute__((always_inline));
inline void RunState_Running(ES_Event *ThisEvent, FSMState_t *nextState, uint8_t *makeTransition) __attribute__((always_inline));
inline void RunState_Exploding(ES_Event *ThisEvent, FSMState_t *nextState, uint8_t *makeTransition) __attribute__((always_inline));
inline void RunState_Solved(ES_Event *ThisEvent, FSMState_t *nextState, uint8_t *makeTransition) __attribute__((always_inline));

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                    *
 ******************************************************************************/
static uint8_t nStrikes = 0;

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/



// Minimize actions performed in the initializer because we don't know 
// what other initialization has/hasn't taken place yet
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
    FSMState_t nextState = InitPState;

    if(ThisEvent.EventType == ES_ENTRY)
    {
        Serial.print(F("Entering state: "));
        Serial.println(StateNames[CurrentState]);
    }

    // TODO Replace switch statement with function table
    switch (CurrentState) {
    case InitPState: // If current state is initial Psedudo State
        RunState_InitialPseudoState(&ThisEvent,&nextState,&makeTransition);
        break;
    case Booting:
        RunState_Booting(&ThisEvent,&nextState,&makeTransition);
        break;
    case Idle:
        RunState_Idle(&ThisEvent,&nextState,&makeTransition);        
        break;
    case Setup:     
        RunState_Setup(&ThisEvent,&nextState,&makeTransition);              
        break;
    case Running:
        RunState_Running(&ThisEvent,&nextState,&makeTransition);               
        break;
    case Exploding:
        RunState_Exploding(&ThisEvent,&nextState,&makeTransition);   
        break;
    case Solved:
        RunState_Solved(&ThisEvent,&nextState,&makeTransition);   
        break;
    default: // all unhandled states fall into here
        break;
    } // end switch on Current State

    if((ThisEvent.EventType != ES_NO_EVENT)
        && (ThisEvent.EventType != ES_ENTRY)
        && (ThisEvent.EventType != ES_EXIT))
    {
        // TODO Silence Entry/Exit events
        Serial.print(F("Unhandled event: "));
        print2digithex(ThisEvent.EventType);
        Serial.print(F("("));
        Serial.print(EventNames[ThisEvent.EventType]);
        Serial.print(F("):"));        
        Serial.print(ThisEvent.EventParam);
        Serial.println();
    }

    if (makeTransition == TRUE) { // making a state transition, send EXIT and ENTRY
        // recursively call the current state with an exit event
        RunTimerFSM(EXIT_EVENT);
        CurrentState = nextState;
        RunTimerFSM(ENTRY_EVENT);
    }
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
    Consequences_Fire();
}

void ClearStrikeLEDs()
{
    digitalWrite(STRIKE1_PIN,LOW);
    digitalWrite(STRIKE2_PIN,LOW);
}

// State functions

/**
 * @brief Initializes the state machine and puts us into the Booting state.
 */
void RunState_InitialPseudoState(ES_Event *ThisEvent, FSMState_t *nextState, uint8_t *makeTransition)
{
    if (ThisEvent->EventType == ES_ENTRY
     || ThisEvent->EventType == ES_INIT)
    {            
        TimerEEPROM_Initialize();
        TimerUART_SendCommandByte(i2c_address,RESET);
        
        // TODO If we make a speaker Service then this can definitely go away
        Speaker_Tone(NOTE_C7, 100);

        // now put the machine into the actual initial state
        *nextState = Booting;
        *makeTransition = TRUE;
        ThisEvent->EventType = ES_NO_EVENT;
    }
}

/**
 * @brief Waits for the PC to connect. Otherwise, starts the game.
 */
void RunState_Booting(ES_Event *ThisEvent, FSMState_t *nextState, uint8_t *makeTransition)
{    
    switch (ThisEvent->EventType)
    {
    case ES_ENTRY:
        Consequences_Safe();
        StopAllPseudoTimers();
        StartPseudoTimer(0, 5000); // 5 seconds to allow PC to connect
        Clock_Reset(); // unnecessary call?
        Clock_Spin();
        ThisEvent->EventType = ES_NO_EVENT;
        break;
    case PC_CONNECTION_CHANGED:
        if(ThisEvent->EventParam == 1)
        {
            // Computer connection detected. Go to idle state.
            *nextState = Idle;
            *makeTransition = TRUE;
            ThisEvent->EventType = ES_NO_EVENT;
        }
        break;
    case ES_TIMEOUT:
        // No response from computer. Start game.
        *nextState = Setup;
        *makeTransition = TRUE;
        ThisEvent->EventType = ES_NO_EVENT;
        break;
    case ES_EXIT:
        Clock_Reset(); // Stop the clock spinning
        break;
    default:
        break;
    }
}

/**
 * @brief Waits for the start command.
 */
void RunState_Idle(ES_Event *ThisEvent, FSMState_t *nextState, uint8_t *makeTransition)
{
    switch (ThisEvent->EventType)
    {
    case ES_TIMEOUT:
    case ES_ENTRY:
        TimerUART_EnableStatusTransmissions();
        TimerI2C_EnableBusScanning();
        TimerI2C_EnableStatusMonitoring();
        ClearStrikeLEDs();
        Consequences_Safe();
        Clock_Reset();
        Clock_SetTimeLimit_s(TimerEEPROM_GetTimeLimit_s());
        ThisEvent->EventType = ES_NO_EVENT;
        break;
    case EVENT_START:
        *nextState = Setup;
        *makeTransition = TRUE;
        ThisEvent->EventType = ES_NO_EVENT;
        break;
    // The "game" doesn't care if you're connecting and disconnecting
    // modules right now. If anything needs to happen in response to that,
    // it'll be handled by I2C and Uart layers. This might be helpful for
    // debuging, but should be moved.
    //case MODULE_CONNECTED:
    //    Serial.print(F("Module connected at address: "));
    //    Serial.println(ThisEvent->EventParam);
    //    ThisEvent->EventType = ES_NO_EVENT;
    //    break;
    //case MODULE_DISCONNECTED:
    //    Serial.print(F("Module disconnected at address: "));
    //    Serial.println(ThisEvent->EventParam);
    //    ThisEvent->EventType = ES_NO_EVENT;
    //    break;
    default:
        break;
    }
}

/**
 * @brief Checks with each module to confirm that it's ready to start, then 
 *        starts the game.
 * 
 * If the modules don't all ready up within the timeout we'll go back to the
 * Idle state.
 * 
 * *Idea: We could display an error here
 * "Error: Module not ready at address xx"
 */
void RunState_Setup(ES_Event *ThisEvent, FSMState_t *nextState, uint8_t *makeTransition)
{
    static uint8_t isFirstScan = 0;

    switch (ThisEvent->EventType)
    {
    case ES_ENTRY:
        nStrikes = 0; //* Idea: Strike Counter HAL?
        isFirstScan = 1;
        StopAllPseudoTimers();
        // Safety timeout. It shouldn't take this long to scan the 
        // bus and configure the modules.
        StartPseudoTimer(1, 2000); 
        TimerI2C_StartBusScan();
        TimerI2C_EnableStatusMonitoring();
        TimerUART_EnableStatusTransmissions();
        Clock_SetTimeLimit_s(TimerEEPROM_GetTimeLimit_s());
        ThisEvent->EventType = ES_NO_EVENT;        
        break;
    case I2C_BUS_SCAN_COMPLETE:    
        if(isFirstScan)
        {
            isFirstScan = 0;
            // This is a kind of long/blocking function call. 
            // Consider refactoring it to take advantage of the event checker?
            TimerI2C_BroadcastGameConfigInfo(); 
            TimerI2C_ResetAllModules();
            TimerI2C_StartBusScan();
        }
        else
        {
            // Make sure this doesn't run on the first scan because 
            // then it might look like everything is ready due to missing
            // modules
            if(TimerI2C_AreAllModulesRead())
            {
                *nextState = Running;
                *makeTransition = TRUE;
            }
            else
            {
                Serial.println(F("Modules not ready."));
                // The modules weren't ready. Scan the bus again.
                // (This doesn't actually _cause_ them to ready up, it just
                // is a way to buy some time and get another BUS_SCAN_COMPLETE
                // event posted.) Also, the ready status update isn't 
                // guaranteed to be synced with the bus scan.
                // Important that status monitoring is enabled.
                TimerI2C_StartBusScan();
            }
        }
        ThisEvent->EventType = ES_NO_EVENT;
        break;
    case ES_TIMEOUT:
        if(ThisEvent->EventParam == 1)
        {
            Serial.println(F("Timeout: Modules not ready."));
            *nextState = Idle;
            *makeTransition = TRUE;
            ThisEvent->EventType = ES_NO_EVENT;
        }
        break;
    case ES_EXIT:
        // ? Should we disable bus scanning here?
        // ? We turned it on, so it would make sense to turn it off ?
        // ? Same for status monitoring.
        break;
    default:
        break;
    }
}

/**
 * @brief Waiting the modules to be solved or the Clock to expire.
 * 
 * Also keeps track of strikes.
 * 
 * * Idea: We should have it listening for MODULE_SOLVED events and then
 * *       asking TimerI2C if the modules are all solved. Or listen for just
 * *       the single ALL_MODULES_SOLVED event.
 */
void RunState_Running(ES_Event *ThisEvent, FSMState_t *nextState, uint8_t *makeTransition)
{
    switch (ThisEvent->EventType)
    {
    case ES_ENTRY:
        nStrikes = 0; // Consider a strike counter HAL
        StopAllPseudoTimers();
        TimerI2C_DisableBusScanning();
        TimerI2C_EnableStatusMonitoring();
        TimerI2C_StartAllModules();        
        TimerUART_EnableStatusTransmissions();
        Clock_Reset();
        Clock_Start();
        ThisEvent->EventType = ES_NO_EVENT;
        break;
    case ALL_MODULES_SOLVED: 
        Serial.println(F("All modules solved!"));
        *nextState = Solved;
        *makeTransition = TRUE;
        ThisEvent->EventType = ES_NO_EVENT;
        break;
    case MODULE_STRIKE:        
        Serial.println(F("Strike!"));
        // Consider a Strike Counter HAL
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
            ES_PostAll((ES_Event){EVENT_DETONATE,1}); // param=1: Strikes
            ThisEvent->EventType = ES_NO_EVENT;
        }
        break;
    case CLOCK_EXPIRED:
        ES_PostAll((ES_Event){EVENT_DETONATE,0}); // param=0: Clock
        ThisEvent->EventType = ES_NO_EVENT;
        break;
    case EVENT_DETONATE:
        *nextState = Exploding;
        *makeTransition = TRUE;
        ThisEvent->EventType = ES_NO_EVENT;
        break;
    case ES_EXIT:
        TimerI2C_ResetAllModules(); // ? Should this reset be here?
        ThisEvent->EventType = ES_NO_EVENT;
        break;
    default:
        break;
    }   
}

/**
 * @brief Triggers the consequence then returns to Idle after a short time.
 */
void RunState_Exploding(ES_Event *ThisEvent, FSMState_t *nextState, uint8_t *makeTransition)
{
    switch (ThisEvent->EventType)
    {
    case ES_ENTRY:
        digitalWrite(STRIKE1_PIN,HIGH);
        digitalWrite(STRIKE2_PIN,HIGH);
        Consequences_Fire();
        Clock_Stop(); // TODO Consider a way to stop the clock without it flashing
        ThisEvent->EventType = ES_NO_EVENT;
        StopPseudoTimer(0);
        StopPseudoTimer(1);
        StartPseudoTimer(0, 3000);
        break;        
    case ES_TIMEOUT:
        *nextState = Idle;
        *makeTransition = TRUE;
        ThisEvent->EventType = ES_NO_EVENT;
        break;
    default:
        break;
    }
}

/**
 * @brief Stops the clock and waits.
 * 
 * Transitions to Idle state after a pause if the PC is connected. Otherwise
 * it just sits and waits for the user to cycle power.
 */
void RunState_Solved(ES_Event *ThisEvent, FSMState_t *nextState, uint8_t *makeTransition)
{    
    switch (ThisEvent->EventType)
    {
    case ES_ENTRY:
        Consequences_Safe();
        Clock_Stop();
        Clock_StartFlashing();
        StopAllPseudoTimers();
        StartPseudoTimer(0, 5000);
        ThisEvent->EventType = ES_NO_EVENT;
        break;        
    case ES_TIMEOUT:
        if(TimerUART_IsPCConnected())
        {
            *nextState = Idle;
            *makeTransition = TRUE;
            ThisEvent->EventType = ES_NO_EVENT;
        }
        break;
    case PC_CONNECTION_CHANGED:
        if(ThisEvent->EventParam == 1)
        {
            *nextState = Idle;
            *makeTransition = TRUE;
            ThisEvent->EventType = ES_NO_EVENT;
        }
        break;
    default:
        break;
    }
}
