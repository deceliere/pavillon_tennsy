#include <iostream>
#include <locale>

int main() {
    // Configurez la locale pour utiliser UTF-8
    // std::locale::global(std::locale("en_US.UTF-8"));

    // Affichez une chaîne de caractères UTF-8
    std::cout << "Caractères accentués : éàçêö - CinÈma" << std::endl;

    return 0;
}
