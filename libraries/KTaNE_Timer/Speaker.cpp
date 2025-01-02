#include <Arduino.h>
#include "BOARD_Timer.h"
#include "Speaker.h"


void Speaker_BeepBlocking(void)
{
    Serial.println(NOTE_C7);
    tone(SPEAKER_PIN, NOTE_C7, 150);
    delay(300);
    Serial.println(NOTE_AS6);
    tone(SPEAKER_PIN, NOTE_AS6, 50);
    delay(700);
}

void Speaker_Tone(uint32_t frequency, uint64_t duration)
{
    tone(SPEAKER_PIN, frequency, duration);
}