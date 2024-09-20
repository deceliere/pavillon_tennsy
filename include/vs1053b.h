#ifndef VS1053B_H
#define VS1053B_H

#include <Arduino.h>

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

////////////////////////////////////////////////////////////////
// Configure interrupt for Data XDREQ from vs1053
// XDREQ is low while the receive buffer is full
////////////////////////////////////////////////////////////////

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

#define vs1053_CONTROL_SPI_SETTING SPISettings(250000, MSBFIRST, SPI_MODE0) // 2.5 MHz SPI speed Control
#define vs1053_DATA_SPI_SETTING SPISettings(8000000, MSBFIRST, SPI_MODE0)	// 8 MHz SPI speed Data

union twobyte
{
	uint16_t word;
	uint8_t byte[2];
};

#endif