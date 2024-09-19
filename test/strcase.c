/*** strcase.c ***/

#include <locale.h>
#include <stdlib.h>
#include <string.h>

#include <unicode/ustdio.h>
#include <unicode/ustring.h>

#define BUFSZ 1024

/* wrapper function for u_strToTitle with signature
 * matching the other casing functions */
int32_t title(UChar *dest, int32_t destCapacity,
		const UChar *src, int32_t srcLength,
		const char *locale, UErrorCode *pErrorCode)
{
	return u_strToTitle(dest, destCapacity, src,
			srcLength, NULL, locale, pErrorCode);
}

int main(int argc, char **argv)
{
	UFILE *in;
	char *locale;
	UChar line[BUFSZ], cased[BUFSZ];
	UErrorCode status = U_ZERO_ERROR;
	int32_t (*op)(
			UChar*, int32_t, const UChar*, int32_t,
			const char*, UErrorCode*
		) = NULL;

	/* casing is locale-dependent */
	if (!(locale = setlocale(LC_CTYPE, "")))
	{
		fputs("Cannot determine system locale\n", stderr);
		return EXIT_FAILURE;
	}

	if (argc < 2 || strcmp(argv[1], "upper") == 0)
		op = u_strToUpper;
	else if (strcmp(argv[1], "lower") == 0)
		op = u_strToLower;
	else if (strcmp(argv[1], "title") == 0)
		op = title;
	else
	{
		fprintf(stderr, "Unrecognized case: %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	if (!(in = u_finit(stdin, NULL, NULL)))
	{
		fputs("Error opening stdin as UFILE\n", stderr);
		return EXIT_FAILURE;
	}

	/* Ideally we should change case up to the last word
	 * break and push the remaining characters back for
	 * a future read if the line was longer than BUFSZ.
	 * Currently, if the string is truncated, the final
	 * character would incorrectly be considered
	 * terminal, which affects casing rules in Greek. */
	while (u_fgets(line, BUFSZ, in))
	{
		op(cased, BUFSZ, line, -1, locale, &status);
		/* if casing increases string length, and goes
		 * beyond buffer size like the german ß -> SS */
		if (status == U_BUFFER_OVERFLOW_ERROR)
		{
			/* Just issue a warning and read another line.
			 * Don't treat it as severely as other errors. */
			fputs("Line too long\n", stderr);
			status = U_ZERO_ERROR;
		}
		else if (U_FAILURE(status))
		{
			fputs(u_errorName(status), stderr);
			break;
		}
		else
			u_printf("%S", cased);
	}

	u_fclose(in);
	return U_SUCCESS(status)
		? EXIT_SUCCESS : EXIT_FAILURE;
}