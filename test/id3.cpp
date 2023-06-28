//https://github.com/TobiasVanDyk/VS1053B-Teensy36-Teensy41-SDCard-Music-Player

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

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "Adafruit_TPA2016.h"
#include <cmath>

// Connect SCLK, MISO and MOSI to standard hardware SPI pins.
#define SCLK 13       // SPI Clock shared with SD card
#define MISO 12       // Input data from vs1053 or SD card
#define MOSI 11       // Output data to vs1053 or SD card

// These are the pins used for the Adafruit vs1053B breakout module
#define XRST  9                // vs1053 reset (output)
#define XCS   10               // vs1053 chip select (output)
#define XDCS  8                // vs1053 Data select (output)
#define XDREQ 3                // vs1053 Data Ready an Interrupt pin (input)
//#define SDCS  BUILTIN_SDCARD   // Use Teensy built-in card
// For Teensy 3.5, 3.6, 4.0, 4.1 better to use its built-in SDCard
#define SDCS 4                // Use vs1053 SDCard Card chip select pin


/////////////////////////////////////////////////////////////////////////////////
// vs1053B.h
/////////////////////////////////////////////////////////////////////////////////

#define vs1053_FILEPLAYER_TIMER0_INT 255 // allows useInterrupt to accept pins 0 to 254
#define vs1053_FILEPLAYER_PIN_INT 5      // Interrupt number from dreqinttable dreq pin # = 3

#define vs1053_SCI_READ 0x03
#define vs1053_SCI_WRITE 0x02

#define vs1053_REG_MODE  0x00
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

#define vs1053_INT_ENABLE  0xC01A

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


////////////////////////////////////////////////////////////////////////////////
// vs1053B.cpp
////////////////////////////////////////////////////////////////////////////////
#define vs1053_CONTROL_SPI_SETTING  SPISettings(250000,  MSBFIRST, SPI_MODE0) // 2.5 MHz SPI speed Control 
#define vs1053_DATA_SPI_SETTING     SPISettings(8000000, MSBFIRST, SPI_MODE0) // 8 MHz SPI speed Data


////////////////////////////////////////////////////////////////
// Configure interrupt for Data XDREQ from vs1053
// XDREQ is low while the receive buffer is full
////////////////////////////////////////////////////////////////
void vs1053Interrupt()
{ SPI.usingInterrupt(XDREQ);                          // Disable Interrupt during SPI transactions
  attachInterrupt(XDREQ, vs1053FeedBuffer, CHANGE);   // Interrupt on Pin XDREQ state change
}                                                    // feeder->feedBuffer executed (lines 26, 209)

//////////////////////////////////////////////////////////
// Set the card to be disabled while we get the vs1053 up
//////////////////////////////////////////////////////////
void vs1053DisableCard ()
{ playingMusic = false;
  pinMode(SDCS, OUTPUT);
  digitalWrite(SDCS, HIGH);
}

/////////////////////////////////////////////////////////////////////////
// Play file without interrupts
/////////////////////////////////////////////////////////////////////////
boolean vs1053PlayFullFile(const char *trackname) {
  Serial.print("Playing ");
  Serial.println(trackname);
  if (! vs1053StartPlayingFile(trackname)) return false;
  while (playingMusic) {
    vs1053FeedBuffer();
  }
  return true;
}

/////////////////////////
void vs1053StopPlaying()
{ playingMusic = false;
  currentTrack.close();
}

///////////////////////////////////////
void vs1053PausePlaying(boolean pause)
{ if (pause) playingMusic = false; else {
    playingMusic = true;
    vs1053FeedBuffer();
  }
}

///////////////////////
boolean vs1053Paused()
{ return (!playingMusic && currentTrack);
}

////////////////////////
boolean vs1053Stopped()
{ return (!playingMusic && !currentTrack);
}

//////////////////////////////////////////////////////
boolean vs1053StartPlayingFile(const char *trackname)  {
  Serial.println("vs1053StartPlayingFile");
  currentTrack = SD.open(trackname);
  if (!currentTrack) {
    Serial.print("Failed to open from SD: ");
    Serial.println(trackname);
    return false;
  }
  else {
    Serial.print("Opened from SD: ");
    Serial.println(trackname);
  }
  playingMusic = true;
  while (!vs1053ReadyForData() );                                    // wait for ready for data
  while (playingMusic && vs1053ReadyForData()) vs1053FeedBuffer();   // then send data
  return true;
}


