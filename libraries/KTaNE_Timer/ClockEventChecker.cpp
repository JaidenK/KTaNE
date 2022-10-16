/*
 * File:   TemplateEventChecker.c
 * Author: Gabriel Hugh Elkaim
 *
 * Template file to set up typical EventCheckers for the  Events and Services
 * Framework (ES_Framework) on the Uno32 for the CMPE-118/L class. Note that
 * this file will need to be modified to fit your exact needs, and most of the
 * names will have to be changed to match your code.
 *
 * This EventCheckers file will work with both FSM's and HSM's.
 *
 * Remember that EventCheckers should only return TRUE when an event has occured,
 * and that the event is a TRANSITION between two detectable differences. They
 * should also be atomic and run as fast as possible for good results.
 *
 * This file includes a test harness that will run the event detectors listed in the
 * ES_Configure file in the project, and will conditionally compile main if the macro
 * EVENTCHECKER_TEST is defined (either in the project or in the file). This will allow
 * you to check you event detectors in their own project, and then leave them untouched
 * for your project unless you need to alter their post functions.
 *
 * Created on September 27, 2013, 8:37 AM
 */

/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

#include <Arduino.h>

#include "ES_Framework.h"
#include "ClockEventChecker.h"
#include "BOARD_Timer.h"

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/

/*******************************************************************************
 * EVENTCHECKER_TEST SPECIFIC CODE                                                             *
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this EventChecker. They should be functions
   relevant to the behavior of this particular event checker */

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                    *
 ******************************************************************************/
uint32_t timeLimit_ms = 0;
static uint32_t startTime = 0;
static uint32_t finishTime = 0;
static uint32_t currentElapsedTime = 0;
static int32_t previousTimeRemaining = 0;
static uint8_t isClockRunning = FALSE;
int32_t timeRemaining = 0;

static uint8_t whichTone = 0;
static uint32_t nextToneAtElapsedTime = 0;
static uint32_t nextDisplayUpdateAtElapsedTime = 0;

static uint16_t toneDurations[] = {145, 150, 50, 1000-145-190-50};
static uint16_t toneFrequencies[] = {2093, 0, 1865, 0};

TM1637Display display(TM1637_CLK,TM1637_DIO);

/* Any private module level variable that you might need for keeping track of
   events would be placed here. Private variables should be STATIC so that they
   are limited in scope to this module. */
/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

uint8_t CheckClock(void) {
    uint8_t returnVal = FALSE;

    if(isClockRunning)
    {
      currentElapsedTime = millis() - startTime;
      timeRemaining = timeLimit_ms - currentElapsedTime;
      if(timeRemaining <= 0)
      {
        showTime(0);
        StopClock();
        ES_PostAll((ES_Event){CLOCK_EXPIRED,0});
        tone(SPEAKER_PIN,600,1000);
        returnVal = TRUE;
      }
      else
      {
        // Display update
        if(currentElapsedTime >= nextDisplayUpdateAtElapsedTime)
        {
          showTime(timeRemaining);
          if(timeRemaining <= 60000)
          {
            nextDisplayUpdateAtElapsedTime += 10; 
          }
          else
          {
            nextDisplayUpdateAtElapsedTime += 1000;  
          }
        }
        showTime(timeRemaining);
        // Increase beep frequency
        if(timeRemaining <= 60000 && previousTimeRemaining > 60000)
        {
          //Serial.println("60 seconds left.");
          // toneDurations[0] = 125;
          // toneDurations[1] = 125;
          // toneDurations[2] = 250;
          // toneDurations[3] = 250;
        }
        else if(timeRemaining <= 10000 && previousTimeRemaining > 10000)
        {
          //Serial.println("10 seconds left.");
          toneDurations[0] = 145;
          toneDurations[1] = 10;
          toneDurations[2] = 50;
          toneDurations[3] = 250-toneDurations[0]-toneDurations[1]-toneDurations[2]; 
        }

        // Beep
        if(currentElapsedTime >= nextToneAtElapsedTime)
        {
          tone(SPEAKER_PIN,toneFrequencies[whichTone],toneDurations[whichTone]);
          nextToneAtElapsedTime = nextToneAtElapsedTime + toneDurations[whichTone]; 
          whichTone++;
          if(whichTone >= 4)
          {
            whichTone = 0;
            //Serial.println(timeRemaining);
          }
        }
      }
      previousTimeRemaining = timeRemaining;
    }

    // Set 7-seg display
    // beep the buzzer

    return (returnVal);
}
void SetTimeLimit(uint32_t newLimit_s)
{
  timeLimit_ms = newLimit_s * 1000;
  if(!isClockRunning)
  {
    showTime(timeLimit_ms);
  }
}
void StartClock(void)
{
  startTime = millis();
  isClockRunning = TRUE;
  whichTone = 3; // Setting to 3 makes the first beep a 1
  nextToneAtElapsedTime = 0;
  nextDisplayUpdateAtElapsedTime = 0;
  timeRemaining = timeLimit_ms;
  previousTimeRemaining = timeRemaining;

  toneDurations[0] = 145;
  toneDurations[1] = 150;
  toneDurations[2] = 50; 
  toneDurations[3] = 1000-toneDurations[0]-toneDurations[1]-toneDurations[2]; 

  display.setBrightness(0xFF);
  display.clear();
}
void StopClock(void)
{
  isClockRunning = FALSE;
  finishTime = millis();
  noTone(SPEAKER_PIN);
}
void OffsetLimit(uint8_t deltaSeconds)
{
  timeLimit_ms += deltaSeconds;
}
void ClearClock()
{
  display.setBrightness(0xFF);
  display.clear();
}

void clockDisplayTest()
{
  uint8_t i = 0;
  uint8_t data[] = { 0, 0, 0, 0 };
  while(1)
  {
    for(int k = 3; k >= 0; k--)
    {
      for(int j = 0; j < 6; j++)
      {
        data[k] = 1 << j;
        display.setSegments(data);
        delay(50);
      }
      data[k] = 0;
    }
    {
      display.showNumberDecEx(i % 99, 0b01000000, TRUE, 2, 0);
      display.showNumberDec(i % 99, TRUE, 2, 2);
      i++;
    }
    tone(SPEAKER_PIN,800,300);
    delay(1000);
  }
}


void showTime(int32_t t)
{
  if(t > 60000)
  {
    display.showNumberDecEx(t/60000,0b01000000, TRUE, 2, 0);
    display.showNumberDec((t / 1000) % 60, TRUE, 2, 2);
  }
  else
  {
    display.showNumberDecEx(t/1000,0b01000000, TRUE, 2, 0);
    display.showNumberDec((t / 10) % 100, TRUE, 2, 2);
  }
}