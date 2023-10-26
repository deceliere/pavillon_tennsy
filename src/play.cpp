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
#define XCS 10	// vs1053 chip select (output) // aka CS on vs1053
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

#define SCI_AICTRL3 0x0F	// address du Vu metre
#define SS_VU_ENABLE 0x0200 // address pour activer le Vu metre
#define SCI_STATUS 0x01		// address du status du Vu metre

#define INIT_AMP_GAIN 20
#define VOLUME_ROTARY_POT 22

File currentTrack;
boolean playingMusic;
uint8_t SoundBuffer[vs1053_DATABUFFERLEN];
s_id3 id3;
int trackNumber = 0;

uint8_t volume = 0;
int amp_gain = INIT_AMP_GAIN;
uint16_t volume_pot;
uint8_t fileCount = 0;
Adafruit_TPA2016 audioamp = Adafruit_TPA2016();
char *soundfile;
String soundfile_str; // wip TBC

union twobyte
{
	uint16_t word;
	uint8_t byte[2];
};

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
	DPRINT("Playing ");
	DPRINTLN(trackname);
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
	vs1053flush_cancel(none);
	vs1053resetPosition();
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
	DPRINTLN("vs1053StartPlayingFile");
	currentTrack = SD.open(trackname);
	// message_oled(trackname);
	// delay(500);
	if (!currentTrack)
	{
		message_oled("Failed to open file");
		delay(500);
		DPRINT("Failed to open from SD: ");
		DPRINTLN(trackname);
		return false;
	}
	else
	{
		// message_oled(strcat("opened: ", trackname));
		// delay(500);
		DPRINT("Opened from SD: ");
		DPRINTLN(trackname);
	}
	// parse_id3();
	parse_id3v2();
	DPRINTLN("parse id3v2");
	DPRINTLN(trackNumber);
	playingMusic = true;
	while (!vs1053ReadyForData())
	{ // wait for ready for data
		message_oled("waiting for data...");
		// delay(1000);
	}
	// message_oled("data ready");
	DPRINTLN("data ready");
	// delay(1000);
	while (playingMusic && vs1053ReadyForData())
	{
		// DPRINTLN("feeding buffer");
		vs1053FeedBuffer(); // then send data
		message_oled("feeding buffer");
		// message_oled("feeding buffer");
	}
	return true;
}

