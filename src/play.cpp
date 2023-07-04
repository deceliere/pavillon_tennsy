// https://github.com/TobiasVanDyk/VS1053B-Teensy36-Teensy41-SDCard-Music-Player

//////////////////////////////////////////////////////////////////////////
// Modified for Teensy 3.6 Tobias van Dyk October 2020
//
// Also based on PJRC.com Teensy libraries
/////////////////////////////////////////////////////////////////////////
/***************************************************
	This is a library for the Adafruit VS1053 Codec Breakout

	Designed specifically to work with the Adafruit VS1053 Codec Breakout
	----> https://www.adafruit.com/products/1381

	Adafruit invests time and resources providing this open source code,
	please support Adafruit and open-source hardware by purchasing
	products from Adafruit!

	Written by Limor Fried/Ladyada for Adafruit Industries.
	BSD license, all text above must be included in any redistribution
 ****************************************************/

#include "pavillon.h"

// Connect SCLK, MISO and MOSI to standard hardware SPI pins.
#define SCLK 13 // SPI Clock shared with SD card
#define MISO 12 // Input data from vs1053 or SD card
#define MOSI 11 // Output data to vs1053 or SD card

// These are the pins used for the Adafruit vs1053B breakout module
#define XRST 9	// vs1053 reset (output)
#define XCS 10	// vs1053 chip select (output)
#define XDCS 8	// vs1053 Data select (output)
#define XDREQ 3 // vs1053 Data Ready an Interrupt pin (input)
// #define SDCS  BUILTIN_SDCARD   // Use Teensy built-in card
//  For Teensy 3.5, 3.6, 4.0, 4.1 better to use its built-in SDCard
#define SDCS 4 // Use vs1053 SDCard Card chip select pin

/////////////////////////////////////////////////////////////////////////////////
// vs1053B.h
/////////////////////////////////////////////////////////////////////////////////

#define vs1053_FILEPLAYER_TIMER0_INT 255 // allows useInterrupt to accept pins 0 to 254
#define vs1053_FILEPLAYER_PIN_INT 5		 // Interrupt number from dreqinttable dreq pin # = 3

#define vs1053_SCI_READ 0x03
#define vs1053_SCI_WRITE 0x02

#define vs1053_REG_MODE 0x00
#define vs1053_REG_STATUS 0x01
#define vs1053_REG_BASS 0x02
#define vs1053_REG_CLOCKF 0x03
#define vs1053_REG_DECODETIME 0x04
#define vs1053_REG_AUDATA 0x05
#define vs1053_REG_WRAM 0x06
#define vs1053_REG_WRAMADDR 0x07
#define vs1053_REG_HDAT0 0x08
#define vs1053_REG_HDAT1 0x09
#define vs1053_REG_VOLUME 0x0B

#define vs1053_GPIO_DDR 0xC017
#define vs1053_GPIO_IDATA 0xC018
#define vs1053_GPIO_ODATA 0xC019

#define vs1053_INT_ENABLE 0xC01A

#define vs1053_MODE_SM_DIFF 0x0001
#define vs1053_MODE_SM_LAYER12 0x0002
#define vs1053_MODE_SM_RESET 0x0004
#define vs1053_MODE_SM_CANCEL 0x0008
#define vs1053_MODE_SM_EARSPKLO 0x0010
#define vs1053_MODE_SM_TESTS 0x0020
#define vs1053_MODE_SM_STREAM 0x0040
#define vs1053_MODE_SM_SDINEW 0x0800
#define vs1053_MODE_SM_ADPCM 0x1000
#define vs1053_MODE_SM_LINE1 0x4000
#define vs1053_MODE_SM_CLKRANGE 0x8000

#define vs1053_SCI_AIADDR 0x0A
#define vs1053_SCI_AICTRL0 0x0C
#define vs1053_SCI_AICTRL1 0x0D
#define vs1053_SCI_AICTRL2 0x0E
#define vs1053_SCI_AICTRL3 0x0F

#define vs1053_DATABUFFERLEN 32

File currentTrack;
boolean playingMusic;
uint8_t SoundBuffer[vs1053_DATABUFFERLEN];
s_id3 id3;
int trackNumber = 1;

uint8_t volume = 0;
int amp_gain = 20;
uint16_t volume_pot;
uint8_t fileCount = 0;
Adafruit_TPA2016 audioamp = Adafruit_TPA2016();
char *soundfile;

/* TBC pour le Vu metre*/
union twobyte
{
	uint16_t word;
	uint8_t byte[2];
};

