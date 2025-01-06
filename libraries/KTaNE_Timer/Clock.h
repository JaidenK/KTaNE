#ifndef _CLOCK_H_
#define _CLOCK_H_

#include <stdint.h>

typedef enum
{
    CLOCK_DISPLAY_SPINNING, // "Loading animation" no numbers shown
    CLOCK_DISPLAY_SOLID,    // Normal display mode
    CLOCK_DISPLAY_FLASHING, // Flash whatever value is being shown
    CLOCK_DISPLAY_OFF       // Display nothing (turn off)
}
ClockDisplayMode_t;

// Please avoid directly manipulating the display object because it will
// cause the rest of the functions to go out of sync
//TM1637Display *Clock_GetDisplay(void); 

/**
 * @brief The main loop function of the Clock controller.
 * 
 * This function is listed in EVENT_CHECK_LIST in ES_Configure.h
 * and will be called repeatedly in sequence with the other event 
 * checkers. It is in control of the TM1637 display used to show
 * the time remaining in the game. Additionally, it can run a
 * limited number of extra functions, such as spinning all the 
 * displays to indicate loading. You can provide it
 * a time limit and command it to begin counting down in a non-
 * blocking way. The main output of this controller is the 
 * CLOCK_EXPIRED event it posts to the state machine, and it 
 * can also provide the digits currently being displayed. 
 * 
 * Posts:
 * {CLOCK_EXPIRED,0} - When the running clock runs out of time.
 * 
 */
uint8_t Clock_CheckEvents(void);

/**
 * @brief Sets the new time limit for the countdown timer.
 * 
 * If the Clock is not running, it will also update the value
 * shown on the display. This ensures no visual glitches 
 * occur while a game is running.
 * 
 * @note This does _not_ update the EEPROM.
 */
void Clock_SetTimeLimit_s(uint32_t newLimit_s);

/**
 * @brief Shows a time (in milliseconds) on the display.
 * 
 * The number will be converted to minutes and seconds
 * with a colon in between them. If the time is 60 seconds
 * or less, it will display as seconds and centiseconds.
 */
void Clock_ShowTime_ms(int32_t time_ms);

/**
 * @brief Starts the countdown timer (non-blocking).
 * 
 * After starting, the clock can be stopped when the
 * bomb is disarmed by calling Clock_Stop(). If no 
 * action is taken then a CLOCK_EXPIRED event will be
 * posted. The speaker will be triggered to beep every
 * second. 
 * 
 * This should be called at the start of the game, not
 * to resume the timer after it's been paused.
 */
void Clock_Start(void);

/**
 * @brief Stops the countdown timer.
 * 
 * This will also cause it to flash the value
 * last shown on the display.
 * 
 * This should be used when the game is over, 
 * either solved or detonated. Not for pausing the timer.
 */
void Clock_Stop(void);

/**
 * @brief Stops the countdown timer and shows the time limit.
 * 
 * Also stops it from flashing if it was.
 */
void Clock_Reset(void);

/**
 * @brief Clears the display until the next time is shown.
 * 
 * TODO This has not been tested.
 */
void Clock_Clear(void);

void Clock_Spin(void);

/**
 * @brief Packs the 4 digits currently displayed into 2 bytes.
 * 
 * Each digit is encoded as a single nibble within the 16 bit return
 * value. Left-to-right as shown on the display is ordered 
 * most significant nibble to least. This is used primarily to
 * answer requests from the Button module.
 * TODO Double check the nibble order
 */
uint16_t Clock_GetDisplayedDigits(void);

//* Idea:
// Flashes the clock between "ERR" and the given parameter. 
// Implement this once we actually have an error worth showing.
//Clock_ShowError(uint8_t error)

void Clock_StartFlashing(void);

#endif /* _CLOCK_H_ */