void vs1053FeedBuffer()
{ // debugging this function causes memory overruns
	//  DPRINTLN("vs1053FeedBuffer");

	static uint8_t running = 0;
	// uint8_t sregsave;

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
	//    DPRINTLN("Paused or stopped");
	//    running = 0; return;
	//  }
	//  if (!currentTrack){
	//    DPRINTLN("No track open from SD card");
	//    running = 0; return;
	//  }
	//  if(!vs1053ReadyForData()){
	//    DPRINTLN("XDREQ=0 receive buffer full");
	//    running = 0; return;
	//  }

	//  DPRINTLN("Ready to send buffer");

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

void dcs_low()
{
	digitalWrite(XDCS, LOW);
}

void dcs_high()
{
	digitalWrite(XDCS, HIGH);
}

void vs1053spiInit()
{
	// Set SPI bus for write
	SPI.setBitOrder(MSBFIRST);
	SPI.setDataMode(SPI_MODE0);
	SPI.setClockDivider(spi_Write_Rate);
}

uint16_t vs1053Mp3ReadWRAM(uint16_t addressbyte)
{

	unsigned short int tmp1, tmp2;

	// Set SPI bus for write
	vs1053spiInit();
	SPI.setClockDivider(spi_Read_Rate);

	vs1053SciWrite(SCI_WRAMADDR, addressbyte);
	tmp1 = vs1053SciRead(SCI_WRAM);

	vs1053SciWrite(SCI_WRAMADDR, addressbyte);
	tmp2 = vs1053SciRead(SCI_WRAM);

	if (tmp1 == tmp2)
		return tmp1;
	vs1053SciWrite(SCI_WRAMADDR, addressbyte);
	tmp2 = vs1053SciRead(SCI_WRAM);

	if (tmp1 == tmp2)
		return tmp1;
	vs1053SciWrite(SCI_WRAMADDR, addressbyte);
	tmp2 = vs1053SciRead(SCI_WRAM);

	if (tmp1 == tmp2)
		return tmp1;
	return tmp1;
}

void vs1053flush_cancel(flush_m mode)
{
	int8_t endFillByte = (int8_t)(vs1053Mp3ReadWRAM(para_endFillByte) & 0xFF);

	if ((mode == post) || (mode == both))
	{

		dcs_low(); // Select Data
		for (int y = 0; y < 2052; y++)
		{
			while (!digitalRead(XDREQ))
				;					   // wait until DREQ is or goes high
			SPI.transfer(endFillByte); // Send SPI byte
		}
		dcs_high(); // Deselect Data
	}

	for (int n = 0; n < 64; n++)
	{
		//    Mp3WriteRegister(SCI_MODE, SM_LINE1 | SM_SDINEW | SM_CANCEL); // old way of SCI_MODE WRITE.
		vs1053SciWrite(SCI_MODE, (vs1053SciRead(SCI_MODE) | SM_CANCEL));
		// Mp3WriteRegister(SCI_MODE, (Mp3ReadRegister(SCI_MODE) | SM_CANCEL));

		dcs_low(); // Select Data
		for (int y = 0; y < 32; y++)
		{
			while (!digitalRead(XDREQ))
				;					   // wait until DREQ is or goes high
			SPI.transfer(endFillByte); // Send SPI byte
		}
		dcs_high(); // Deselect Data

		int cancel = vs1053SciRead(SCI_MODE) & SM_CANCEL;
		if (cancel == 0)
		{
			// Cancel has succeeded.
			if ((mode == pre) || (mode == both))
			{
				dcs_low(); // Select Data
				for (int y = 0; y < 2052; y++)
				{
					while (!digitalRead(XDREQ))
						;					   // wait until DREQ is or goes high
					SPI.transfer(endFillByte); // Send SPI byte
				}
				dcs_high(); // Deselect Data
			}
			return;
		}
	}
	// Cancel has not succeeded.
	// DPRINTLN(F("Warning: VS10XX chip did not cancel, reseting chip!"));
	//  Mp3WriteRegister(SCI_MODE, SM_LINE1 | SM_SDINEW | SM_RESET); // old way of SCI_MODE WRITE.
	vs1053SciWrite(SCI_MODE, (vs1053SciRead(SCI_MODE) | SM_RESET)); // software reset. but vs_init will HW reset anyways.
																	//  vs_init(); // perform hardware reset followed by re-initializing.
																	// vs_init(); // however, vs1053::begin() is member function that does not exist statically.
}

int8_t vs1053setVUmeter(int8_t enable)
{
	uint16_t MP3Status = vs1053SciRead(SCI_STATUS);

	if (enable)
		vs1053SciWrite(SCI_STATUS, MP3Status | SS_VU_ENABLE);
	else
		vs1053SciWrite(SCI_STATUS, MP3Status & ~SS_VU_ENABLE);
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

void vs1053resetPosition()
{
	vs1053SciWrite(SCI_DECODE_TIME, 0);
	delayNanoseconds(10);
	vs1053SciWrite(SCI_DECODE_TIME, 0);
	// return (vs1053SciRead(SCI_DECODE_TIME));
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

#if (F_CPU == 16000000)
	spi_Read_Rate = SPI_CLOCK_DIV4;	 // use safe SPI rate of (16MHz / 4 = 4MHz)
	spi_Write_Rate = SPI_CLOCK_DIV2; // use safe SPI rate of (16MHz / 2 = 8MHz)
#else
	// must be 8000000
	spi_Read_Rate = SPI_CLOCK_DIV2;	 // use safe SPI rate of (8MHz / 2 = 4MHz)
	spi_Write_Rate = SPI_CLOCK_DIV2; // use safe SPI rate of (8MHz / 2 = 4MHz)
#endif

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
	// DPRINTLN("IN TRACK INFO:");
	// DPRINT("fileSize: ");
	// DPRINTLN(fileSize);
	// DPRINT("currentTrack:");
	// DPRINTLN(currentTrack.name());
	// DPRINT("currentTrack position:");
	// DPRINTLN(currentTrack.position());
	// disable interupts
	noInterrupts();

	// record current file position
	//  uint32_t currentPos = currentTrack.curPosition();

	// skip to end
	currentTrack.seek(fileSize - 128 + offset);
	// DPRINT("currentTrack position after seek:");
	// DPRINTLN(currentTrack.position());

	// currentTrack.seekEnd((-128 + offset));

	// read 30 bytes of tag informat at -128 + offset
	currentTrack.read(info, 30);
	info[30] = 0;
	// if (!strcmp(info, "") || !isAlphaNumeric(info[0]))  {
	// 	info = strcpy(info, "no tag id3");
	// 	info[10] = 0;
	// }
	// DPRINT("id3 from get info:");
	// DPRINTLN(info);
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
			// DPRINTLN("**nomorefiles**");
			break;
		}
		for (uint8_t i = 0; i < numTabs; i++)
		{
			DPRINT('\t');
		}
		DPRINT(entry.name());
		if (entry.isDirectory())
		{
			DPRINTLN("/");
			printDirectory(entry, numTabs + 1);
		}
		else
		{
			// files have sizes, directories do not
			DPRINT("\t\t");
			DPRINTLN(entry.size(), DEC);
		}
		entry.close();
	}
}

