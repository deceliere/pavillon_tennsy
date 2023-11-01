#include <iostream>
#include <locale>
#include <string>

int testConvertUTF();
void GetUnicodeChar(unsigned int code, char chars[5]);
int main() {
    // Configurez la locale pour utiliser UTF-8
    // std::locale::global(std::locale("en_US.UTF-8"));

    // Affichez une chaîne de caractères UTF-8
    // testConvertUTF();
    char c[5];
    GetUnicodeChar(233, c);
    std::cout << c << std::endl;
    
    // std::cout << "Caractères accentués : éàçêö - CinÈma" << std::endl;

    return 0;
}


int testConvertUTF() {
    // Point de code Unicode décimal
    int unicodeCodePoint = 233; // Exemple : "é"
    char str[10] = "coucou";

    // Calcul des composants haut et bas
    int highSurrogate = 0xD800 + ((unicodeCodePoint - 0x10000) >> 10);
    int lowSurrogate = 0xDC00 + ((unicodeCodePoint - 0x10000) & 0x3FF);

    // Création de la chaîne de caractères UTF-16 à partir des composants
    // std::wstring utf16String;
    // utf16String += static_cast<wchar_t>(highSurrogate);
    // utf16String += static_cast<wchar_t>(lowSurrogate);

    // Affichage de la chaîne de caractères UTF-16
    std::wcout << highSurrogate << std::endl;
    std::wcout << lowSurrogate << std::endl;
    std::cout << str << std::endl;

    return 0;
}

void GetUnicodeChar(unsigned int code, char chars[5]) {
    if (code <= 0x7F) {
        chars[0] = (code & 0x7F); chars[1] = '\0';
    } else if (code <= 0x7FF) {
        // one continuation byte
        chars[1] = 0x80 | (code & 0x3F); code = (code >> 6);
        chars[0] = 0xC0 | (code & 0x1F); chars[2] = '\0';
    } else if (code <= 0xFFFF) {
        // two continuation bytes
        chars[2] = 0x80 | (code & 0x3F); code = (code >> 6);
        chars[1] = 0x80 | (code & 0x3F); code = (code >> 6);
        chars[0] = 0xE0 | (code & 0xF); chars[3] = '\0';
    } else if (code <= 0x10FFFF) {
        // three continuation bytes
        chars[3] = 0x80 | (code & 0x3F); code = (code >> 6);
        chars[2] = 0x80 | (code & 0x3F); code = (code >> 6);
        chars[1] = 0x80 | (code & 0x3F); code = (code >> 6);
        chars[0] = 0xF0 | (code & 0x7); chars[4] = '\0';
    } else {
        // unicode replacement character
        chars[2] = 0xEF; chars[1] = 0xBF; chars[0] = 0xBD;
        chars[3] = '\0';
    }
}