void vs1053FeedBuffer() { //debugging this function causes memory overruns
  //  Serial.println("vs1053FeedBuffer");

  static uint8_t running = 0;
  uint8_t sregsave;

  // Do not allow 2 FeedBuffer instances to run concurrently
  // SREG bit 7 = OFF => no interrupts until bit 7 = ON
  //  sregsave = SREG; // Status Register can clear/set interrupt enable bit 7 cli or sei
  noInterrupts();           // Clear interrupt enable bit 7 in SREG => disable interrupts
  //  if (running) { SREG = sregsave; return; } else { running = 1; SREG = sregsave;  }
  // paused or stopped. no SDCard track open, XDREQ=0 receive buffer full
  if ((!playingMusic) || (!currentTrack) || (!vs1053ReadyForData())) {
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
  { int bytesread = currentTrack.read(SoundBuffer, vs1053_DATABUFFERLEN);
    if (bytesread == 0)           // End of File
    { playingMusic = false;
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
{ return digitalRead(XDREQ);
}

void vs1053PlayData(uint8_t *buffer, uint8_t buffsiz)
{ SPI.beginTransaction(vs1053_DATA_SPI_SETTING);
  digitalWrite(XDCS, LOW);

  for (uint8_t i = 0; i < buffsiz; i++) {
    vs1053SpiWrite(buffer[i]);  // buffsiz = 32
  }

  digitalWrite(XDCS, HIGH);
  SPI.endTransaction();
}

void vs1053SetVolume(uint8_t left, uint8_t right)
{ uint16_t v;
  v = left;
  v <<= 8;
  v |= right;

  cli();
  vs1053SciWrite(vs1053_REG_VOLUME, v);
  sei();
}

uint16_t vs1053DecodeTime()
{ cli();
  uint16_t t = vs1053SciRead(vs1053_REG_DECODETIME);
  sei();
  return t;
}

void vs1053SoftReset()
{ vs1053SciWrite(vs1053_REG_MODE, vs1053_MODE_SM_SDINEW | vs1053_MODE_SM_RESET);
  delay(100);
}

void vs1053Reset()
{ if (XRST >= 0)
  { digitalWrite(XRST, LOW);
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
{ if (XRST >= 0) {
    pinMode(XRST, OUTPUT);  // if reset = -1 ignore
    digitalWrite(XRST, LOW);
  }

  pinMode(XCS, OUTPUT);
  digitalWrite(XCS, HIGH);
  pinMode(XDCS, OUTPUT);
  digitalWrite(XDCS, HIGH);
  pinMode(XDREQ, INPUT);

  SPI.begin();
  //  SPI.setDataMode(SPI_MODE0);
  //  SPI.setBitOrder(MSBFIRST);
  //  SPI.setClockDivider(SPI_CLOCK_DIV128);

  vs1053Reset();

  return (vs1053SciRead(vs1053_REG_STATUS) >> 4) & 0x0F;
}

/////////////////////////////////////
uint16_t vs1053SciRead(uint8_t addr)
{ uint16_t data;
  SPI.beginTransaction(vs1053_CONTROL_SPI_SETTING);
  digitalWrite(XCS, LOW);
  vs1053SpiWrite(vs1053_SCI_READ);
  vs1053SpiWrite(addr);
  delayMicroseconds(10);
  data = vs1053SpiRead();
  data <<= 8;
  data |= vs1053SpiRead();
  digitalWrite(XCS, HIGH);
  SPI.endTransaction();
  return data;
}

//////////////////////////////////////////////
void vs1053SciWrite(uint8_t addr, uint16_t data)
{ SPI.beginTransaction(vs1053_CONTROL_SPI_SETTING);
  digitalWrite(XCS, LOW);
  vs1053SpiWrite(vs1053_SCI_WRITE);
  vs1053SpiWrite(addr);
  vs1053SpiWrite(data >> 8);
  vs1053SpiWrite(data & 0xFF);
  digitalWrite(XCS, HIGH);
  SPI.endTransaction();
}

static volatile uint8_t *clkportreg;
static uint8_t clkpin;

////////////////////////
uint8_t vs1053SpiRead()
{ int8_t x;
  x = 0;
  //clkportreg = portOutputRegister(digitalPinToPort(SCLK));
  //clkpin = digitalPinToBitMask(SCLK);
  // MSB first, clock low when inactive (CPOL 0), data valid on leading edge (CPHA 0)
  // Make sure clock starts low
  x = SPI.transfer(0x00);
  // Make sure clock ends low
  //*clkportreg &= ~clkpin;

  return x;
}

///////////////////////////////
void vs1053SpiWrite(uint8_t c)
{ // MSB first, clock low when inactive (CPOL 0), data valid on leading edge (CPHA 0)
  // Make sure clock starts low
  //clkportreg = portOutputRegister(digitalPinToPort(SCLK));
  //clkpin = digitalPinToBitMask(SCLK);
  SPI.transfer(c);
  //*clkportreg &= ~clkpin;   // Make sure clock ends low
}

/////////////////////////////////////
// End .cpp
/////////////////////////////////////



File mp3File;

void setup() {
Serial.begin(9600);
while (!Serial) ; // wait for Arduino Serial Monitor

if (! vs1053Begin()) { // initialise the music player
    Serial.println(F("Couldn't find vs1053"));
    while (1);
}
else
    Serial.println(F("vs1053 found"));

if (!SD.begin(SDCS)) {
    Serial.println("Erreur lors de l'initialisation de la carte SD");
    while (1);  // En cas d'erreur, on arrête l'exécution
  }
mp3File = SD.open("jurg_1_test1_4.mp3");

if (!mp3File) {
  Serial.println("Erreur lors de l'ouverture du fichier MP3.");
  return;
}

// Accède aux derniers 128 octets du fichier
const int fileSize = mp3File.size();
const int id3TagSize = 128;

if (fileSize < id3TagSize) {
  Serial.println("Le fichier est trop court pour contenir des tags ID3.");
  mp3File.close();
  return;
}

mp3File.seek(fileSize - id3TagSize);

// Lecture des données des tags ID3
char tag[4];
char title[31];
char artist[31];
char album[31];

mp3File.read(tag, 3);
tag[3] = '\0';

// Vérification si le fichier contient des tags ID3
if (strcmp(tag, "TAG") == 0) {
  // Lecture des informations ID3
  mp3File.read(title, 30);
  title[30] = '\0';

  mp3File.read(artist, 30);
  artist[30] = '\0';

  mp3File.read(album, 30);
  album[30] = '\0';

  // Affichage des informations ID3
  Serial.println("Informations ID3 :");
  Serial.print("Titre : ");
  Serial.println(title);
  Serial.print("Artiste : ");
  Serial.println(artist);
  Serial.print("Album : ");
  Serial.println(album);
} else {
  Serial.println("Aucun tag ID3 trouvé dans le fichier.");
}

// Ferme le fichier MP3
mp3File.close();
}

void loop() {

}