#include "pavillon.h"
#include <iostream>
#include <locale> 

void	ft_bzero(unsigned char *s, size_t n)
{
	unsigned char	*txt;
	int				a;
	int				b;

	txt = s;
	a = 0;
	b = n;
	while (a < b)
	{
		txt[a] = 0;
		a++;
	}
}

int isAlnumUTF(char c) {
    // std::wstring c = 'é';  // L'é est une chaîne de caractères larges (wide string)

    // Configure la locale pour traiter les caractères Unicode
    std::locale::global(std::locale(""));

    if (std::iswalnum(c))
		return 1;
    return 0;
}

void printCharBit(char c, int i)
{
    DPRINT("char[");
    DPRINT(i);
    DPRINT("]=");
    DPRINT((char)c);
    DPRINT("\t int=");
    DPRINT((int)c);
    DPRINT("\t");
    for (int x = 0; x < 8; x++)
    {
        if (c & 1)
        {
            DPRINT("1");
        }
        else
            DPRINT("0");
        c >>= 1;
    }
    DPRINTLN("");
}