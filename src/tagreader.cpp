
#include <pavillon.h>

void checkUTF(s_id3 id3) // TEMP - pour comprendre
{
    int len;

    len = strlen(id3.title);
    DPRINT("len=");
    DPRINTLN(len);
    for (int i = 0; i < len; i++)
    {
        DPRINT("char[");
        DPRINT(i);
        DPRINT("]=");
        DPRINT(id3.title[i]);
        DPRINT("\t int=");
        DPRINTLN((int)id3.title[i]);
    }
    while (1)
        ;
}

s_id3 frameInfo(File track, s_id3 id3)
{
    char id3_check[4]; // pour voir si ID3 est bien present au debut du fichier
    char version[2]; //

    strcpy(id3.title, track.name());
    strcpy(id3.artist, DEFAULT_ARTIST);
    strcpy(id3.album, DEFAULT_ALBUM);
    // checkUTF(id3);
    noInterrupts();
    track.seek(0); // peut etre pas necessaire, a voir.
    track.read(id3_check, 3);
    id3_check[3] = 0;
    track.seek(3);
    track.read(version, 2);
    if (strcmp(id3_check, "ID3"))
    {
        DPRINTLN("no ID3 tag found in file");
        DPRINT("found in file=");
        DPRINTLN(id3_check);
        track.seek(0);
        interrupts();
        return (id3);
    }
    else if (!strcmp(id3_check, "ID3") && version[0] >= 3)
    {
        DPRINTLN("file with ID3v30 ok");
        DPRINT("found in file=");
        DPRINTLN(id3_check);
        DPRINT("version=");
#ifdef DEBUG_PAVILLON
        version[0] += '0';
        version[1] += '0';
#endif
        DPRINT(version[0]);
        DPRINT(".");
        DPRINT(version[1]);
        id3 = id3v2r30(track, id3);
    }
    else if (!strcmp(id3_check, "ID3") && version[0] < 3)
    {
        DPRINTLN("file with ID3v30 ok");
        DPRINT("found in file=");
        DPRINTLN(id3_check);
        DPRINT("version=");
#ifdef DEBUG_PAVILLON
        version[0] += '0';
        version[1] += '0';
#endif
        DPRINT(version[0]);
        DPRINT(".");
        DPRINT(version[1]);
        id3 = id3v2r20(track, id3);
        // attention que les interrupt soient remis
    }
    track.seek(0);
    interrupts();
    return (id3);
}

s_id3 id3v2r20(File track, s_id3 id3)
{
    char tvalue[256];
    char albval[256];
    char artval[256];

    uint32_t start = 10;
    uint32_t lastfrm = 10;
    bool done = false;

    while (!done)
    {
        DPRINTLN("\tin frameInfo loop v20");
        uint8_t buff[3];
        uint32_t ltwo[3];
        char tag[4];
        uint32_t fsize;

        noInterrupts();

        track.seek(start);
        track.read((uint8_t *)tag, 3);
        tag[3] = 0;
        // DPRINT("\ttag= ");
        // DPRINTLN(tag);
        if (!strIsAlphaNumeric(tag))
        {
            DPRINTLN("non alpha numeric");
            track.seek(0);
            interrupts();
            return (id3);
        }

        track.seek(start + 3);
        track.read((uint8_t *)buff, 3);
        ltwo[0] = buff[0];
        ltwo[1] = buff[1];
        ltwo[2] = buff[2];
        // ltwo[3] = buff[3];

        fsize = (ltwo[0] << 16) | (ltwo[1] << 8) | ltwo[2];
        start = start + 7;

        if (tag[0] == 'T' && tag[1] == 'T' && tag[2] == '2')
        {
            uint32_t i;
            int vlen = 0;
            char val[fsize];

            track.seek(start);
            track.read((uint8_t *)val, fsize);
            for (i = 0; i < fsize; i++)
            {
                // if (isAlphaNumeric(val[i]))
                if (isAlnumUTF(val[i]))
                {
                    tvalue[vlen] = val[i];
                    vlen++;
                }
                else if (val[i] == '(' || val[i] == ')' || val[i] == ' ')
                {
                    tvalue[vlen] = val[i];
                    vlen++;
                }
            }
            tvalue[vlen] = '\0';
#ifdef UPPER_TITLE
            strcpy(id3.title, strToUpper(tvalue));
#else
            strcpy(id3.title, tvalue);
#endif
            // DPRINT("tvalue: ");
            // DPRINTLN(tvalue);
            // DPRINT("id3.title: ");
            // DPRINTLN(id3.title);
        }
        else if (tag[0] == 'T' && tag[1] == 'A' && tag[2] == 'L')
        {
            uint32_t i;
            int vlen = 0;
            char val[fsize];

            track.seek(start);
            track.read((uint8_t *)val, fsize);
            for (i = 0; i < fsize; i++)
            {
                if (isAlphaNumeric(val[i]))
                {
                    albval[vlen] = val[i];
                    vlen++;
                }
                else if (val[i] == '(' || val[i] == ')' || val[i] == ' ')
                {
                    albval[vlen] = val[i];
                    vlen++;
                }
            }
            albval[vlen] = '\0';
            strcpy(id3.album, albval);
            DPRINT("albval: ");
            DPRINTLN(albval);
        }
        else if (tag[0] == 'T' && tag[1] == 'P' && tag[2] == '1')
        {
            uint32_t i;
            int vlen = 0;
            char val[fsize];

            track.seek(start);
            track.read((uint8_t *)val, fsize);
            for (i = 0; i < fsize; i++)
            {
                if (isAlphaNumeric(val[i]))
                {
                    artval[vlen] = val[i];
                    vlen++;
                }
                else if (val[i] == '(' || val[i] == ')' || val[i] == ' ')
                {
                    artval[vlen] = val[i];
                    vlen++;
                }
            }
            artval[vlen] = '\0';
            strcpy(id3.artist, artval);
            DPRINT("artval: ");
            DPRINTLN(artval);
        }
        else
        {
            done = true;
        }

        // done = true;
        start = start + fsize;
        if ((start == lastfrm + 10) || !isAlphaNumeric(tag[0]))
        {
            done = true;
        }
        else
        {
            lastfrm = start;
        }
    }
    return (id3);
}