#define SCI_AICTRL3 0x0F	// address du Vu metre
#define SS_VU_ENABLE 0x0200 // address pour activer le Vu metre
#define SCI_STATUS 0x01		// adress du status du Vu metre

////////////////////////////////////////////////////////////////////////////////
// vs1053B.cpp
////////////////////////////////////////////////////////////////////////////////
#define vs1053_CONTROL_SPI_SETTING SPISettings(250000, MSBFIRST, SPI_MODE0) // 2.5 MHz SPI speed Control
#define vs1053_DATA_SPI_SETTING SPISettings(8000000, MSBFIRST, SPI_MODE0)	// 8 MHz SPI speed Data

////////////////////////////////////////////////////////////////
// Configure interrupt for Data XDREQ from vs1053
// XDREQ is low while the receive buffer is full
////////////////////////////////////////////////////////////////
void vs1053Interrupt()
{
	SPI.usingInterrupt(XDREQ);						  // Disable Interrupt during SPI transactions
	attachInterrupt(XDREQ, vs1053FeedBuffer, CHANGE); // Interrupt on Pin XDREQ state change
} // feeder->feedBuffer executed (lines 26, 209)

//////////////////////////////////////////////////////////
// Set the card to be disabled while we get the vs1053 up
//////////////////////////////////////////////////////////
void vs1053DisableCard()
{
	playingMusic = false;
	pinMode(SDCS, OUTPUT);
	digitalWrite(SDCS, HIGH);
}

/////////////////////////////////////////////////////////////////////////
// Play file without interrupts
/////////////////////////////////////////////////////////////////////////
boolean vs1053PlayFullFile(const char *trackname)
{
	Serial.print("Playing ");
	Serial.println(trackname);
	if (!vs1053StartPlayingFile(trackname))
		return false;
	while (playingMusic)
	{
		vs1053FeedBuffer();
	}
	return true;
}

/////////////////////////
void vs1053StopPlaying()
{
	playingMusic = false;
	currentTrack.close();
}

///////////////////////////////////////
void vs1053PausePlaying(boolean pause)
{
	if (pause)
		playingMusic = false;
	else
	{
		playingMusic = true;
		vs1053FeedBuffer();
	}
}

///////////////////////
boolean vs1053Paused()
{
	return (!playingMusic && currentTrack);
}

////////////////////////
boolean vs1053Stopped()
{
	return (!playingMusic && !currentTrack);
}

//////////////////////////////////////////////////////
boolean vs1053StartPlayingFile(const char *trackname)
{
	Serial.println("vs1053StartPlayingFile");
	currentTrack = SD.open(trackname);
	// message_oled(trackname);
	// delay(500);
	if (!currentTrack)
	{
		message_oled("Failed to open file");
		delay(500);
		Serial.print("Failed to open from SD: ");
		Serial.println(trackname);
		return false;
	}
	else
	{
		// message_oled(strcat("opened: ", trackname));
		// delay(500);
		Serial.print("Opened from SD: ");
		Serial.println(trackname);
	}
	parse_id3();
	Serial.println("parse id3");
	Serial.println(trackNumber);
	playingMusic = true;
	while (!vs1053ReadyForData())
	{ // wait for ready for data
		// message_oled("ready for data... waiting");
		// delay(1000);
	}
	// message_oled("data ready");
	Serial.println("data ready");
	// delay(1000);
	while (playingMusic && vs1053ReadyForData())
	{
		Serial.println("feeding buffer");
		vs1053FeedBuffer(); // then send data
		// message_oled("feeding buffer");
	}
	return true;
}

void vs1053FeedBuffer()
{ // debugging this function causes memory overruns
	//  Serial.println("vs1053FeedBuffer");

	static uint8_t running = 0;
	uint8_t sregsave;

	// Do not allow 2 FeedBuffer instances to run concurrently
	// SREG bit 7 = OFF => no interrupts until bit 7 = ON
	//  sregsave = SREG; // Status Register can clear/set interrupt enable bit 7 cli or sei
	noInterrupts(); // Clear interrupt enable bit 7 in SREG => disable interrupts
	//  if (running) { SREG = sregsave; return; } else { running = 1; SREG = sregsave;  }
	// paused or stopped. no SDCard track open, XDREQ=0 receive buffer full
	if ((!playingMusic) || (!currentTrack) || (!vs1053ReadyForData()))
	{
		running = 0;
		interrupts();
		return;
	}
	interrupts();
	//  if (!playingMusic){
	//    Serial.println("Paused or stopped");
	//    running = 0; return;
	//  }
	//  if (!currentTrack){
	//    Serial.println("No track open from SD card");
	//    running = 0; return;
	//  }
	//  if(!vs1053ReadyForData()){
	//    Serial.println("XDREQ=0 receive buffer full");
	//    running = 0; return;
	//  }

	//  Serial.println("Ready to send buffer");

	// Send buffer
	while (vs1053ReadyForData())
	{
		int bytesread = currentTrack.read(SoundBuffer, vs1053_DATABUFFERLEN);
		if (bytesread == 0) // End of File
		{
			playingMusic = false;
			currentTrack.close();
			running = 0;
			return;
		}
		vs1053PlayData(SoundBuffer, bytesread);
	}
	running = 0;
	return;
}