#ifdef DEBUG
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
		if (c == 'n')
			playNext();

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
				DPRINTLN("Pause - press p to pause, s to stop");
				vs1053PausePlaying(true);
			}
			else
			{
				DPRINTLN("Resumed - press p to pause, s to stop");
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
			DPRINT("gain = ");
			DPRINTLN(audioamp.getGain());
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
			DPRINT("gain = ");
			DPRINTLN(audioamp.getGain());
		}
		if (c == '3')
		{
			audioamp.enableChannel(false, false);
			// DPRINT("gain = ");
			DPRINTLN("Amp mute");
		}
		if (c == '4')
		{
			audioamp.enableChannel(true, true);
			DPRINTLN("Amp unmute");
			for (int i = 0; i < 5; i++)
			{
				noInterrupts();
				audioamp.setGain(amp_gain);
				interrupts();
				// delay(10);
			}
			DPRINT("gain = ");
			DPRINTLN(audioamp.getGain());
		}
		if (c == 'q')
			printDirectory(SD.open("/"), 0);
		if (c == '-')
		{
			volume++;
			vs1053SetVolume(volume, volume);
			DPRINT("volume= ");
			DPRINTLN(volume);
		}
		if (c == '+')
		{
			volume--;
			vs1053SetVolume(volume, volume);
			DPRINT("volume= ");
			DPRINTLN(volume);
		}
	}
	return 0;
}
#endif

void parse_id3()
{
	// char *display;

	// display = new char[12];
	vs1053getTrackInfo(TRACK_TITLE, id3.title);
	DPRINTLN(id3.title);
	vs1053getTrackInfo(TRACK_ARTIST, id3.artist);
	DPRINTLN(id3.artist);
	vs1053getTrackInfo(TRACK_ALBUM, id3.album);
	DPRINTLN(id3.album);

	itoa(trackNumber + 1, id3.fileCurrent, 10); // + 1 pour que la piste 0 s affiche comme etant la piste 1
	strcpy(id3.trackDisplay, id3.fileCurrent);
	strcat(id3.trackDisplay, " / ");
	strcat(id3.trackDisplay, id3.fileTotal);

	DPRINT("trackDisplay=");
	DPRINTLN(id3.trackDisplay);

	// strcpy(id3.trackDisplay, display);
	// delete[] display;
}

