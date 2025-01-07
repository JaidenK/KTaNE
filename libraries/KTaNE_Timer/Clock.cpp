#include <Arduino.h>
#include "BOARD.h"
#include "ES_Framework.h"
#include "Clock.h"
#include "Speaker.h"
#include "TM1637Display.h"


/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
void UpdateSpinningDisplay(void);
void UpdateFlashingDisplay(void);
uint8_t UpdateRunningClock(void);

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                    *
 ******************************************************************************/
#define SPIN_UPDATE_RATE 60 // Arbitrary value to make the spin "look nice"

ClockDisplayMode_t ClockDisplayMode = CLOCK_DISPLAY_OFF;
TM1637Display display(TM1637_CLK,TM1637_DIO);

static uint8_t isClockRunning = 0;
static uint8_t isFlashing = 0;
static uint8_t isSpinning = 0;

static uint32_t timeLimit_ms = 0;
static int32_t timeRemaining = 0;
static int32_t previousTimeRemaining = 0;
static uint32_t tStart_ms = 0;
static uint32_t tFinish_ms = 0;
static int32_t timeToShow = 0;
static int32_t displayedTime_ms = 0;

static uint32_t currentElapsedTime_ms = 0;
static uint32_t tNextBeep = 0; // When to start the next beep. Compared to currentElapsedTime
static uint32_t tNextDisplayUpdate = 0; // When to update the display. Compared to currentElapsedTime

static uint8_t spinCounter = 0;
static uint32_t tNextSpinStep = 0;

static uint16_t displayedDigitsAsNibbles = 0;
static uint32_t flashing_tNextToggle = 0;
static uint8_t flashing_isOn = 0;
static uint32_t flashing_timeToFlash = 0;
#define FLASHING_TOGGLE_PERIOD 200
#define DISPLAY_BRIGHTNESS 0xFF

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

/**
 * @brief Updates the displayed digits on the clock.
 * 
 * The event checker is essentially used like a super loop.
 * 
 * Posts:
 * {CLOCK_EXPIRED,0} - When the running clock runs out of time.
 * 
 */
uint8_t Clock_CheckEvents(void)
{
    uint8_t eventOccurred = FALSE;

    if(isClockRunning)
    {
        eventOccurred = UpdateRunningClock();
    }

    if(isFlashing)
    {
        UpdateFlashingDisplay();
    }

    if(isSpinning)
    {
        UpdateSpinningDisplay();
    }

    return eventOccurred;
}

void Clock_SetTimeLimit_s(uint32_t newLimit_s)
{
    timeLimit_ms = newLimit_s * 1000;

    // TODO check if this logic is appropriate
    // I essentially want the display to automatically update
    // when the time limit is changed, but only if the game 
    // isn't running.
    if(!isClockRunning)
    {
        Clock_ShowTime_ms(timeLimit_ms);
    }
}

void Clock_StartFlashing(void)
{
    isFlashing = 1;
    flashing_tNextToggle = millis();
    //flashing_timeToFlash = timeToShow;
}

void Clock_ShowTime_ms(int32_t time_ms)
{
    // Ensure we're in a number-displaying mode
    isSpinning = 0;
    displayedTime_ms = time_ms;

    // Compute numbers to show based on if there's more 
    // or less than 1 minute remaining.
    uint8_t firstNumber = 0;
    uint8_t secondNumber = 0;
    if(time_ms >= 60000)
    {
        firstNumber = time_ms/60000;
        secondNumber = (time_ms / 1000) % 60;
    }
    else
    {
        firstNumber = time_ms/1000;
        secondNumber = (time_ms / 10) % 100;
    }

    // Display the numbers
    display.showNumberDecEx(firstNumber,0b01000000, TRUE, 2, 0);
    display.showNumberDec(secondNumber, TRUE, 2, 2);

    // Encode the digits in the nibbles
    uint8_t nibble1 = firstNumber / 10;
    uint8_t nibble2 = firstNumber - 10 * nibble1;
    uint8_t nibble3 = secondNumber / 10;
    uint8_t nibble4 = secondNumber - 10 * nibble3;
    displayedDigitsAsNibbles = (nibble1 << 12) | (nibble2 << 8) | (nibble3 << 4) | nibble4;
}

void Clock_Spin(void)
{
    isSpinning = 1;    
    isFlashing = 0;    
    tNextSpinStep = millis();
    spinCounter = 0;
}

void Clock_Start(void)
{
    tStart_ms = millis();
    isClockRunning = TRUE;   
    tNextBeep = 0; 
    tNextDisplayUpdate = 0;
    timeRemaining = timeLimit_ms;
    previousTimeRemaining = timeRemaining;

    
    isFlashing = 0; // Logically, should this be here?
    //Clock_ShowTime_ms(timeLimit_ms);
}

