/****************************************************************************
 Module
         ES_Timers.h

 Revision
         1.0.1

 Description
         Header File for the ME218 Timer Module

 Notes


 History
 When           Who	What/Why
 -------------- ---	--------
 01/15/12 16:43 jec  converted for Gen2 of the Events & Services Framework
 10/21/11 18:33 jec  Begin conversion for use with the new Event Framework
 09/01/05 12:29 jec  Converted rates and return values to enumerated constants
 06/15/04 09:58 jec  converted all prefixes to EF_Timer
 05/27/04 15:04 jec  revised TMR_RATE macros for the S12
 11/24/00 13:17 jec  revised TMR_RATE macros for the HC12
 02/20/99 14:11 jec  added #defines for TMR_ERR, TMR_ACTIVE, TMR_EXPIRED
 02/24/97 14:28 jec  Began Coding
****************************************************************************/

//#include <inttypes.h>
#include "Arduino.h"
#include "ES_Timers.h"
#include "ES_Framework.h"


static uint8_t is0TimerRunning = 0;
static uint32_t timer0ExpirationTime = 0;
static uint8_t is1TimerRunning = 0;
static uint32_t timer1ExpirationTime = 0;


uint8_t checkPseudoTimers(void)
{
  uint8_t returnVal = 0;

  if(is0TimerRunning)
  {
    if(millis() >= timer0ExpirationTime)
    {
      is0TimerRunning = 0;
      ES_PostAll((ES_Event){ES_TIMEOUT,0});
      returnVal = 1;
    }
  }

  if(is1TimerRunning)
  {
    if(millis() >= timer1ExpirationTime)
    {
      is1TimerRunning = 0;
      ES_PostAll((ES_Event){ES_TIMEOUT,1});
      returnVal = 1;
    }
  }

  return (returnVal);
}

void StartPseudoTimer(uint8_t whichTimer, uint16_t duration)
{
  if(whichTimer == 0)
  {
    timer0ExpirationTime = millis() + duration;
    is0TimerRunning = 1;
  }
  else
  {
    timer1ExpirationTime = millis() + duration;
    is1TimerRunning = 1;
  }
}

void StopAllPseudoTimers()
{
  is0TimerRunning = 0;
  is1TimerRunning = 0;
}

void StopPseudoTimer(uint8_t whichTimer)
{
  if(whichTimer == 0)
  {
    is0TimerRunning = 0; 
  }
  else
  {
    is1TimerRunning = 0; 
  }
}

/*------------------------------ End of file ------------------------------*/


