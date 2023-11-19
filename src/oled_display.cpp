#include <Arduino.h>
// #include <U8g2lib.h> // not needed
#include "pavillon.h"
#include "oled_logo.h"

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif


U8G2_SH1107_64X128_1_4W_SW_SPI u8g2(U8G2_R1, /* clock=*/27, /* data=*/26, /* cs=*/14, /* dc=*/15, /* reset=*/16);

// u_int32_t count = 0;
char str[33];
elapsedMillis currentMillli_oled;
// int previousMilli_oled;
char minStr[3]; // minutes
char secStr[3]; // secondes
static char minSecStr[6];

/* pour le scrolling - WIP */
// u8g2_uint_t offset;			// current offset for the scrolling text
// u8g2_uint_t width;			// pixel width of the scrolling text (must be lesser than 128 unless U8G2_16BIT is defined
void scroll_setup(void);
void scroll_loop(void);

bool setup_oled(void)
{
  // Serial.begin(115200);
  // while(!Serial) ;

  // u8g2.setBusClock(400000);
  /* U8g2 Project: SSD1306 Test Board */
  // pinMode(10, OUTPUT);
  // pinMode(9, OUTPUT);
  // digitalWrite(10, 0);
  // digitalWrite(9, 0);

  /* U8g2 Project: T6963 Test Board */
  // pinMode(18, OUTPUT);
  // digitalWrite(18, 1);

  /* U8g2 Project: KS0108 Test Board */
  // pinMode(16, OUTPUT);
  // digitalWrite(16, 0);

  /* U8g2 Project: LC7981 Test Board, connect RW to GND */
  // pinMode(17, OUTPUT);
  // digitalWrite(17, 0);

  /* U8g2 Project: Pax Instruments Shield: Enable Backlight */
  // pinMode(6, OUTPUT);
  // digitalWrite(6, 0);

  // SPI1.setMOSI(26);
  // SPI1.setSCK(27);

  currentMillli_oled = 0;
  // previousMilli_oled = currentMillli_oled;

  if (!u8g2.begin())
    return (false);
  // DPRINTLN("oled not found");
  else
  {
    u8g2.setFontMode(0); // enable transparent mode, which is faster
#ifdef LOGO_STARTUP
    oled_logo_xbm();
#ifdef LOGO_STUCK
    while (1)
      ;
#else
    delay(1000);
#endif
#endif
    delay(500);
    message_oled("oled setup ok");
    delay(100);
#ifdef SCROLLING_TEST
    scroll_setup();
    while (1)
      scroll_loop();
#endif
    return (true);
  }
  // DPRINTLN("oled init ok  ");
}

void message_oled(const char *message)
{
  // DPRINTLN("coucou loop");
  u8g2.firstPage();
  do
  {
    u8g2.setFont(FONT_NORMAL);
    u8g2.drawUTF8(0, 10, message);
    // u8g2.drawUTF8(0, 64, "bisouéèêėàç");
  } while (u8g2.nextPage());
}

char *GetMinSec(void)
{
  u_int32_t count = 0;
  uint8_t min;
  uint8_t sec;

  count = vs1053getPosition();
  sec = count % 60;
  min = count / 60;
  itoa(sec, secStr, 10);
  itoa(min, minStr, 10);
  if (min < 10)
  {
    strcpy(minSecStr, "0");
    strcat(minSecStr, minStr);
  }
  else
    strcpy(minSecStr, minStr);
  strcat(minSecStr, ":");
  if (sec < 10)
    strcat(minSecStr, "0");
  strcat(minSecStr, secStr);
  return (minSecStr);
}

