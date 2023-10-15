#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Fonction de comparaison personnalisée
int compare(const void *a, const void *b) {
    // Extrait les numéros à partir des chaînes de caractères
    int num1, num2;
    sscanf(*(const char**)a, "%d-", &num1);
    sscanf(*(const char**)b, "%d-", &num2);
    
    return num1 - num2;
}

int main() {
    char *arr[] = {"132-hello.wav", "2-coucou.wav", "3-party.wav", "10-goodbye.wav", "5-bye.wav"};
    int n = sizeof(arr) / sizeof(arr[0]);

    qsort(arr, n, sizeof(char*), compare);

    for (int i = 0; i < n; i++) {
        printf("%s\n", arr[i]);
    }

    return 0;
}