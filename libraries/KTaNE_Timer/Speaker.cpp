#include <Arduino.h>
#include "BOARD.h"
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

void Speaker_BeginBeep(void)
{
    tone(SPEAKER_PIN, NOTE_C7, 150);
    // TODO Create speaker event checker 
    // that will cycle through an array of frequencies
    // so you can trigger it by calling this function
}

void Speaker_PlaySound(SpeakerSounds_t whichSound)
{
    // TODO This should queue up the notes to be played
    switch (whichSound)
    {
    default:
        tone(SPEAKER_PIN, NOTE_AS6, 150);
        break;
    }
}
