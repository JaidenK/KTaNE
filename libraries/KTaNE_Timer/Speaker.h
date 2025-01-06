#ifndef _SPEAKER_H_
#define _SPEAKER_H_

#define NOTE_C7  2093
#define NOTE_AS6 1865

// Plays the beeps to communicate with the user and
// replicate the virtual game experience. Mostly used

/**
 * @brief plays a full 2-tone beep like the video game.
 *  
 * ? Should this method be exposed in the public API?
 * It's mainly for testing, and will probably become 
 * deprecated. 
 */
void Speaker_BeepBlocking(void);

/**
 * @brief Plays a tone on the speaker at a given 
 *        frequency for the specified duration.
 * 
 * Just a Hardware Abstraction Layer for the speaker
 * pin assignment. Non-blocking. If we want to play
 * a beep for the user, use this function.
 * 
 * *Idea: Speaker_Beep1(), Speaker_Beep2() etc.
 * This would allow us to define that we want a feedback
 * beep, but make it easy to change how that's implemented.
 */
void Speaker_Tone(uint32_t frequency, uint64_t duration);

/**
 * @brief Plays a beep like from the video game non-blocking.
 * 
 * This is mainly called by Clock.h
 * 
 * TODO Implement the state machine event checker needed for this
 */
void Speaker_BeginBeep(void);

typedef enum
{
    SOUND_CONNECTION_MADE,
    SOUND_CONNECTION_LOST,
}
SpeakerSounds_t;

void Speaker_PlaySound(SpeakerSounds_t whichSound);

#endif
