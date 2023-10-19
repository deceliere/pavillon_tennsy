/* from vs1053_SdFat.h */
/* Arduino Library for VS10xx shield*/

#ifndef PAVILLON_H
#define PAVILLON_H

#define DEBUG
// #define WAIT_DEVICES
#define NO_VOL_POT /* working without volume pot connected */
// #define SCROLLING_TEST /* working scolling text - cut startup sequence */

#ifdef DEBUG
#define DPRINT(...) Serial.print(__VA_ARGS__)
#define DPRINTLN(...) Serial.println(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINTLN(...)
#endif

#define SCI_MODE 0x00
#define SCI_STATUS 0x01
#define SCI_BASS 0x02
#define SCI_CLOCKF 0x03
#define SCI_DECODE_TIME 0x04
#define SCI_AUDATA 0x05
#define SCI_WRAM 0x06
#define SCI_WRAMADDR 0x07
#define SCI_HDAT0 0x08
#define SCI_HDAT1 0x09
#define SCI_AIADDR 0x0A
#define SCI_VOL 0x0B
#define SCI_AICTRL0 0x0C
#define SCI_AICTRL1 0x0D
#define SCI_AICTRL2 0x0E
#define SCI_AICTRL3 0x0F
//------------------------------------------------------------------------------
#define SM_DIFF 0x0001
#define SM_LAYER12 0x0002
#define SM_RESET 0x0004
#define SM_CANCEL 0x0008
#define SM_EARSPEAKER_LO 0x0010
#define SM_TESTS 0x0020
#define SM_STREAM 0x0040
#define SM_EARSPEAKER_HI 0x0080
#define SM_DACT 0x0100
#define SM_SDIORD 0x0200
#define SM_SDISHARE 0x0400
#define SM_SDINEW 0x0800
#define SM_ADPCM 0x1000
#define SM_PAUSE 0x2000 // note: Only availble with patch. This only quickly pauses the VS's internal buffer, when canceling quickly. It won't unpause.
#define SM_LINE1 0x4000
#define SM_CLK_RANGE 0x8000

// configure Line1 as single ended, otherwise as differential 10x gain for microphones.
#define VS_LINE1_MODE
//------------------------------------------------------------------------------
#define SS_VU_ENABLE 0x0200
//------------------------------------------------------------------------------
#define para_chipID_0 0x1E00
#define para_chipID_1 0x1E01
#define para_version 0x1E02
#define para_config1 0x1E03
#define para_playSpeed 0x1E04
#define para_byteRate 0x1E05
#define para_endFillByte 0x1E06
#define para_MonoOutput 0x1E09
#define para_positionMsec_0 0x1E27
#define para_positionMsec_1 0x1E28
#define para_resync 0x1E29

/* ID3v1 */ // WIP
#define TRACK_TITLE 3
#define TRACK_ARTIST 33
#define TRACK_ALBUM 63

/* PINS */
#define FET 23 // PWM pin > FET
#define BUTTON_PREV 0
#define BUTTON_PLAY 1
#define BUTTON_NEXT 2
#define DEBOUNCE 20

/* OLED */
#define FONT_NORMAL u8g2_font_helvR08_te
#define	OLED_WIDTH 128
#define	OLED_HEIGHT 64
#define UPPER_TITLE // capitilize id3 title
#define WAIT_BEFORE_SCROLL 2000 // millis
#define FIRST_LINE 10
#define SECOND_LINE 12
#define THIRD_LINE 13
#define LINE_FROM_BOTTOM 11 // space from bottom screen

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <cctype>
#include "Adafruit_TPA2016.h"
#include <cmath>
#include <limits.h>
#include <strings.h>
#include <taglib.h>
#include <fileref.h>
#include "vs1053_patch.h"
#include <U8g2lib.h>
// #include <filesystem> 
#include <iostream>

/* test TAGLIB pour id3v2*/
// #include <fileref.h>
// #include <tag.h>
// #include <tpropertymap.h>
// #include <taglib.h>
// #include <mpegfile.h>
// #include <id3v2tag.h>
// #include <tpropertymap.h>
// #include <iostreams.h>
// #include <iostream.h>

#endif

struct s_id3
{
	char title[64];
	char artist[64];
	char album[64];
	char fileTotal[4];
	char fileCurrent[4];
	char trackDisplay[12];
	// char time[12]; // replaced by minSecStr
};

struct scroll_msg 
{
  u8g2_uint_t offset;
  u8g2_uint_t width;
  char str[64];
  u8g2_uint_t x;
  int y; // y pixel to display text
  elapsedMillis timer;
};

enum flush_m
{
	post,
	pre,
	both,
	none
};

/* VS1053 */
void vs1053FeedBuffer();
boolean vs1053StartPlayingFile(const char *trackname);
boolean vs1053ReadyForData();
void vs1053PlayData(uint8_t *buffer, uint8_t buffsiz);
void vs1053SetVolume(uint8_t left, uint8_t right);
uint16_t vs1053DecodeTime();
void vs1053SoftReset();
void vs1053Reset();
uint8_t vs1053Begin();
void vs1053SpiWrite(uint8_t c);
uint8_t vs1053SpiRead();
void vs1053SciWrite(uint8_t addr, uint16_t data);
uint16_t vs1053SciRead(uint8_t addr);
void vs1053Interrupt();
int pot_debounce(int threshold);
uint32_t vs1053getPosition();
void LoadUserCode(void);
void vs1053getTrackInfo(uint8_t offset, char *info);
int listFiles();
void parse_id3(void);
// void 		playFilesInLoop(const char *path);
bool hasExtension(const char *filename, const char *extension);
void dcs_low();
void dcs_high();
uint16_t vs1053Mp3ReadWRAM(uint16_t addressbyte);
void vs1053flush_cancel(flush_m mode);
void vs1053resetPosition(void);
void getScaledVolume(void);

/* custom vs1053 */
void playNext();
void playPrevious();

/* libft */
void ft_bzero(void *s, size_t n);

/* OLED */
bool setup_oled(void);
void loop_oled(s_id3 id3, const char *soundfile);
void message_oled(const char *soundfile);
void loop_oled_scroll(s_id3 id3);
void scroll_parseId3v2(s_id3 id3);
void scroll_loop(s_id3 id3);

/* id3v2 */
s_id3 frameInfo(File track, s_id3 id3);
void parse_id3v2();



/* utils */
bool copyFileToSD(const char* sourceFileName, const char* targetFileName);
char *strToUpper(char *str);
int strIsAlphaNumeric(char *str);



static uint16_t spi_Read_Rate; 
static uint16_t spi_Write_Rate;