boolean vs1053ReadyForData()
{
	return digitalRead(XDREQ);
}

int8_t vs1053getVUmeter()
{
	if (vs1053SciRead(SCI_STATUS) & SS_VU_ENABLE)
	{
		return 1;
	}
	return 0;
}

int8_t vs1053setVUmeter(int8_t enable)
{
	uint16_t MP3Status = vs1053SciRead(SCI_STATUS);

	if (enable)
	{
		vs1053SciWrite(SCI_STATUS, MP3Status | SS_VU_ENABLE);
	}
	else
	{
		vs1053SciWrite(SCI_STATUS, MP3Status & ~SS_VU_ENABLE);
	}
	return 1; // in future return if not available, if patch not applied.
}

uint16_t vs1053VuLevel()
{
	return (vs1053SciRead(SCI_AICTRL3));
}

uint32_t vs1053getPosition()
{
	return (vs1053SciRead(SCI_DECODE_TIME));
}

void vs1053PlayData(uint8_t *buffer, uint8_t buffsiz)
{
	SPI.beginTransaction(vs1053_DATA_SPI_SETTING);
	digitalWrite(XDCS, LOW);

	for (uint8_t i = 0; i < buffsiz; i++)
	{
		vs1053SpiWrite(buffer[i]); // buffsiz = 32
	}
	digitalWrite(XDCS, HIGH);
	SPI.endTransaction();
}

void vs1053SetVolume(uint8_t left, uint8_t right)
{
	uint16_t v;
	v = left;
	v <<= 8;
	v |= right;

	cli();
	vs1053SciWrite(vs1053_REG_VOLUME, v);
	sei();
}

uint16_t vs1053DecodeTime()
{
	cli();
	uint16_t t = vs1053SciRead(vs1053_REG_DECODETIME);
	sei();
	return t;
}

void vs1053SoftReset()
{
	vs1053SciWrite(vs1053_REG_MODE, vs1053_MODE_SM_SDINEW | vs1053_MODE_SM_RESET);
	delay(100);
}

void vs1053Reset()
{
	if (XRST >= 0)
	{
		digitalWrite(XRST, LOW);
		delay(100);
		digitalWrite(XRST, HIGH);
	}
	digitalWrite(XCS, HIGH);
	digitalWrite(XDCS, HIGH);
	delay(100);
	vs1053SoftReset();
	delay(100);

	vs1053SciWrite(vs1053_REG_CLOCKF, 0x6000);

	vs1053SetVolume(40, 40);
}

uint8_t vs1053Begin()
{
	if (XRST >= 0)
	{
		pinMode(XRST, OUTPUT); // if reset = -1 ignore
		digitalWrite(XRST, LOW);
	}

	pinMode(XCS, OUTPUT);
	digitalWrite(XCS, HIGH);
	pinMode(XDCS, OUTPUT);
	digitalWrite(XDCS, HIGH);
	pinMode(XDREQ, INPUT);

	SPI.begin();
	SPI.setDataMode(SPI_MODE0);			   // TBC pour test 23.7.1
	SPI.setBitOrder(MSBFIRST);			   // TBC pour test 23.7.1
	SPI.setClockDivider(SPI_CLOCK_DIV128); // TBC pour test 23.7.1

	vs1053Reset();
	return (vs1053SciRead(vs1053_REG_STATUS) >> 4) & 0x0F;
}

/////////////////////////////////////
uint16_t vs1053SciRead(uint8_t addr)
{
	uint16_t data;
	SPI.beginTransaction(vs1053_CONTROL_SPI_SETTING);
	digitalWrite(XCS, LOW);
	vs1053SpiWrite(vs1053_SCI_READ);
	vs1053SpiWrite(addr);
	delayMicroseconds(10);
	// while(!digitalRead(XDREQ)); /// TBC
	data = vs1053SpiRead();
	data <<= 8;
	// while(!digitalRead(XDREQ)); /// TBC
	data |= vs1053SpiRead();
	digitalWrite(XCS, HIGH);
	SPI.endTransaction();
	return data;
}

