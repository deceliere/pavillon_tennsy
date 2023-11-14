#include "pavillon.h"

char *GetUnicodeChar(unsigned int code)
{
    char *chars = new char[5];
    if (code <= 0x7F)
    {
        chars[0] = (code & 0x7F);
        chars[1] = '\0';
    }
    else if (code <= 0x7FF)
    {
        // one continuation byte
        chars[1] = 0x80 | (code & 0x3F);
        code = (code >> 6);
        chars[0] = 0xC0 | (code & 0x1F);
        chars[2] = '\0';
    }
    else if (code <= 0xFFFF)
    {
        // two continuation bytes
        chars[2] = 0x80 | (code & 0x3F);
        code = (code >> 6);
        chars[1] = 0x80 | (code & 0x3F);
        code = (code >> 6);
        chars[0] = 0xE0 | (code & 0xF);
        chars[3] = '\0';
    }
    else if (code <= 0x10FFFF)
    {
        // three continuation bytes
        chars[3] = 0x80 | (code & 0x3F);
        code = (code >> 6);
        chars[2] = 0x80 | (code & 0x3F);
        code = (code >> 6);
        chars[1] = 0x80 | (code & 0x3F);
        code = (code >> 6);
        chars[0] = 0xF0 | (code & 0x7);
        chars[4] = '\0';
    }
    else
    {
        // unicode replacement character
        chars[2] = 0xEF;
        chars[1] = 0xBF;
        chars[0] = 0xBD;
        chars[3] = '\0';
    }
    return (chars);
}

char *decimalToUTF(File track, int start, u_int32_t fsize)
{
    uint32_t i;
    int vlen = 0;
    uint8_t val[fsize];
    uint8_t strStart = 0;
    char *frame = new char[1024];
    DPRINT("fsize=");
    DPRINTLN(fsize);

    track.seek(start);
    track.read((uint8_t *)val, fsize);
    if (val[0] && val[1] == 255 && val[2] == 254)
    {
        DPRINTLN("hello val++");
        strStart = 3;
    }

    for (u_int32_t i = strStart; i < fsize; i++)
    {
#ifdef DEBUG_PAVILLON
        printCharBit(val[i], i);
#endif
        if (val[i])
        {
            char *tmp;

            tmp = GetUnicodeChar((int)val[i]);
            DPRINT("tmp=");
            DPRINTLN(tmp);
            while (*tmp)
            {
                val[i] = *tmp;
                frame[vlen] = val[i];
                vlen++;
                tmp++;
            }
        }
    }
    frame[vlen] = 0;
    return(frame);
}