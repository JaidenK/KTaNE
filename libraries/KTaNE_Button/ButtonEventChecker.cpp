#include "ButtonEventChecker.h"
#include "ES_Framework.h"
#include "BOARD.h"
#include "Arduino.h"

// This monitors the button for a press/release using 
// a shifting register of the button state to apply some
// debouncing. 
//
// Posts: BUTTON_EVENT
//

uint8_t ButtonCurrentState = 0; // Active low
uint8_t BtnStateRegister = 0; // Shift register

#define POLLING_PERIOD_MS 1

unsigned long t_nextSample = 0;

uint8_t CheckButtons(void)
{
    // Limit sample rate
    if(millis() < t_nextSample)
        return 0;
    // Initialization. 
    if(t_nextSample == 0)
        t_nextSample = millis();
    // Increment sample time
    t_nextSample += POLLING_PERIOD_MS;

    // Perform sample.
    // Shift in current button state.
    BtnStateRegister = (BtnStateRegister << 1) + digitalRead(BTN_PIN);

    uint8_t hasEventOccurred = 0;
    uint16_t eventParam = 0;

    // Check for a change
    if(ButtonCurrentState)
    {
        // We think the button is currently released.
        // Check for a falling edge on the register
        if(BtnStateRegister == 0xF0)
        {
            // Shift register is all low. The button has
            // been pressed.
            ButtonCurrentState = 0;
            eventParam |= (1 << 8);
            hasEventOccurred = 1;
        }
    }
    else
    {
        // Button is currently pressed.
        // Check for a rising edge on the register
        if(BtnStateRegister == 0x0F)
        {
            // Button was previously released.
            ButtonCurrentState = 1;
            eventParam |= (1 << 8);
            hasEventOccurred = 1;
        }
    }

    if(hasEventOccurred)
    {
        // Event param has "which button" in bit    0b00010000
        // and the pin's current logic level in bit 0b00000001
        if(ButtonCurrentState)
            eventParam |= (1 << 0);
        ES_PostAll((ES_Event){BUTTON_EVENT,eventParam});
    }

    return (hasEventOccurred);
}