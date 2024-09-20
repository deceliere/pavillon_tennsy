#include "vs1053b.h"
#include "pavillon.h"
#include "variables.h"

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

///////////////////////////////
void vs1053getTrackInfo(uint8_t offset, char *info)
{
	const int fileSize = currentTrack.size();
	noInterrupts();
	// skip to end
	currentTrack.seek(fileSize - 128 + offset);
	// read 30 bytes of tag informat at -128 + offset
	currentTrack.read(info, 30);
	info[30] = 0;
	currentTrack.seek(0);
	// renable interupt
	interrupts();
}
/////////////////////////////////////
// End .cpp
/////////////////////////////////////