void loop_oled(s_id3 id3, const char *soundfile)
{
  // currentMillli_oled = 0;
  if (currentMillli_oled > 100)
  {

    strcpy(minSecStr, GetMinSec());
    // DPRINT("id3.time=");
    // DPRINTLN(id3.time);
    currentMillli_oled = 0;
  }
  // strcpy(id3.time, minSecStr);
  u8g2.firstPage();
  do
  {
    u8g2.setFont(FONT_NORMAL);
    if (!isAlphaNumeric(id3.title[0]))
      u8g2.drawUTF8(0, 10, soundfile);
    else
      // u8g2.drawUTF8(0, 8, id3.title);
      u8g2.drawUTF8(0, 10, id3.title);
    if (!isAlphaNumeric(id3.artist[0]))
      u8g2.drawUTF8(0, 22, DEFAULT_ARTIST);
    else
      // u8g2.drawUTF8(0, 8, id3.title);
      u8g2.drawUTF8(0, 22, id3.artist);
    if (!isAlphaNumeric(id3.album[0]))
      u8g2.drawUTF8(0, 35, DEFAULT_ALBUM);
    else
      u8g2.drawUTF8(0, 35, id3.album);
    // u8g2.drawStr(0, 53, itoa(count, str, 10));
    u8g2.drawUTF8(0, 53, minSecStr);

    u8g2.drawUTF8(OLED_WIDTH - u8g2.getUTF8Width(id3.trackDisplay), 53, id3.trackDisplay);
  } while (u8g2.nextPage());
  // DPRINTLN(id3.time);

  // delay(10);
  // DPRINTLN(str);
  // DPRINTLN(count);
}

/*

// scrolling WIP //

void loop_oled_scroll(const char *soundfile) {
  width = u8g2.getUTF8Width(soundfile);
  static u8g2_uint_t x;

  u8g2.firstPage();
  do {

    // draw the scrolling text at current offset
    x = offset;
    u8g2.setFont(FONT_NORMAL);		// set the target font
    do {								// repeated drawing of the scrolling text...
      u8g2.drawUTF8(x, 30, soundfile);			// draw the scolling text
      x += width;						// add the pixel width of the scrolling text
    } while( x < u8g2.getDisplayWidth() );		// draw again until the complete display is filled

    u8g2.setFont(FONT_NORMAL);		// draw the current pixel width
    u8g2.setCursor(0, 58);
    u8g2.print("coucou");					// this value must be lesser than 128 unless U8G2_16BIT is set

  } while ( u8g2.nextPage() );

  offset-=1;							// scroll by one pixel
  if ( (u8g2_uint_t)offset < (u8g2_uint_t)-width )
    offset = 0;							// start over again

  delay(10);							// do some small delay
}
*/

/* test basic pour scroll */

scroll_msg scroll[3];

// const char *text1 = "oui oui c'est ça à asd é è ç ";	// scroll this text from right to left
// const char *text2 = "OUIIIIII ";	// scroll this text from right to left

void scroll_parseId3v2(s_id3 id3)
{

  // u8g2.begin();
  u8g2.setFont(FONT_NORMAL); // set the target font to calculate the pixel width
  // scroll[0].str = id3.title;
  strcpy(scroll[0].str, id3.title);
  // scroll[0].str = "coucou";
  scroll[0].y = FIRST_LINE;
  strcpy(scroll[1].str, id3.artist);
  // scroll[1].str = id3.artist;
  scroll[1].y = FIRST_LINE + SECOND_LINE;
  // scroll[2].str = id3.album;
  strcpy(scroll[2].str, id3.album);
  scroll[2].y = FIRST_LINE + SECOND_LINE + THIRD_LINE;
  // message_oled(scroll[0].str);
  // delay(1000);
  // scroll[1].str = "puree pourquoi ca marche pas?";

  for (int i = 0; i < 3; i++)
  {
    scroll[i].width = u8g2.getUTF8Width(scroll[i].str); // calculate the pixel width of the text
    if (scroll[i].width > 128)
    {
      strcat(scroll[i].str, "       "); // if scrolling, than add some spaces
      scroll[i].width = u8g2.getUTF8Width(scroll[i].str);
    }
    scroll[i].timer = 0;
    scroll[i].x = 0;
    scroll[i].offset = 0;
  }
  u8g2.setFontMode(0); // enable transparent mode, which is faster

  for (int i = 0; i < 3; i++) ///
  {
    DPRINT("scroll[");
    DPRINT(i);
    DPRINT("] = ");
    DPRINTLN(scroll[i].str);
    DPRINT("offset= ");
    DPRINTLN(scroll[i].offset);
    DPRINT("x= ");
    DPRINTLN(scroll[i].x);
    DPRINT("y= ");
    DPRINTLN(scroll[i].y);
    DPRINT("width= ");
    DPRINTLN(scroll[i].width);
    DPRINT("timer= ");
    DPRINTLN(scroll[i].timer);
  }
}

