#ifndef _SPEAKER_H_
#define _SPEAKER_H_


#define NOTE_C7  2093
#define NOTE_AS6 1865

void Speaker_BeepBlocking(void);
void Speaker_Tone(uint32_t frequency, uint64_t duration);

#endif
