#include <Arduino.h>
#include "Utilities.h"

// Just a utility function
void print2digithex(uint8_t value)
{
    Serial.print(F("x"));
    if (value < 16)
        Serial.print(F("0"));
    Serial.print(value, HEX);
}