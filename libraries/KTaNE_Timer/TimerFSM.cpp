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
            tone(SPEAKER_PIN,C4,100);
            ClearClock();
            showTime(0);
            //initModules();


            tone(SPEAKER_PIN,G4,90);
            delay(100);

            // Inform modules of configuration settings
            BroadcastAllConfigInfo();

            // Request all modules to "reset"
            // for(uint8_t i = 0; i < nConnectedModules; i++)
            // {
            //     Wire.beginTransmission(ConnectedModules[i].i2c_address);
            //     //Wire.write(REQ_RESET);
            //     Wire.endTransmission();
            // }
            // Serial.println(F("Modules reset"));
            
            tone(SPEAKER_PIN,E4,100);
            delay(100);

            ClearClock();

            // now put the machine into the actual initial state
            nextState = Idle;
            makeTransition = TRUE;
            ThisEvent.EventType = ES_NO_EVENT;
        }
        break;
    case Idle:
        switch (ThisEvent.EventType)
        {
        case ES_TIMEOUT:
        case ES_ENTRY:
            StartPseudoTimer(0, 3000);
            ScanForModules();
            ThisEvent.EventType = ES_NO_EVENT;
            break;
        case I2C_CMD_RECEIVED:
            if(LastCommand.CommandID == SOLVED)
            {
                Serial.print(F("SOLVED signal received from address "));
                print2digithex(LastCommand.SenderAddress);
                Serial.println();
                ThisEvent.EventType = ES_NO_EVENT;
            }
            else
            {
                Serial.print(F("Unhandled I2C command."));
            }
            break;
        default:
            break;
        }
        break;
    default: // all unhandled states fall into here
        break;
    } // end switch on Current State

    // Clear the serial buffer if it's still there
    while(Serial.available())
    {
        Serial.readString();
    }

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


// void initModules()
// {
//   uint8_t areAllModulesInitialized = 0;
  
//   byte error, address; //variable for error and I2C address
//   uint8_t nDevices;

//   Serial.println(F("Initializing modules..."));
  
//   do
//   {
//     areAllModulesInitialized = 1;
//     nDevices = 0;
//     for (address = 0; address < 128; address++ )
//     {
//         //Serial.println(address);
//         // if address == reserved address then continue

//         // The i2c_scanner uses the return value of
//         // the Write.endTransmisstion to see if
//         // a device did acknowledge to the address.
//         Wire.beginTransmission(address);
//         //Serial.println(F("Bar"));
//         Wire.write(i2c_address);
//         Wire.write(REQUEST_SYNC); // Signal to a potential device that we want the sync word.
//         Wire.write(N_SYNC_BYTES); // Indicate we want 16 copies
//         //Serial.println(F("Baz"));
//         error = Wire.endTransmission();
//         //Serial.println(F("Foo"));
  
//         if (error == 0) // 0 = success
//         {
//             // Print the device address
//             Serial.print(F("I2C device found at address 0x"));
//             if (address < 16)
//               Serial.print(F("0"));
//             Serial.print(address, HEX);
//             Serial.println(F("  !"));

//             // Request the module to send the sync word
//             Serial.print(F("Scanning for sync word: "));
//             Wire.requestFrom((uint8_t)address, (uint8_t)(2*N_SYNC_BYTES)); // Request the sync word to be written 16 times.
//             uint8_t isValid = 1;
//             uint8_t nBytesRead = 0;
//             while(Wire.available())
//             {
//               uint8_t c1 = Wire.read();
//               uint8_t c2 = Wire.read();
//               uint8_t r = c1 | c2;
//               if(r != 0xFF)
//               {
//                 isValid = 0;
//               }
              
//               nBytesRead += 2;
//             }
//             Serial.println(F(""));
//             if(!isValid || (nBytesRead != 2*N_SYNC_BYTES))
//             {
//               Serial.println(F("Device Invalid"));
//               areAllModulesInitialized = 0;

//               // Send the command to scramble their addressses
//               Wire.beginTransmission(address);
//               Wire.write(REQ_REASSIGN); // Signal to a potential device that we want the sync word.
//               Wire.endTransmission();
//             }
//             else
//             {
//                 // Device is a valid module!
//                 // Ask for its name
//                 Wire.beginTransmission(address);
//                 Wire.write(REQUEST_NAME);
//                 Wire.endTransmission();
//                 Wire.requestFrom((uint8_t)address, (uint8_t)MODULE_NAME_LENGTH); 

//                 // Clear the existing name
//                 memset(ConnectedModules[nConnectedModules].name,0,sizeof(ConnectedModules[nConnectedModules].name));
                
//                 // Read the name
//                 for(uint8_t i = 0; i < MODULE_NAME_LENGTH && Wire.available(); i++)
//                 {
//                     ConnectedModules[nConnectedModules].name[i] = Wire.read();
//                 }
//                 while(Wire.available()) Wire.read(); // Consume extra characters

//                 ConnectedModules[nConnectedModules].i2c_address = address;
//                 ConnectedModules[nConnectedModules].isDisarmed = 0;
//                 nConnectedModules++;
//             }
//             nDevices++;
//         }
//         else if (error == 4)
//         {
//             Serial.print(F("Unknown error at address 0x"));
//             if (address < 16)
//               Serial.print(F("0"));
//             Serial.println(address, HEX);
//         }
//         else if (error == 3)
//         {
//             Serial.print(F("Error at 0x"));
//             if (address < 16)
//               Serial.print(F("0"));
//             Serial.println(address, HEX);
//             Serial.print(error);
//             Serial.println();
//         }
//     }

//     // If there are no modules then that's not allowed. They must have at least 1 module.
//     if (nConnectedModules == 0)
//     {
//         areAllModulesInitialized = 0;
//         showTime(4*60+4);
//         Serial.println(F("No I2C modules found. Scanning again in 5 seconds...\n"));
//         delay(1000); // wait 5 seconds for the next I2C scan
//         showTime(0);
//     }
//   }
//   while(!areAllModulesInitialized);

//   Serial.print(F("\nInitialized "));
//   Serial.print(nDevices);
//   Serial.println(F(" modules."));
  
//   // Print connected modules names
//   for(int i = 0; i < nConnectedModules; i++)
//   {
//     Wire.beginTransmission(ConnectedModules[i].i2c_address);
//     //Wire.write(REQ_RESET); // Signal to a potential device that we want it to reset
//     Wire.endTransmission();

//     Serial.print(F("0x"));
//     if (ConnectedModules[i].i2c_address < 16)
//         Serial.print(F("0"));
//     Serial.print(ConnectedModules[i].i2c_address, HEX);
//     Serial.print(F(" "));
//     Serial.println(ConnectedModules[i].name);

//   }
//   Serial.print(F("\n\n"));
//   delay(1000);
// }