//////////////////////////////////////////////
void vs1053SciWrite(uint8_t addr, uint16_t data)
{
	SPI.beginTransaction(vs1053_CONTROL_SPI_SETTING);
	digitalWrite(XCS, LOW);
	vs1053SpiWrite(vs1053_SCI_WRITE);
	vs1053SpiWrite(addr);
	vs1053SpiWrite(data >> 8);
	vs1053SpiWrite(data & 0xFF);
	digitalWrite(XCS, HIGH);
	SPI.endTransaction();
}

static volatile uint32_t *clkportreg;
static uint8_t clkpin;

////////////////////////
uint8_t vs1053SpiRead()
{
	int8_t x;
	x = 0;
	// clkportreg = portOutputRegister(digitalPinToPort(SCLK)); // TBC
	// clkpin = digitalPinToBitMask(SCLK); // TBC
	// MSB first, clock low when inactive (CPOL 0), data valid on leading edge (CPHA 0)
	// Make sure clock starts low
	x = SPI.transfer(0x00);
	// Make sure clock ends low
	// *clkportreg &= ~clkpin;  // TBC

	return x;
}

///////////////////////////////
void vs1053SpiWrite(uint8_t c)
{ // MSB first, clock low when inactive (CPOL 0), data valid on leading edge (CPHA 0)
	// Make sure clock starts low
	// clkportreg = portOutputRegister(digitalPinToPort(SCLK)); // TBC
	// clkpin = digitalPinToBitMask(SCLK); // TBC
	SPI.transfer(c);
	// *clkportreg &= ~clkpin;   // Make sure clock ends low // TBC
}

///////////////////////////////
void LoadUserCode(void)
{
	int i;
	for (i = 0; i < CODE_SIZE; i++)
	{
		vs1053SciWrite(atab[i], dtab[i]);
	}
}

bool isFnMusic(char *filename)
{
	int8_t len = strlen(filename);
	bool result;
	if (strstr(strlwr(filename + (len - 4)), ".mp3") || strstr(strlwr(filename + (len - 4)), ".aac") || strstr(strlwr(filename + (len - 4)), ".wma") || strstr(strlwr(filename + (len - 4)), ".wav") || strstr(strlwr(filename + (len - 4)), ".fla") || strstr(strlwr(filename + (len - 4)), ".mid"))
	{
		result = true;
	}
	else
	{
		result = false;
	}
	return result;
}

// static SdFile track;

///////////////////////////////
void vs1053getTrackInfo(uint8_t offset, char *info)
{

	const int fileSize = currentTrack.size();
	// Serial.println("IN TRACK INFO:");
	// Serial.print("fileSize: ");
	// Serial.println(fileSize);
	// Serial.print("currentTrack:");
	// Serial.println(currentTrack.name());
	// Serial.print("currentTrack position:");
	// Serial.println(currentTrack.position());
	// disable interupts
	noInterrupts();

	// record current file position
	//  uint32_t currentPos = currentTrack.curPosition();

	// skip to end
	currentTrack.seek(fileSize - 128 + offset);
	// Serial.print("currentTrack position after seek:");
	// Serial.println(currentTrack.position());

	// currentTrack.seekEnd((-128 + offset));

	// read 30 bytes of tag informat at -128 + offset
	currentTrack.read(info, 30);
	info[30] = 0;
	// if (!strcmp(info, "") || !isAlphaNumeric(info[0]))  {
	// 	info = strcpy(info, "no tag id3");
	// 	info[10] = 0;
	// }
	// Serial.print("id3 from get info:");
	// Serial.println(info);
	// infobuffer = strip_nonalpha_inplace(infobuffer);
	// seek back to saved file position
	currentTrack.seek(0); /// TBC

	// renable interupt
	interrupts();
}
/////////////////////////////////////
// End .cpp
/////////////////////////////////////



// variable pour test pour obtenir tous les nom de fichier du repertoire

const char *folderPath = "/"; // Chemin du dossier à la racine
const int MAX_FILES = 100;	  // Nombre maximal de fichiers à stocker
char *fileNames[MAX_FILES];	  // Tableau de pointeurs de caractères pour stocker les noms des fichiers

/// File listing helper
void printDirectory(File dir, int numTabs)
{
	while (true)
	{

		File entry = dir.openNextFile();
		if (!entry)
		{
			// no more files
			// Serial.println("**nomorefiles**");
			break;
		}
		for (uint8_t i = 0; i < numTabs; i++)
		{
			Serial.print('\t');
		}
		Serial.print(entry.name());
		if (entry.isDirectory())
		{
			Serial.println("/");
			printDirectory(entry, numTabs + 1);
		}
		else
		{
			// files have sizes, directories do not
			Serial.print("\t\t");
			Serial.println(entry.size(), DEC);
		}
		entry.close();
	}
}

