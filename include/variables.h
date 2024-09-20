#ifndef VARIABLES_H
#define VARIABLES_H

#include "pavillon.h"
#include "vs1053b.h"

extern File currentTrack;
extern boolean playingMusic;
extern uint8_t SoundBuffer[vs1053_DATABUFFERLEN];
extern s_id3 id3;
extern int trackNumber;

extern uint8_t volume;
extern int amp_gain;
extern uint16_t volume_pot;
extern uint8_t fileCount;
extern Adafruit_TPA2016 audioamp;
extern char *soundfile;

#endif