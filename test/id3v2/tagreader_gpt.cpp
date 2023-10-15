// #include <iostream>
#include <iomanip>
// #include <stdio.h>


// #include <iostream>
#include <taglib/taglib.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/tpropertymap.h>
#include <taglib/tag.h>
#include <taglib/fileref.h>


using namespace std;

int main() {
    // Chemin vers le fichier MP3
    const char* filePath = "/Users/r/Desktop/mud/Nous sommes la pluie sur le sol nu.mp3";

    try {
        // Ouvrir le fichier MP3
        TagLib::FileRef f(filePath);
        TagLib::MPEG::File file(filePath);

        // Vérifier si le fichier est valide
        if (!file.isValid()) {
            std::cerr << "Erreur : fichier MP3 invalide." << std::endl;
            return 1;
        }

        // Obtenir les tags ID3v2
        TagLib::Tag *tag = f.tag();

        // Vérifier si les tags existent
        if (!f.isNull() && f.tag()) {
            // Lire et afficher les informations des tags
            std::cout << "Titre : " << tag->title().toCString(true) << std::endl;
            std::cout << "Artiste : " << tag->artist().toCString(true) << std::endl;
            std::cout << "Album : " << tag->album().toCString(true) << std::endl;
            // Vous pouvez continuer avec d'autres informations de tag

            TagLib::PropertyMap tags = f.file()->properties();


            if(!f.isNull() && f.audioProperties()) {

              TagLib::AudioProperties *properties = f.audioProperties();

              int seconds = properties->length() % 60;
              int minutes = (properties->length() - seconds) / 60;

              cout << "-- AUDIO --" << endl;
              cout << "bitrate     - " << properties->bitrate() << endl;
              cout << "sample rate - " << properties->sampleRate() << endl;
              cout << "channels    - " << properties->channels() << endl;
              cout << "length      - " << minutes << ":" << setfill('0') << setw(2) << seconds << endl;
              cout << "length(sec) - " << properties->length() << endl;
            }
            return 0;

        } else {
            std::cerr << "Aucun tag ID3v2 trouvé dans le fichier." << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Erreur : " << e.what() << std::endl;
        return 1;
    }
}