/// waiting for serial messages
int check_serial()
{
	if (Serial.available())
	{
		char c = Serial.read();
		// if we get an 's' on the serial console, stop!
		if (c == 's')
		{
			vs1053StopPlaying();
		}
		if (c == 'r')
		{
			vs1053StartPlayingFile(soundfile);
		}

		// if we get an 'p' on the serial console, pause/unpause!
		if (c == 'p')
		{
			if (vs1053Stopped())
			{
				vs1053StartPlayingFile(soundfile);
				loop_oled(id3, soundfile);
				return 1;
			}
			if (!vs1053Paused())
			{
				Serial.println("Pause - press p to pause, s to stop");
				vs1053PausePlaying(true);
			}
			else
			{
				Serial.println("Resumed - press p to pause, s to stop");
				vs1053PausePlaying(false);
			}
		}
		if (c == '1')
		{
			amp_gain -= 5;
			// vs1053PausePlaying(true);
			if (amp_gain <= -28)
			{
				amp_gain = -28;
			}
			// noInterrupts();
			audioamp.setGain(amp_gain);
			// interrupts();
			// delay(100);
			// vs1053PausePlaying(false);
			Serial.print("gain = ");
			Serial.println(audioamp.getGain());
		}
		if (c == '2')
		{
			amp_gain += 5;
			// vs1053PausePlaying(true);
			if (amp_gain >= 30)
			{
				amp_gain = 30;
			}
			noInterrupts();
			audioamp.setGain(amp_gain);
			interrupts();
			// delay(100);
			// vs1053PausePlaying(false);
			Serial.print("gain = ");
			Serial.println(audioamp.getGain());
		}
		if (c == '3')
		{
			audioamp.enableChannel(false, false);
			// Serial.print("gain = ");
			Serial.println("Amp mute");
		}
		if (c == '4')
		{
			audioamp.enableChannel(true, true);
			Serial.println("Amp unmute");
			for (int i = 0; i < 5; i++)
			{
				noInterrupts();
				audioamp.setGain(amp_gain);
				interrupts();
				// delay(10);
			}
			Serial.print("gain = ");
			Serial.println(audioamp.getGain());
		}
		if (c == 'q')
			printDirectory(SD.open("/"), 0);
		if (c == '-')
		{
			volume++;
			vs1053SetVolume(volume, volume);
			Serial.print("volume= ");
			Serial.println(volume);
		}
		if (c == '+')
		{
			volume--;
			vs1053SetVolume(volume, volume);
			Serial.print("volume= ");
			Serial.println(volume);
		}
	}
	return 0;
}

void parse_id3()
{
	// char *display;
	
	// display = new char[12];
	vs1053getTrackInfo(TRACK_TITLE, id3.title);
	// Serial.println(id3.title);
	vs1053getTrackInfo(TRACK_ARTIST, id3.artist);
	// Serial.println(id3.artist);
	vs1053getTrackInfo(TRACK_ALBUM, id3.album);
	// Serial.println(id3.album);
	
	itoa(trackNumber + 1, id3.fileCurrent, 10); // + 1 pour que la piste 0 s affiche comme etant la piste 1
	strcpy(id3.trackDisplay, id3.fileCurrent);
	strcat(id3.trackDisplay, " / ");
	strcat(id3.trackDisplay, id3.fileTotal);

	Serial.print("trackDisplay=");
	Serial.println(id3.trackDisplay);

	// strcpy(id3.trackDisplay, display);
	// delete[] display;
}

		// trackNumber--;
				// if (trackNumber < 0)
				// 	trackNumber = fileCount - 1; 
				// vs1053StopPlaying();
				// soundfile = fileNames[trackNumber];
				// vs1053StartPlayingFile(soundfile);

void	playPrevious() {
	// if (!vs1053Stopped)
	vs1053StopPlaying();
	trackNumber--;
	if (trackNumber < 0)
		trackNumber = fileCount - 1; 
	soundfile = fileNames[trackNumber];
	Serial.print("soundfile playNext= ");
	Serial.println(soundfile);
	while(!vs1053StartPlayingFile(soundfile));
}

