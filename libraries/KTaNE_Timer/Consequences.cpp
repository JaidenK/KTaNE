#include "Consequences.h"
#include <Arduino.h>
#include "BOARD_Timer.h"

void Consequences_Arm(void)
{
    // If we were generating a heartbeat signal, we'd start generating that here
}

void Consequences_Safe(void)
{
    // If there was a way to safely turn off the heartbeat, it would be done here    
    digitalWrite(CONSEQUENCE_PIN,LOW);   
}

void Consequences_Fire(void)
{
    digitalWrite(CONSEQUENCE_PIN,HIGH);  
}