void parse_id3v2()
{
	// char *display;

	id3 = frameInfo(currentTrack, id3);
	// display = new char[12];
	// vs1053getTrackInfo(TRACK_TITLE, id3.title);
	// DPRINTLN(id3.title);
	// vs1053getTrackInfo(TRACK_ARTIST, id3.artist);
	// DPRINTLN(id3.artist);
	// vs1053getTrackInfo(TRACK_ALBUM, id3.album);
	// DPRINTLN(id3.album);

	itoa(trackNumber + 1, id3.fileCurrent, 10); // + 1 pour que la piste 0 s affiche comme etant la piste 1
	strcpy(id3.trackDisplay, id3.fileCurrent);
	strcat(id3.trackDisplay, " / ");
	strcat(id3.trackDisplay, id3.fileTotal);
	scroll_parseId3v2(id3); /// temp - for testing purpose
	DPRINT("trackDisplay=");
	DPRINTLN(id3.trackDisplay);

	// strcpy(id3.trackDisplay, display);
	// delete[] display;
}

// trackNumber--;
// if (trackNumber < 0)
// 	trackNumber = fileCount - 1;
// vs1053StopPlaying();
// soundfile = fileNames[trackNumber];
// vs1053StartPlayingFile(soundfile);

void playPrevious()
{
	// if (!vs1053Stopped)
	vs1053StopPlaying();
	trackNumber--;
	if (trackNumber < 0)
		trackNumber = fileCount - 1;
	soundfile = fileNames[trackNumber];
	DPRINT("soundfile playPrevious= ");
	DPRINTLN(soundfile);
	while (!vs1053StartPlayingFile(soundfile))
		;
}

void playNext()
{
	// if (!vs1053Stopped)
	vs1053StopPlaying();
	trackNumber++;
	if (trackNumber > fileCount - 1)
		trackNumber = 0;
	soundfile = fileNames[trackNumber];
	DPRINT("soundfile playNext= ");
	DPRINTLN(soundfile);
	while (!vs1053StartPlayingFile(soundfile))
		;
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
	//     DPRINTLN("coucou prev");
	// if (!digitalRead(BUTTON_PLAY))
	//     DPRINTLN("coucou play");
	// if (!digitalRead(BUTTON_NEXT))
	//     DPRINTLN("coucou next");
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
				DPRINTLN("prevButton pushed");
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
				DPRINTLN("nextButton pushed");
			}
		}
	}
	lastNextButtonState = readingNext;
}

#ifdef DEVEL
void testExp(void)
{
	float testExp;
	float testSqrt;
	for (int i = 0; i < 255; i++)
	{

		// testExp = map((float) i, 0, 255, 0, 1);
		// DPRINT("exp(");
		// DPRINT(testExp);
		// DPRINT(")= ");
		// DPRINTLN(testExp = map(exp(testExp), 0, 2.71, 0, 1));
		// DPRINT("exp* ");
		// DPRINT(i);
		// DPRINT("=");
		// DPRINTLN((int) (testExp*i));
		testSqrt = map((float)i, 0, 255, 0, 1);
		DPRINT("sqrt(");
		DPRINT(testSqrt);
		DPRINT(")= ");
		// DPRINTLN(sqrt(testSqrt));
		DPRINTLN(testSqrt = sqrt(testSqrt));
		DPRINT("sqrt* ");
		DPRINT(i);
		DPRINT("=");
		DPRINTLN((int)(testSqrt * 255));
	}
	while (1)
		;
}
#endif

ExponentMap<int> expoVolume(1022, 1023);
int volumEx;
elapsedMillis display;

