#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argv, char **argc)
{

    FILE *fichier; // Déclarer un pointeur vers un fichier

    fichier = fopen(argc[1], "r"); // Ouvrir le fichier en mode lecture ("r")

    if (fichier != NULL)
    {
        printf("Le fichier a été ouvert avec succès.\n");

        char ligne[100]; // Un tampon pour stocker chaque ligne lue

        int32_t size;

        fgets(ligne, sizeof(ligne), fichier);
        for(int i = 0; i <= 2; i++)
            printf("%c", ligne[i]);
        printf("\nversion: ");
        printf("%c", ligne[3] + '0');
        printf("%c\n", ligne[4] + '0');
        printf("frame:");
        printf("%c", ligne[10]);
        printf("%c", ligne[11]);
        printf("%c", ligne[12]);
        // printf("%c\n", ligne[13]);
        printf("frame size:");
        // size = ligne[14] << 24 | ligne[15] << 16 | ligne[16] << 8 | ligne[17];
        size = ligne[13] << 16 | ligne[14] << 8 | ligne[15];
        printf("%i\n",size);
        // printf("%i%i%i%i", ligne[13], ligne[14], ligne[15], ligne[16]);
        // for (int i = 14; i <= 22; i++)
            // printf("%c", ligne[i] + '0');

        // char version[2] = "";
        // strncat(version, &ligne[3], 2);
        // printf("%s\n", ligne);
        //     fgets(ligne, sizeof(ligne), fichier);
        // printf("%s\n", ligne);
        //     fgets(ligne, sizeof(ligne), fichier);
        // printf("%s\n", ligne);

        fclose(fichier); // Fermer le fichier lorsque vous avez terminé
    }
    else
    {
        fprintf(stderr, "Impossible d'ouvrir le fichier.\n");
    }

    return 0;
}
