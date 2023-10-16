
#include <pavillon.h>

// int max_chars = 30;

void frameInfo(File track, char tvalue[64], char albval[64], char artval[64],
               char fname[30], bool showpic)
{
    uint32_t start = 10;
    uint32_t lastfrm = 10;
    bool done = false;

    fname = (char*) track.name();
    // strlcpy(fname, track.name(), max_chars);

    while (!done)
    {
        uint8_t buff[4];
        uint32_t ltwo[4];
        char tag[5];
        uint32_t fsize;
        track.seek(start);
        track.read((uint8_t *)tag, 4);
        tag[5] = 0;

        track.seek(start + 4);
        track.read((uint8_t *)buff, 4);
        ltwo[0] = buff[0];
        ltwo[1] = buff[1];
        ltwo[2] = buff[2];
        ltwo[3] = buff[3];

        fsize = (ltwo[0] << 24) | (ltwo[1] << 16) | (ltwo[2] << 8) | ltwo[3];
        start = start + 10;

        if (tag[0] == 'T' && tag[1] == 'I' && tag[2] == 'T' && tag[3] == '2')
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
        }
        else if (tag[0] == 'T' && tag[1] == 'A' && tag[2] == 'L' && tag[3] == 'B')
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
        }
        else if (tag[0] == 'T' && tag[1] == 'P' && tag[2] == 'E' && tag[3] == '1')
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
        }

        else if (tag[0] == 'A' && tag[1] == 'P' && tag[2] == 'I' && tag[3] == 'C')
        {
            uint32_t i;
            int vlen = 0;
            char val[30];
            char test[30];
            uint8_t raw[fsize - 11];

            track.seek(start);
            track.read((uint8_t *)val, 11);
            for (i = 0; i < 11; i++)
            {
                if (isAlphaNumeric(val[i]))
                {
                    test[vlen] = val[i];
                    vlen++;
                }
                else if (val[i] == '(' || val[i] == ')' || val[i] == ' ')
                {
                    test[vlen] = val[i];
                    vlen++;
                }
            }
            test[vlen] = '\0';

            if (fsize < 200000)
            {
                track.seek(start + 11);
                Serial.println(fsize + 11);
                // track.read((uint8_t *)pic, fsize-11);
            }
        }
        else
        {
        }

        // start = start + fsize;
        // if ((start == lastfrm + 10) || start > musicPlayer.mp3_ID3Jumper(track))
        // {
        //     done = true;
        // }
        // else
        // {
        //     lastfrm = start;
        // }
    }
    // track.rewind();
}