void getScaledVolumeEx(void)
{
	float volumeSqrt;
	pot_debounce(POT_DEBOUNCE_THRESHOLD);

// #ifdef DEBUG
// 	if (display >= 20)
// 	{
// 		DPRINT("volume POT=");
// 		DPRINTLN(volume_pot);
// 	}
// #endif
	volumeSqrt = map((float)volume_pot, 0, 1023, 0, 1);
	volume = map(sqrt(sqrt(volumeSqrt)), 0, 1, 200, 1); // racine carre de racine carree pour un rendu plus naturel (sorte d'exponentiel à l'envers)
// #ifdef DEBUG
// 	if (display >= 20)
// 	{
// 		DPRINT("volume SQRT MAPPED=");
// 		DPRINTLN(volume);
// 		display = 0;
// 	}
// #endif
	while (audioamp.getGain() != amp_gain) // a confirmer - je ne vois pas a quoi ca sert pour l'instant
		;
}

int tr = exp(20);

void getScaledVolume(void)
{
	pot_debounce(POT_DEBOUNCE_THRESHOLD);
	if (volume_pot >= 0 && volume_pot <= 400)
		volume = (unsigned int)map(volume_pot, 0, 400, 200, 30);
	if (volume_pot > 400)
		volume = (unsigned int)map(volume_pot, 400, 1023, 29, 1);
	while (audioamp.getGain() != amp_gain)
		;
}

/* pour la conversion exponentielle */
///
const int pwm_max_value = 1023;

// Since we won't provide the number of steps, this variable
// will be defined after the object calculates them.
int steps_count;

ExponentMap<int> e(1022, 1023);
ExponentMap<int> expoLampVu(1022, 1023);

///

void setup()
{
#ifdef DEBUG
	Serial.begin(9600);
	while (!Serial)
		; // wait for Arduino Serial Monitor
	DPRINTLN(F("Serial OK"));
#endif
	// testExp(); /// WIP
	pinMode(FET, OUTPUT);
	analogWriteFrequency(FET, 128000);
	analogWriteResolution(10);
	pinMode(BUTTON_PREV, INPUT_PULLUP);
	pinMode(BUTTON_PLAY, INPUT_PULLUP); // tbc
	pinMode(BUTTON_NEXT, INPUT_PULLUP);
	if (!setup_oled())
		DPRINTLN(F("Oled not found"));
	else
		DPRINTLN(F("Oled init"));
	delay(DELAY_STARTUP_SCREENS);

	audioamp.begin();
	if (!audioamp.begin())
	{ // initialise the music player
		// DPRINTLN(F("Couldn't find amp"));
		while (1)
			;
	}
	else
	{
		message_oled("audio amp ok");
		delay(500);
	}
	DPRINTLN(F("Amp found"));
	audioamp.setAGCCompression(TPA2016_AGC_OFF);
	audioamp.setReleaseControl(0);
	audioamp.setAttackControl(0);
	audioamp.setHoldControl(0);
	audioamp.setLimitLevelOff();
	audioamp.setGain(amp_gain);
	while (audioamp.getGain() != amp_gain)
	{
		audioamp.setGain(amp_gain);
	}

	char str3[10];															// tmp wip
	message_oled(strcat("amp gain =", itoa(audioamp.getGain(), str3, 10))); // tmp wip
	delay(DELAY_STARTUP_SCREENS);											// tmp wip
#ifdef RANDOM_INIT
	Entropy.Initialize();
#endif

	// if (vs1053vs_init()) { // initialise the music player
	if (!vs1053Begin())
	{ // initialise the music player
		DPRINTLN(F("Couldn't find vs1053"));
		message_oled("vs1053 not found");
		while (1)
			;
	}
	else
	{
		message_oled("vs1053 found");
		// DPRINTLN(F("vs1053 found"));
	}
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
		message_oled("Vu meter not OK");
		delay(DELAY_STARTUP_SCREENS);
	}
	fileCount = listFiles();
	itoa(fileCount, id3.fileTotal, 10);
	message_oled(strcat("filecount= ", id3.fileTotal));
	delay(DELAY_STARTUP_SCREENS);
	DPRINT(F("fileCount:"));
	DPRINTLN(fileCount);
	// DPRINT("sounfile is ");
	// DPRINTLN(fileNames[0]);

	// Set volume for left, right channels. lower numbers is higher volume
	// delay(500);

	// If XDREQ is on an interrupt pin (any Teensy pin) can do background audio playing
	vs1053Interrupt(); // XDREQ int
	// message_oled("set vs1053 Interrupt OK");
	// delay(800);

	// Play one file, don't return until complete - i.e. serial "s" or "p" will not interrupt
	// DPRINTLN(F("jurg"));
	// vs1053PlayFullFile("jurg.mp3");
	//  DPRINTLN(F("Playing track 008"));
	//  vs1053PlayFullFile("track008.wav");
	//  DPRINTLN(F("Playing SDTEST with super long file name.wav"));
	//  vs1053PlayFullFile("/SDTEST with super long file name.wav");

	// Play another file in the background, use interrupt serial "s" or "p" will interrupt
	// Can only do one buffer feed at a time
	// DPRINTLN(F("Playing track 002 - press p to pause, s to stop"));
	// vs1053StartPlayingFile("track002.mp3");
	//  DPRINTLN(F("Playing track 010 - press p to pause, s to stop"));
	//  vs1053StartPlayingFile("track010.wav");

	vs1053SciWrite(0x00, SM_SDINEW | SM_LINE1); // new mode + line in
	DPRINT("SCI mode 0x");
	DPRINTLN(vs1053SciRead(0x00), HEX);
	char str2[10];
	itoa(vs1053SciRead(0x00), str2, 16);
	message_oled(strcat("SCI mode 0x", str2));
	delay(DELAY_STARTUP_SCREENS);

	// DPRINT("SCI mode after change 0x");
	// DPRINTLN(vs1053SciRead(0x00), HEX);

	// message_oled("start playingfile OK");
	pinMode(VOLUME_ROTARY_POT, INPUT);
	volume_pot = analogRead(VOLUME_ROTARY_POT);