void	playNext() {
	// if (!vs1053Stopped)
	vs1053StopPlaying();
	trackNumber++;
	if (trackNumber > fileCount - 1)
		trackNumber = 0; 
	soundfile = fileNames[trackNumber];
	Serial.print("soundfile playNext= ");
	Serial.println(soundfile);
	while(!vs1053StartPlayingFile(soundfile));
	// vs1053StartPlayingFile(soundfile);
}

/* push buttons */

// Délai de debounce en millisecondes
const unsigned long debounceDelay = 50;

// Variables pour le debounce
unsigned long previousMillis = 0;
int previousButtonState = HIGH;
int lastPreviousButtonState = HIGH;
unsigned long nextMillis = 0;
int nextButtonState = HIGH;
int lastNextButtonState = HIGH;
int playButtonState = HIGH;
int lastPlayButtonState = HIGH;

// int nextButtonState = HIGH;
int releasedTime;
int pressedTime;
// long startMillis = millis();

void buttonCheck()
{
	// if (!digitalRead(BUTTON_PREV))
	//     Serial.println("coucou prev");
	// if (!digitalRead(BUTTON_PLAY))
	//     Serial.println("coucou play");
	// if (!digitalRead(BUTTON_NEXT))
	//     Serial.println("coucou next");
	// Lecture de l'état des boutons avec debounce

	int readingPrevious = digitalRead(BUTTON_PREV);
	int readingNext = digitalRead(BUTTON_NEXT);

	if (readingPrevious != lastPreviousButtonState)
	{
		previousMillis = millis();
	}
	if ((millis() - previousMillis) >= debounceDelay)
	{
		if (readingPrevious != previousButtonState)
		{
			previousButtonState = readingPrevious;
			if (previousButtonState == HIGH)
			{
				previousMillis = millis();
				playPrevious();
				Serial.println("prevButton pushed");
			}
		}
	}
	lastPreviousButtonState = readingPrevious;
	if (readingNext != lastNextButtonState)
	{
		nextMillis = millis();
	}
	if ((millis() - nextMillis) >= debounceDelay)
	{
		if (readingNext != nextButtonState)
		{
			nextButtonState = readingNext;
			if (nextButtonState == HIGH)
			{
				nextMillis = millis();
				playNext();
				Serial.println("nextButton pushed");
			}
		}
	}
	lastNextButtonState = readingNext;
}

void setup()
{
	Serial.begin(9600);
	while (!Serial); // wait for Arduino Serial Monitor
	pinMode(FET, OUTPUT);
	pinMode(BUTTON_PREV, INPUT_PULLUP);
	pinMode(BUTTON_PLAY, INPUT_PULLUP);
	pinMode(BUTTON_NEXT, INPUT_PULLUP);
	while (!setup_oled());
	delay(500);
	audioamp.begin();
	if (!audioamp.begin())
	{ // initialise the music player
		// Serial.println(F("Couldn't find amp"));
		while (1);
	}
	else
	{
		message_oled("audio amp ok");
		delay(500);
	}
	// Serial.println(F("Amp found"));
	audioamp.setAGCCompression(TPA2016_AGC_OFF);
	audioamp.setReleaseControl(0);
	audioamp.setAttackControl(0);
	audioamp.setHoldControl(0);
	audioamp.setLimitLevelOff();
	audioamp.setGain(amp_gain);
	// if (vs1053vs_init()) { // initialise the music player
	if (!vs1053Begin())
	{ // initialise the music player
		Serial.println(F("Couldn't find vs1053"));
		message_oled("vs1053 not found");
		while (1);
	}
	else
		message_oled("vs1053 found");
	delay(500);
	while (!SD.begin(SDCS))
	{
		message_oled("SD card not found");
	} // initialise the SD card
	message_oled("SD card found");
	delay(500);
	LoadUserCode();		 // patch pour avoir le Vu metre
	vs1053setVUmeter(1); // allumer les infos du Vu metre
	if (vs1053getVUmeter())
	{
		message_oled("Vu meter OK");
		delay(500);
	}
	else
	{
		message_oled("Vu meter OK");
		delay(500);
	}
	fileCount = listFiles();
	itoa(fileCount, id3.fileTotal, 10);
	message_oled(strcat("filecount= ", id3.fileTotal));
	delay(500);
	Serial.print(F("fileCount:"));
	Serial.println(fileCount);
	Serial.println(F("vs1053 found"));
	// Serial.print("sounfile is ");
	// Serial.println(fileNames[0]);

	// Set volume for left, right channels. lower numbers is higher volume
	vs1053SetVolume(volume, volume);
	message_oled("set volume ok");
	delay(500);
	// delay(500);

	// If XDREQ is on an interrupt pin (any Teensy pin) can do background audio playing
	vs1053Interrupt(); // XDREQ int
	// message_oled("set vs1053 Interrupt OK");
	// delay(800);

	// Play one file, don't return until complete - i.e. serial "s" or "p" will not interrupt
	// Serial.println(F("jurg"));
	// vs1053PlayFullFile("jurg.mp3");
	//  Serial.println(F("Playing track 008"));
	//  vs1053PlayFullFile("track008.wav");
	//  Serial.println(F("Playing SDTEST with super long file name.wav"));
	//  vs1053PlayFullFile("/SDTEST with super long file name.wav");

	// Play another file in the background, use interrupt serial "s" or "p" will interrupt
	// Can only do one buffer feed at a time
	// Serial.println(F("Playing track 002 - press p to pause, s to stop"));
	// vs1053StartPlayingFile("track002.mp3");
	//  Serial.println(F("Playing track 010 - press p to pause, s to stop"));
	//  vs1053StartPlayingFile("track010.wav");

	vs1053SciWrite(0x00, SM_SDINEW | SM_LINE1); // new mode + line in
	Serial.print("SCI mode 0x");
	Serial.println(vs1053SciRead(0x00), HEX);
	char str2[10];
	itoa(vs1053SciRead(0x00), str2, 16);
	message_oled(strcat("SCI mode 0x", str2));
	delay(500);

	// Serial.print("SCI mode after change 0x");
	// Serial.println(vs1053SciRead(0x00), HEX);

	// message_oled("start playingfile OK");
	pinMode(22, INPUT);
	volume_pot = analogRead(22);
	// Serial.println(volume_pot);
	// Serial.println(volume);
	soundfile = fileNames[trackNumber];
	// Serial.print("soundfile:");
	// Serial.println(soundfile);
	Serial.print("fileNames[trackNumber]: ");
	Serial.println(fileNames[trackNumber]);
	//  playFilesInLoop("/");
	// id3 = parse_id3(id3);
	vs1053StartPlayingFile(soundfile);
	message_oled("start playingfile OK");
	delay(500);
	Serial.println(F("Playing - press p to pause, s to stop"));
	if (playingMusic)
		Serial.println("Playback started");
	else
		Serial.println("Playback failed");
}