void Clock_Stop(void)
{
    isClockRunning = FALSE;
    tFinish_ms = millis();
}

void Clock_Reset(void)
{
    isClockRunning = FALSE;
    isFlashing = 0;
    isSpinning = 0;
    display.setBrightness(DISPLAY_BRIGHTNESS);
    Clock_ShowTime_ms(timeLimit_ms);
}

void Clock_Clear(void)
{
    display.clear();
}

void Clock_SetMode(ClockDisplayMode_t mode)
{
    ClockDisplayMode = mode;

    display.setBrightness(0xFF); // 0: off, 7:full brightness
    flashing_isOn = 1;

    switch (ClockDisplayMode)
    {
    case CLOCK_DISPLAY_SPINNING:
        break;
    case CLOCK_DISPLAY_SOLID:
        break;
    case CLOCK_DISPLAY_FLASHING:
        break;
    case CLOCK_DISPLAY_OFF:
        break;
    default: 
        break;
    }
}

/**
 * Packs the 4 digits displayed into 2 bytes: 1 nibble per digit
 */
uint16_t Clock_GetDisplayedDigits(void)
{
    return displayedDigitsAsNibbles;
}

TM1637Display *Clock_GetDisplay(void)
{
    return &display;
}

/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/

// Used at startup to show we're waiting for something. Causes a single 
// segment of each display to light up and go around in a circle.
void UpdateSpinningDisplay(void)
{    
    if(millis() >= tNextSpinStep)
    {
        tNextSpinStep += SPIN_UPDATE_RATE; 
        uint8_t segments = 1 << spinCounter;
        uint8_t allSegments[] = {segments, segments, segments, segments};
        
        display.setSegments(allSegments);
        
        spinCounter++;            
        if(spinCounter >= 6)
            spinCounter = 0;
    } 
}

void UpdateFlashingDisplay(void)
{
    // Toggle between the display being off and showing a number.
    if(millis() >= flashing_tNextToggle)
    {
        flashing_tNextToggle += FLASHING_TOGGLE_PERIOD;
        flashing_isOn = !flashing_isOn;
        if(flashing_isOn)
        {
            // displayedTime_ms is updated within Clock_ShowTime_ms, so this
            // pattern will result in the display flashing whatever the last 
            // time displayed was.
            Clock_ShowTime_ms(displayedTime_ms); 
        }
        else
        {
            display.clear();
        }
    }
}

// Computes the current elapsed time.
// Updates the 7 seg display.
// Triggers the speaker to do a beep.
// Posts an event when time runs out.
uint8_t UpdateRunningClock(void)
{
    uint8_t eventOccurred = FALSE;

    // Get current time
    currentElapsedTime_ms = millis() - tStart_ms;
    timeRemaining = timeLimit_ms - currentElapsedTime_ms;

    // TODO How can we make the clock speed up when a strike occurs?

    // Check if we've run out of time
    if(timeRemaining <= 0)
    {
        timeToShow = 0;
        Clock_ShowTime_ms(timeToShow);
        Clock_Stop();     
        ES_PostAll((ES_Event){CLOCK_EXPIRED,0});
        // TODO Play detonation tone on speaker?
        eventOccurred = TRUE;
    }
    else
    {
        // We haven't run out of time yet, so update the display every
        // so many milliseconds (sync it with least significant digit)
        if(currentElapsedTime_ms >= tNextDisplayUpdate)
        {
            // It's possible for the value of timeRemaining to be off by one or more, 
            // causing the timer to show strange values (repeating the same 
            // number for two cycles, then catching up again). So instead of
            // displaying actual timeRemaining value, we display the number 
            // that's "supposed" to be shown. Note that this assumes the event
            // checker is actually being called frequently enough that no
            // delay accumulates. 
            timeToShow = timeLimit_ms - tNextDisplayUpdate;
            Clock_ShowTime_ms(timeToShow);

            // Time step increment depends on how many significant figs are shown.
            // This event checker function actually does get called too infrequently
            // to keep up once we are running at under a minute, so this loop ensures
            // we keep up. 
            do
            {
                tNextDisplayUpdate += (timeRemaining <= 60000) ? 10 : 1000;
            }
            while(tNextDisplayUpdate <= currentElapsedTime_ms);
        }

        // TODO Increase beep frequency when time is low
        if(currentElapsedTime_ms >= tNextBeep)
        {
            Speaker_BeginBeep();
            tNextBeep += 1000;            
        }
    }
    previousTimeRemaining = timeRemaining;

    return eventOccurred;
}


// TODO Figure out where to use this
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