#ifndef NO_VOL_POT
	// getScaledVolume();
	getScaledVolumeEx(); // WIP
#endif
#ifdef NO_VOL_POT
	volume = 29;
#endif
	vs1053SetVolume(volume, volume);
	message_oled("set volume ok");
	delay(DELAY_STARTUP_SCREENS);
// DPRINTLN(volume_pot);
// DPRINTLN(volume);
#ifdef RANDOM_FIRST_TRACK
	trackNumber = Entropy.random(fileCount);
#endif
	soundfile = fileNames[trackNumber];
	// DPRINT("soundfile:");
	// DPRINTLN(soundfile);
	DPRINT("fileNames[trackNumber]: ");
	DPRINTLN(fileNames[trackNumber]);
	//  playFilesInLoop("/");
	// id3 = parse_id3(id3);
	vs1053StartPlayingFile(soundfile);
	message_oled("start playingfile OK");
	delay(DELAY_STARTUP_SCREENS);
	DPRINTLN(F("Playing - press p to pause, s to stop"));
	if (playingMusic)
		DPRINTLN("Playback started");
	else
		DPRINTLN("Playback failed");
	steps_count = e.stepsCount();
}

elapsedMillis currentMilliVU;
int vu_level = 0;
int currentMilliPrev = millis();
int previousMilliPrec = currentMilliPrev;

#ifdef DEVEL
void ramp_check()
{
	ramp rampCheck;
	elapsedMillis delay1;
	elapsedMillis delay2;
	bool passed1 = false;
	bool passed2 = false;

	while (1)
	{
		if (delay1 >= 200 && !passed1)
		{
			rampCheck.go(255, 200);
			// delay1 = 0;
			passed1 = true;
		}
		if (delay1 >= 2000 && !passed2)
		{
			rampCheck.go(10, 200);
			DPRINTLN("coucou");
			passed2 = true;
			// delay1 = 0;
		}
		if (delay2 >= 10)
		{
			DPRINTLN(rampCheck.update());
			delay2 = 0;
		}
	}
}
#endif

