/****************************************************************************
 Module
     ES_Configure.h
 Description
     This file contains macro definitions that are edited by the user to
     adapt the Events and Services framework to a particular application.
 Notes
     
 History
 When           Who     What/Why
 -------------- ---     --------
 01/15/12 10:03 jec      started coding
 *****************************************************************************/

#ifndef CONFIGURE_H
#define CONFIGURE_H

/****************************************************************************/
// Name/define the events of interest
// Universal events occupy the lowest entries, followed by user-defined events

/****************************************************************************/
typedef enum {
    ES_NO_EVENT, ES_ERROR, /* used to indicate an error from the service */
    ES_INIT, /* 2: used to transition from initial pseudo-state */
    ES_ENTRY, /* 3: used to enter a state*/
    ES_EXIT, /* 4: used to exit a state*/
    ES_KEYINPUT, /* used to signify a key has been pressed*/
    ES_LISTEVENTS, /* used to list events in keyboard input, does not get posted to fsm*/
    ES_TIMEOUT, /* 7: signals that the timer has expired */ 
    ES_TIMERACTIVE, /* signals that a timer has become active */
    ES_TIMERSTOPPED, /* signals that a timer has stopped*/
    /* KTaNE Common Events */
    EVENT_START, // 10
    EVENT_RESET, // 11
    /* Module custom events */
    DIGITAL_INPUT_EVENT,
    SOLVED_EVENT,
    STRIKE_EVENT,
    NUMBEROFEVENTS,
} ES_EventTyp_t;

/****************************************************************************/
// This are the name of the Event checking function header file.
#define EVENT_CHECK_HEADER "ES_EventCheckHeader.h"

/****************************************************************************/
// This is the list of event checking functions
#define EVENT_CHECK_LIST CheckDigitalInputs

/****************************************************************************/
// The maximum number of services sets an upper bound on the number of 
// services that the framework will handle. Reasonable values are 8 and 16
// HOWEVER: at this time only a value of 8 is supported.
#define MAX_NUM_SERVICES 8

/****************************************************************************/
// This macro determines that nuber of services that are *actually* used in
// a particular application. It will vary in value from 1 to MAX_NUM_SERVICES
#define NUM_SERVICES 1

/****************************************************************************/
// These are the definitions for Service 0, the lowest priority service
// every Events and Services application must have a Service 0. Further 
// services are added in numeric sequence (1,2,3,...) with increasing 
// priorities
// the header file with the public fuction prototypes
#define SERV_0_HEADER "ComplicatedWiresFSM.h"
// the name of the Init function
#define SERV_0_INIT InitComplicatedWiresFSM
// the name of the run function
#define SERV_0_RUN RunComplicatedWiresFSM
// How big should this service's Queue be?
#define SERV_0_QUEUE_SIZE 9

/****************************************************************************/
// These are the definitions for Service 1
// Copy this pattern if more services are needed.
#if NUM_SERVICES > 1
// the header file with the public fuction prototypes
#define SERV_1_HEADER "TestService.h"
// the name of the Init function
#define SERV_1_INIT TestServiceInit
// the name of the run function
#define SERV_1_RUN TestServiceRun
// How big should this services Queue be?
#define SERV_1_QUEUE_SIZE 3
#endif

/****************************************************************************/
// the name of the posting function that you want executed when a new 
// keystroke is detected.
// The default initialization distributes keystrokes to all state machines
#define POST_KEY_FUNC ES_PostAll

/****************************************************************************/
// These are the definitions for the Distribution lists. Each definition
// should be a comma seperated list of post functions to indicate which
// services are on that distribution list.
#define NUM_DIST_LISTS 0
#if NUM_DIST_LISTS > 0 
#define DIST_LIST0 PostTemplateFSM
#endif



#endif /* CONFIGURE_H */