int currentMilliVU = millis();
int previousMilliVU = currentMilliVU;
int vu_level = 0;
int currentMilliPrev = millis();
int previousMilliPrec = currentMilliPrev;

void loop()
{

	currentMilliVU = millis();
	if (currentMilliVU - previousMilliVU >= 50)
	{
		vu_level = vs1053VuLevel();
		// Serial.println(vu_level);
		previousMilliVU = currentMilliVU;
	}
	// for (int i = 0; i < 20; i++)
	// {
	//   vu_level += vs1053VuLevel();
	// }
	// vu_level /= 20;
	// File is playing in the background

	// if (vs1053Stopped()) {
	// 	message_oled("playback stopped");
	// 	analogWrite(FET, 0);
	// 	Serial.println("Terminated");
	//    while (!check_serial()) {
	// 	}
	//   check_serial();
	// }

	if (vs1053Stopped())
	{
		analogWrite(FET, 0);
		Serial.print("SCI mode at stop 0x");
		Serial.println(vs1053SciRead(0x00), HEX);
		Serial.print("SCI Clock");
		Serial.println(vs1053SciRead(SCI_CLOCKF));
		if (trackNumber < fileCount - 1)
			trackNumber++;
		else if (trackNumber == fileCount - 1)
			trackNumber = 0;
		Serial.print("trackNumber= ");
		Serial.println(trackNumber);
		soundfile = fileNames[trackNumber];
		vs1053StartPlayingFile(soundfile);
	}

	// if (vs1053Stopped() && trackNumber < fileCount - 1) {
	// 	soundfile = fileNames[++trackNumber];
	// 	Serial.print("trackNumber");
	// 	Serial.println(trackNumber);
	// 	vs1053StartPlayingFile(soundfile);

	// }
	// else if (vs1053Stopped() && trackNumber == fileCount - 1) {
	// 	trackNumber = 0;
	// 	Serial.print("trackNumber");
	// 	Serial.println(trackNumber);
	// 	soundfile = fileNames[trackNumber];
	// 	vs1053StartPlayingFile(soundfile);
	// }

	check_serial();

	// Serial.println("Loop running");
	// Serial.print("pot= ");
	// Serial.println(analogRead(22));;

	pot_debounce(50);
	// logVol = pow(10, volume_pot / 1023.0) - 1;
	// logVol = (int) map(logVol, 0, 9, 100, 0);
	// read = (int) logVol;
	// Serial.print("logVol mapped=");
	// Serial.println(logVol);
	if (volume_pot >= 0 && volume_pot <= 400)
		volume = (unsigned int)map(volume_pot, 0, 400, 200, 30);
	if (volume_pot > 400)
		volume = (unsigned int)map(volume_pot, 400, 1023, 29, 1);
	vs1053SetVolume(volume, volume);
	// delay(10);
	/*
	if (vs1053Stopped()) {
		Serial.println("lecture terminee");
		vs1053StartPlayingFile(soundfile);
	}
	*/
	// loop_oled_scroll(soundfile); // not working

	// Serial.println(vu_level);
	if (vu_level >= 9000 && vu_level <= 20000)
		vu_level = map(vu_level, 9000, 20000, 0, 80);
	if (vu_level >= 20001 && vu_level <= 23000)
		vu_level = map(vu_level, 20001, 23000, 81, 255);
	if (vu_level <= 0)
		vu_level = 0;

	// Serial.println(vu_level);
	// Serial.println(vs1053getVUmeter());
	loop_oled(id3, soundfile);

	analogWrite(FET, vu_level);
	// delay(1000);
	// analogWrite(FET, 10);
	// delay(1000);
	buttonCheck();
}