elapsedMillis scrollMillis = 0;

void scroll_loop(s_id3 id3)
{
  if (scrollMillis > SCROLL_SPEED)
  {
    // u8g2_uint_t x;

    // for (int i = 0; i < 2; i++)
    // {
    if (currentMillli_oled > 100)
    {

      strcpy(minSecStr, GetMinSec());
      // DPRINT("id3.time=");
      // DPRINTLN(id3.time);
      currentMillli_oled = 0;
    }
    u8g2.firstPage();
    do
    {

      u8g2.setFont(FONT_NORMAL); // set the target font
                                 // draw the scrolling text at current offset
      for (int i = 0; i < 3; i++)
      {
        scroll[i].x = scroll[i].offset;
        // if (msg[i].offset > u8g2.getDisplayWidth())
        // {
        // do
        // {                                         // repeated drawing of the scrolling text...
        u8g2.drawUTF8(scroll[i].x, scroll[i].y, scroll[i].str); // draw the scolling text
                                                                // x += msg[i].width;                      // add the pixel width of the scrolling text
        // } while (x < u8g2.getDisplayWidth());     // draw again until the complete display is filled
        // }
        // else
        // u8g2.drawUTF8(0, msg[i].y, msg[i].str);

        u8g2.setFont(FONT_NORMAL); // draw the current pixel width
        // u8g2.setCursor(0, scroll[i].y + 10);
        // u8g2.print(scroll[i].width); // this value must be lesser than 128 unless U8G2_16BIT is set
      }
      u8g2.drawUTF8(0, OLED_HEIGHT - LINE_FROM_BOTTOM, minSecStr);
      u8g2.drawUTF8(OLED_WIDTH - u8g2.getUTF8Width(id3.trackDisplay), OLED_HEIGHT - LINE_FROM_BOTTOM, id3.trackDisplay);

    } while (u8g2.nextPage());

    for (int i = 0; i < 3; i++)
    {
      if (scroll[i].width > OLED_WIDTH && scroll[i].timer > WAIT_BEFORE_SCROLL)
      {
        scroll[i].offset -= 1; // scroll by one pixel
        if ((u8g2_uint_t)scroll[i].offset < (u8g2_uint_t)-scroll[i].width)
        {
          scroll[i].offset = 0; // start over again
          scroll[i].timer = 0;
        }
      }
    }
    scrollMillis = 0;
  }
  // delay(20); // do some small delay
  // }
}

void oled_logo_xbm(void)
{
  u8g2.firstPage();
  do
  {
    u8g2.setFont(FONT_NORMAL);
    u8g2.drawUTF8(OLED_WIDTH - u8g2.getUTF8Width("Pavillon"), 12, "Pavillon");
    u8g2.drawUTF8(OLED_WIDTH - u8g2.getUTF8Width("d'écoute"), 25, "d'écoute");
    u8g2.drawUTF8(OLED_WIDTH - u8g2.getUTF8Width("HEROS"), 45, "HEROS");
    u8g2.drawUTF8(OLED_WIDTH - u8g2.getUTF8Width("LIMITE"), 55, "LIMITE");
    u8g2.setDrawColor(1);
    u8g2.setBitmapMode(1);
    u8g2.drawXBM(0, 0, LOGO_WIDTH, LOGO_HEIGHT, logo_bits);
  } while (u8g2.nextPage());
}