#ifdef DEVEL
void check_fet_lamp()
{
	rampInt rampLamp;
	elapsedMillis lampPotDelay;
	elapsedMillis debugDelay;
	int volPot = 0;

	while (1)
	{
		if (lampPotDelay >= 30)
		{
			char nbr[8];
			volPot = analogRead(VOLUME_ROTARY_POT);
			// volPot = map(volPot, 0, 1023, 0, 255);
			DPRINTLN(volPot);
			volPot = e(volPot - 1);
			volPot = map(volPot, 0, 1023, LAMP_OFFSET, 1023);
			DPRINTLN(volPot);
			// message_oled((const char*) itoa(map(volPot, 0, 1023, 0, 255), nbr, 10));
			message_oled((const char *)itoa(volPot, nbr, 10));
			rampLamp.go(volPot, LAMP_RAMP_TIME);
			// DPRINT("led level=")
			// DPRINT(map(volPot, 0, 1023, 0, 255));

			lampPotDelay = 0;
		}
		analogWrite(FET, rampLamp.update());
	}
}
#endif

rampInt rampVU;

void lampVUmeter()
{
	if (currentMilliVU >= 50)
	{
		vu_level = vs1053VuLevel() >> 8;
		// DPRINT("vu level brut=");
		// DPRINTLN(vu_level);
		// DPRINT("left=");
		// DPRINTLN(vu_level >> 8);
		// DPRINT("right=");
		// DPRINTLN(vu_level & 0xFF);
		vu_level = map(vu_level, 0, 99, LAMP_OFFSET, 1023);
		// DPRINT("vu mapped=");
		// DPRINTLN(vu_level);
		vu_level = expoLampVu(vu_level - 1);
		vu_level = map(vu_level, 0, 1023, LAMP_OFFSET, 1023);
		// DPRINT("vu expo + offset=");
		// DPRINTLN(vu_level);
		rampVU.go(vu_level, LAMP_RAMP_TIME);
		currentMilliVU = 0;
	}
	analogWrite(FET, rampVU.update());
}

void loop()
{
#ifdef CHECK_FET_LAMP
	check_fet_lamp();
#endif
#ifdef DEBUG
	check_serial();
#endif
	lampVUmeter();
	// for (int i = 0; i < 20; i++)
	// {
	//   vu_level += vs1053VuLevel();
	// }
	// vu_level /= 20;
	// File is playing in the background

	// if (vs1053Stopped()) {
	// 	message_oled("playback stopped");
	// 	analogWrite(FET, 0);
	// 	DPRINTLN("Terminated");
	//    while (!check_serial()) {
	// 	}
	//   check_serial();
	// }

	if (vs1053Stopped())
	{
		playNext();

		// vs1053resetPosition();
		// analogWrite(FET, 0);
		// DPRINT("SCI mode at stop 0x");
		// DPRINTLN(vs1053SciRead(0x00), HEX);
		// DPRINT("SCI Clock");
		// DPRINTLN(vs1053SciRead(SCI_CLOCKF));
		// if (trackNumber < fileCount - 1)
		// trackNumber++;
		// else if (trackNumber == fileCount - 1)
		// trackNumber = 0;
		// DPRINT("trackNumber= ");
		// DPRINTLN(trackNumber);
		// soundfile = fileNames[trackNumber];
		// vs1053StartPlayingFile(soundfile);
	}

	// if (vs1053Stopped() && trackNumber < fileCount - 1) {
	// 	soundfile = fileNames[++trackNumber];
	// 	DPRINT("trackNumber");
	// 	DPRINTLN(trackNumber);
	// 	vs1053StartPlayingFile(soundfile);

	// }
	// else if (vs1053Stopped() && trackNumber == fileCount - 1) {
	// 	trackNumber = 0;
	// 	DPRINT("trackNumber");
	// 	DPRINTLN(trackNumber);
	// 	soundfile = fileNames[trackNumber];
	// 	vs1053StartPlayingFile(soundfile);
	// }

	// check_serial();

	// DPRINTLN("Loop running");
	// DPRINT("pot= ");
	// DPRINTLN(analogRead(22));;

	// logVol = pow(10, volume_pot / 1023.0) - 1;
	// logVol = (int) map(logVol, 0, 9, 100, 0);
	// read = (int) logVol;
	// DPRINT("logVol mapped=");
	// DPRINTLN(logVol);

#ifndef NO_VOL_POT
	getScaledVolumeEx();
	vs1053SetVolume(volume, volume);
#endif

	// delay(10);
	/*
	if (vs1053Stopped()) {
		DPRINTLN("lecture terminee");
		vs1053StartPlayingFile(soundfile);
	}
	*/
	// loop_oled_scroll(soundfile); // not working

	// DPRINTLN(vu_level);

	// DPRINTLN(vu_level);
	// DPRINTLN(vs1053getVUmeter());

	// loop_oled(id3, soundfile);

	scroll_loop(id3);

	// delay(1000);
	// analogWrite(FET, 10);
	// delay(1000);
	buttonCheck();
}

