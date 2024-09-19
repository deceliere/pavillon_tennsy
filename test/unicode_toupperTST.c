#include <stdio.h>
#include <unistd.h>
#include <unicode/ustdio.h>
#include <unicode/ustring.h>

int main() {
    // printf("élo\n");
    UChar input[] = u"hélo’"; // Votre caractère Unicode ici
    UChar output[6];
    UErrorCode status = U_ZERO_ERROR;

    u_printf("input =%S\n", input);
    
    // u_strToUpper(output, 2, input, -1, "fr", NULL);
    u_strToUpper(output, 5, input, -1, "fr", &status);

    // char hello[] = "coucou\n";
    // write(1, hello, 7);
    // write(1, (char*) input, 2);
    // printf("Conversion en majuscules : %s\n", (char*) output);
    u_printf("output =%S\n", output);


    return 0;
}

//  gcc unicode_toupperTST.c -I/usr/local/opt/icu4c/include -L/usr/local/opt/icu4c/lib -licuuc -licuio
//  gcc strcase.c -I/usr/local/opt/icu4c/include -L/usr/local/opt/icu4c/lib -licuuc -licuio