int pot_debounce(int threshold)
{
	int read;

	read = analogRead(22);
	if (read > volume_pot + threshold || read < volume_pot - threshold)
	{
		Serial.print("volum_pot= ");
		Serial.println(volume_pot);
		Serial.print("volume ");
		Serial.println(volume);

		// volFloat = (float) volume_pot;
		// logVol = log(volFloat / 1023);

		// Serial.print("volFloat=");
		// Serial.println(volFloat);
		// Serial.print("logVol=");
		// Serial.println(logVol);
		return (volume_pot = read);
	}
	else
		return (volume_pot);

	// delay(10);
}

int compareNames(const void *a, const void *b)
{
	return strcmp(*(const char **)a, *(const char **)b);
}

int listFiles()
{
	File root = SD.open(folderPath);
	int count = 0;
	if (root)
	{
		while (true)
		{
			File entry = root.openNextFile();
			if (!entry)
			{
				// Aucun fichier supplémentaire
				break;
			}
			if (entry.isDirectory())
			{
				// Ignorer les dossiers
				continue;
			}
			if ((strcmp(entry.name(), "") != 0 && strncmp(entry.name(), "._", 2) != 0) && (strstr(entry.name(), ".mp3") != NULL))
			{
				if (count < MAX_FILES)
				{
					fileNames[count] = new char[strlen(entry.name()) + 1];
					strcpy(fileNames[count], entry.name());
					count++;
				}
			}
			entry.close();
		}
		root.close();

		qsort(fileNames, count, sizeof(char *), compareNames);

		// Affichage des noms des fichiers
		for (int i = 0; i < count; i++)
		{
			Serial.println(fileNames[i]);
		}
	}
	else
	{
		Serial.println("Impossible d'ouvrir le dossier.");
	}
	return (count);
}

// void playFilesInLoop(const char *path)
// {
// 	File directory = SD.open(path);

// 	if (!directory)
// 	{
// 		return;
// 	}

// 	if (!directory.isDirectory())
// 	{
// 		directory.close();
// 		return;
// 	}

// 	currentTrack = directory.openNextFile();
// 	Serial.println("current track@854");
// 	Serial.println(currentTrack);

// 	while (currentTrack)
// 	{
// 		if (!currentTrack.isDirectory() && hasExtension(currentTrack.name(), ".mp3"))
// 		{
// 			Serial.print("Lecture du fichier : ");
// 			Serial.println(currentTrack.name());

// 			// Lecture du fichier MP3
// 			if (!vs1053StartPlayingFile(currentTrack.name()))
// 			{
// 				Serial.println("Erreur lors de la lecture du fichier");
// 			}
// 			parse_id3();
// 			//   Attente de la fin de la lecture
// 			while (!vs1053Stopped)
// 			{
// 				loop_oled(id3, soundfile);
// 				// Ajoutez ici d'autres traitements ou fonctionnalités si nécessaire
// 			}

// 			Serial.println("Lecture terminée");
// 		}

// 		currentTrack = directory.openNextFile();
// 	}

// 	directory.close();
// }

bool hasExtension(const char *filename, const char *extension)
{
	const char *ext = strrchr(filename, '.');

	if (ext != nullptr && *filename != '.')
	{
		return strcmp(ext, extension) == 0;
	}

	return false;
}