elapsedMillis potDebounceDelay;

void pot_debounce(int threshold)
{
	int read;
	if (potDebounceDelay >= 10)
	{
		read = abs(analogRead(VOLUME_ROTARY_POT) - 1023);
		// DPRINT("read_pot= ");
		// DPRINTLN(read);
		if (read > volume_pot + threshold || read < volume_pot - threshold)
			volume_pot = read;
		// DPRINT("volum_pot= ");
		// DPRINTLN(volume_pot);
		// DPRINT("volume ");
		// DPRINTLN(volume);
		potDebounceDelay = 0;
	}
}

int compareNames(const void *a, const void *b)
{
	return strcmp(*(const char **)a, *(const char **)b);
}

// Fonction de comparaison personnalisée (met dans l'ordre selon les numeros avant le nom de fichier)
int compareListNbr(const void *a, const void *b)
{
	// Extrait les numéros à partir des chaînes de caractères
	int num1, num2;
	sscanf(*(const char **)a, "%d-", &num1);
	sscanf(*(const char **)b, "%d-", &num2);

	return num1 - num2;
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
					int len = strlen(entry.name());
					DPRINT("file len =");
					DPRINTLN(len);
					fileNames[count] = new char[len + 1];
					fileNames[count][len] = 0;
					strcpy(fileNames[count], entry.name());
					// frameInfo(entry); /// WIP
					DPRINTLN(fileNames[count]);
					message_oled(fileNames[count]);
					count++;
				}
				else
				{
					message_oled("too many files");
					return (0);
				}
			}
			entry.close();
		}
		root.close();

		qsort(fileNames, count, sizeof(char *), compareListNbr);
	}
	else
	{
		DPRINTLN("Impossible d'ouvrir le dossier.");
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
// 	DPRINTLN("current track@854");
// 	DPRINTLN(currentTrack);

// 	while (currentTrack)
// 	{
// 		if (!currentTrack.isDirectory() && hasExtension(currentTrack.name(), ".mp3"))
// 		{
// 			DPRINT("Lecture du fichier : ");
// 			DPRINTLN(currentTrack.name());

// 			// Lecture du fichier MP3
// 			if (!vs1053StartPlayingFile(currentTrack.name()))
// 			{
// 				DPRINTLN("Erreur lors de la lecture du fichier");
// 			}
// 			parse_id3();
// 			//   Attente de la fin de la lecture
// 			while (!vs1053Stopped)
// 			{
// 				loop_oled(id3, soundfile);
// 				// Ajoutez ici d'autres traitements ou fonctionnalités si nécessaire
// 			}

// 			DPRINTLN("Lecture terminée");
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

// int	id3v2_read() {
// 	return(0);
// }
