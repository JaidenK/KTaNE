#include "KeypadsEventChecker.h"
#include "ES_Framework.h"
#include "BOARD_Keypads.h"
#include "Arduino.h"

// This monitors the button for a press/release using 
// a shifting register of the button state to apply some
// debouncing. 
//
// Posts: BUTTON_EVENT
//

#define N_INPUTS 4

uint8_t ButtonEstimatedState[N_INPUTS] = { 0 }; // Active low
uint8_t BtnStateRegister[N_INPUTS] = { 0 }; // Shift register

#define POLLING_PERIOD_MS 1

unsigned long t_nextSample = 0;

uint8_t CheckDigitalInputs(void)
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
    BtnStateRegister[0] = (BtnStateRegister[0] << 1) + digitalRead(BTN1_PIN);
    BtnStateRegister[1] = (BtnStateRegister[1] << 1) + digitalRead(BTN2_PIN);
    BtnStateRegister[2] = (BtnStateRegister[2] << 1) + digitalRead(BTN3_PIN);
    BtnStateRegister[3] = (BtnStateRegister[3] << 1) + digitalRead(BTN4_PIN);

    uint8_t hasEventOccurred = 0;
    uint16_t eventParam = 0;

    // Check for a change
    for(uint8_t i = 0; i < N_BUTTONS; i++)
    {
        if(ButtonEstimatedState[i])
        {
            // We think the button is currently pressed.
            // Check for a falling edge on the register
            if(BtnStateRegister[i] == 0xF0)
            {
                // Falling edge detected. The button has
                // been released.
                ButtonEstimatedState[i] = 0;
                eventParam |= (1 << (8 + i));
                hasEventOccurred = 1;
            }
        }
        else
        {
            // We think the button is currently released.
            // Check for a rising edge on the register
            if(BtnStateRegister[i] == 0x0F)
            {
                // Rising edge detected. The button has
                // been pressed.
                ButtonEstimatedState[i] = 1;
                eventParam |= (1 << (8 + i));
                hasEventOccurred = 1;
            }
        }
        eventParam |= ((ButtonEstimatedState[i] & 1) << i);
    }

    if(hasEventOccurred)
    {
        // Event param has "which button" in bits    0b0000111100000000
        // and the pin's current logic level in bits 0b0000000000001111
        ES_PostAll((ES_Event){BUTTON_EVENT,eventParam});
    }

    return (hasEventOccurred);
}