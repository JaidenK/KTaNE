#include "Consequences.h"
#include <Arduino.h>
#include "BOARD.h"

void Consequences_Arm(void)
{
    // If we were generating a heartbeat signal, we'd start generating that here
    Serial.println(F("ARM"));
}

void Consequences_Safe(void)
{
    // If there was a way to safely turn off the heartbeat, it would be done here    
    digitalWrite(CONSEQUENCE_PIN,LOW);   
    Serial.println(F("SAFE"));
}

void Consequences_Fire(void)
{
    digitalWrite(CONSEQUENCE_PIN,HIGH);  
    Serial.println(F("FIRE"));
}