s_id3 id3v2r30(File track, s_id3 id3)
{
    char *titval;
    char *albval;
    char *artval;
    uint32_t start = 10;
    uint32_t lastfrm = 10;
    bool done = false;

    while (!done)
    {
        DPRINTLN("\tin frameInfo loop v30");
        uint8_t buff[4];
        // uint32_t ltwo[4];
        char tag[5];
        uint32_t fsize;
        noInterrupts();
        track.seek(start);
        track.read((uint8_t *)tag, 4);
        tag[4] = 0;
        if (!strIsAlphaNumeric(tag))
        {
            DPRINTLN("non alpha numeric");
            track.seek(0);
            interrupts();
            return (id3);
        }
        track.seek(start + 4);
        track.read((uint8_t *)buff, 4);
        fsize = decodeSyncSafeSize(buff);
#ifdef DEBUG_PAVILLON
        DPRINT("syncsafe size=");
        DPRINTLN(decodeSyncSafeSize(buff));
#endif
        start += 10;
        if (skipTags(tag))
            ;
        else if (!strncmp(tag, ID3_TITLE, 4))
        {
            titval = decimalToUTF(track, start, fsize);
#ifdef UPPER_TITLE
            strcpy(id3.title, strToUpper(titval));
#else
            strcpy(id3.title, titval);
#endif
        }
        else if (tag[0] == 'T' && tag[1] == 'A' && tag[2] == 'L' && tag[3] == 'B')
        {
            albval = decimalToUTF(track, start, fsize);
            strcpy(id3.album, albval);
            DPRINT("albval: ");
            DPRINTLN(albval);
        }
        else if (tag[0] == 'T' && tag[1] == 'P' && tag[2] == 'E' && tag[3] == '1')
        {
            artval = decimalToUTF(track, start, fsize);
            strcpy(id3.artist, artval);
            DPRINT("artval: ");
            DPRINTLN(artval);
        }
        else
            done = true;
        start += fsize;
        if ((start == lastfrm + 10) || !isAlphaNumeric(tag[0]))
            done = true;
        else
            lastfrm = start;
    }
    return (id3);
}

// Fonction pour décoder la taille syncsafe
unsigned int decodeSyncSafeSize(const unsigned char syncsafeBytes[4]) {
    unsigned int size = 0;

    for (int i = 0; i < 4; ++i) {
        size = (size << 7) | (syncsafeBytes[i] & 0x7F);
    }

    return size;
}

char *strToUpper(char *str)
{
    char *tmp = str;

    while (*str)
    {
        *str = toUpperCase(*str);
        str++;
        // i++;
    }
    *str = 0;
    return (tmp);
}

int strIsAlphaNumeric(char *str)
{
    while (*str)
    {
        if (!isAlphaNumeric(*str))
            return (0);
        str++;
    }
    return (1);
}

int skipTags(char *tag)
{
    if (!strncmp(tag, ID3_YEAR, 4))
    {
        DPRINTLN("year found and skipped");
        return 1;
    }
    else if (!strncmp(tag, ID3_TDRC, 4))
    {
        DPRINTLN("recording year found and skipped");
        return 1;
    }
    else if (!strncmp(tag, ID3_TRACK, 4))
    {
        DPRINTLN("track found and skipped");
        return 1;
    }
    else if (!strncmp(tag, ID3_TCON, 4))
    {
        DPRINTLN("genre found and skipped");
        return 1;
    }
    else if (!strncmp(tag, ID3_COMM, 4))
    {
        DPRINTLN("comment found and skipped");
        return 1;
    }
    else if (!strncmp(tag, ID3_LENGTH, 4))
    {
        DPRINTLN("length found and skipped");
        return 1;
    }
    else if (!strncmp(tag, ID3_PICT, 4))
    {
        DPRINTLN("pict found and skipped");
        return 1;
    }
    return 0;
}