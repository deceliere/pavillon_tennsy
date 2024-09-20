#include "pavillon.h"
#include "vs1053b.h"
#include "variables.h"

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

#ifdef DEBUG_PAVILLON
/// waiting for serial messages
int check_serial()
{
	if (Serial.available())
	{
		char c = Serial.read();
		// if we get an 's' on the serial console, stop!
		if (c == 's')
			vs1053StopPlaying();
		if (c == 'r')
			vs1053StartPlayingFile(soundfile);
		if (c == 'n')
			playNext();
		if (c == 'c')
			playFromBegining();
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

void parse_id3v2()
{
	id3 = frameInfo(currentTrack, id3);
	itoa(trackNumber + 1, id3.fileCurrent, 10); // + 1 pour que la piste 0 s affiche comme etant la piste 1
	strcpy(id3.trackDisplay, id3.fileCurrent);
	strcat(id3.trackDisplay, " / ");
	strcat(id3.trackDisplay, id3.fileTotal);
	scroll_parseId3v2(id3); /// temp - for testing purpose
	DPRINT("trackDisplay=");
	DPRINTLN(id3.trackDisplay);
}

void playFromBegining()
{
	vs1053StopPlaying();
	while (!vs1053StartPlayingFile(soundfile))
		;
}

void playPrevious()
{
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
	vs1053StopPlaying();
	trackNumber++;
	if (trackNumber > fileCount - 1)
		trackNumber = 0;
	soundfile = fileNames[trackNumber];
	DPRINT("soundfile playNext= ");
	DPRINTLN(soundfile);
	while (!vs1053StartPlayingFile(soundfile))
		;
}

/* push buttons */

elapsedMillis prevMillis;
int previousButtonState = HIGH;
int lastPreviousButtonState = HIGH;
elapsedMillis nextMillis;
int nextButtonState = HIGH;
int lastNextButtonState = HIGH;
elapsedMillis playingMillis;
// int playButtonState = HIGH; // no play button implemented
// int lastPlayButtonState = HIGH // no play button implemented;
int releasedTime;
int pressedTime;

void buttonCheck()
{
	int readingPrevious = digitalRead(BUTTON_PREV);
	int readingNext = digitalRead(BUTTON_NEXT);

	if (readingPrevious != lastPreviousButtonState)
		prevMillis = 0;
	if (prevMillis >= BUTTON_DEBOUNCE_DELAY)
	{
		if (readingPrevious != previousButtonState)
		{
			previousButtonState = readingPrevious;
			if (previousButtonState == HIGH)
			{
				if (playingMillis <= PREVIOUS_JUMP_MILLIS)
					playPrevious();
				else
					playFromBegining();
				playingMillis = 0;
				prevMillis = 0;
				DPRINTLN("prevButton pushed");
			}
		}
	}
	lastPreviousButtonState = readingPrevious;
	if (readingNext != lastNextButtonState)
	{
		nextMillis = millis();
	}
	if (nextMillis >= BUTTON_DEBOUNCE_DELAY)
	{
		if (readingNext != nextButtonState)
		{
			nextButtonState = readingNext;
			if (nextButtonState == HIGH)
			{
				nextMillis = 0;
				playNext();
				DPRINTLN("nextButton pushed");
			}
		}
	}
	lastNextButtonState = readingNext;
}

ExponentMap<int> expoVolume(1022, 1023);
int volumEx;
elapsedMillis display;

void getScaledVolumeSq(void)
{
	float volumeSqrt;
	pot_debounce(POT_DEBOUNCE_THRESHOLD);
	volumeSqrt = map((float)volume_pot, 0, 1023, 0, 1);
	volume = map(sqrt(sqrt(volumeSqrt)), 0, 1, 200, 1); // racine carre de racine carree pour un rendu plus naturel (sorte d'exponentiel à l'envers)
	while (audioamp.getGain() != amp_gain)				// a confirmer - je ne vois pas a quoi ca sert pour l'instant
		;
}

#ifdef DEVEL
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
#endif

/* pour la conversion exponentielle */
///
const int pwm_max_value = 1023;

// Since we won't provide the number of steps, this variable
// will be defined after the object calculates them.
int steps_count;

// ExponentMap<int> e(1022, 1023); // pour test, voir si a garder
ExponentMap<int> expoLampVu(1022, 1023);
///

void setup()
{
#ifdef DEBUG_PAVILLON
	Serial.begin(9600);
	while (!Serial)
		; // wait for Arduino Serial Monitor
	DPRINTLN(F("Serial OK"));
#endif
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
	message_oled(strcat("version ", VERSION_PAVILLON)); 
	delay(2 * DELAY_STARTUP_SCREENS);
	message_oled(strcat("git ", GIT_VERSION)); 
	delay(2 * DELAY_STARTUP_SCREENS);
	audioamp.begin();
	if (!audioamp.begin())
	{ // initialise the music player
		// DPRINTLN(F("Couldn't find amp"));
		while (1)
			;
	}
	else
		message_oled("audio amp ok");
	delay(DELAY_STARTUP_SCREENS);
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
	message_oled(strcat("amp gain= ", itoa(audioamp.getGain(), str3, 10))); // tmp wip
	delay(DELAY_STARTUP_SCREENS);											// tmp wip
#ifdef RANDOM_FIRST_TRACK
	Entropy.Initialize();
#endif

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
		delay(DELAY_STARTUP_SCREENS);
		DPRINTLN(F("vs1053 found"));
	}
	while (!SD.begin(SDCS))
	{
		message_oled("SD card not found");
	} // initialise the SD card
	message_oled("SD card found");
	delay(DELAY_STARTUP_SCREENS);
	LoadUserCode();		 // patch pour avoir le Vu metre
	vs1053setVUmeter(1); // allumer les infos du Vu metre
	if (vs1053getVUmeter())
	{
		message_oled("Vu meter OK");
		delay(DELAY_STARTUP_SCREENS);
	}
	else
	{
		message_oled("Vu meter not OK");
		delay(DELAY_STARTUP_SCREENS);
	}
	fileCount = listFiles();
	itoa(fileCount, id3.fileTotal, 10);
	if (!fileCount)
	{
		message_oled("no valid MP3 found...");
		while(1)
			;
	}
	message_oled(strcat("filecount= ", id3.fileTotal));
	delay(DELAY_STARTUP_SCREENS);
	DPRINT(F("fileCount:"));
	DPRINTLN(fileCount);
	vs1053Interrupt(); // XDREQ int
	vs1053SciWrite(0x00, SM_SDINEW | SM_LINE1); // new mode + line in
	DPRINT("SCI mode 0x");
	DPRINTLN(vs1053SciRead(0x00), HEX);
	char str2[10];
	itoa(vs1053SciRead(0x00), str2, 16);
	message_oled(strcat("SCI mode 0x", str2));
	delay(DELAY_STARTUP_SCREENS);
	pinMode(VOLUME_ROTARY_POT, INPUT);
	volume_pot = analogRead(VOLUME_ROTARY_POT);
#ifndef NO_VOL_POT
	getScaledVolumeSq();
#endif
#ifdef NO_VOL_POT
	volume = 29;
#endif
	vs1053SetVolume(volume, volume);
	message_oled("set volume ok");
	delay(DELAY_STARTUP_SCREENS);
#ifdef RANDOM_FIRST_TRACK
	trackNumber = Entropy.random(fileCount);
#endif
	soundfile = fileNames[trackNumber];
	DPRINT("fileNames[trackNumber]: ");
	DPRINTLN(fileNames[trackNumber]);
	vs1053StartPlayingFile(soundfile);
	message_oled("start playingfile OK");
	delay(DELAY_STARTUP_SCREENS);
	DPRINTLN(F("Playing - press p to pause, s to stop"));
	if (playingMusic)
		DPRINTLN("Playback started");
	else
		DPRINTLN("Playback failed");
	steps_count = expoLampVu.stepsCount();
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
		vu_level = map(vu_level, 0, 99, LAMP_OFFSET, 1023);
		vu_level = expoLampVu(vu_level - 1);
		vu_level = map(vu_level, 0, 1023, LAMP_OFFSET, 1023);
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
#ifdef DEBUG_PAVILLON
	check_serial();
#endif
	lampVUmeter();
	if (vs1053Stopped())
	{
		elapsedMillis logoMillis;

		while (logoMillis <= 5000) // pause 3 secs entre chaque piste - avec affichage logo
			oled_logo_xbm();
		playNext();
	}
#ifndef NO_VOL_POT
	getScaledVolumeSq();
	vs1053SetVolume(volume, volume);
#endif
	scroll_loop(id3);
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

bool hasExtension(const char *filename, const char *extension)
{
	const char *ext = strrchr(filename, '.');

	if (ext != nullptr && *filename != '.')
	{
		return strcmp(ext, extension) == 0;
	}

	return false;
}
