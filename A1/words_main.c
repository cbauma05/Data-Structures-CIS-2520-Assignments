#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "word_extractor.h"

static int printWordsInFile(char *filename, int maxLen, int printLength)
{
	struct WordExtractor *wordExtractor = NULL;
	char *aWord = NULL;

	// create the extractor and open the file
	wordExtractor = weCreateExtractor(filename, maxLen);

	if (wordExtractor == NULL) {
		fprintf(stderr, "Failed creating extractor for '%s'\n", filename);
		return 0;
	}

	// read each word from the file using the WordExtractor,
	// and print it out

	while (weHasMoreWords(wordExtractor)) {
		aWord = weGetNextWord(wordExtractor);
		if (printLength) {
			printf(" GOT '%s' len %ld\n", aWord, strlen(aWord));
		} else {
			printf(" GOT '%s'\n", aWord);
		}
	}

	// Close the file when we are done
	weDeleteExtractor(wordExtractor);

	return 1;
}


/**
 * Program mainline
 */

// define the maximum length of word we will look for, and by extension,
// the number of entries that must be in the array of word lists
#define	MAX_WORD_LEN	24

int main(int argc, char **argv)
{
	int i, printLength = 0, didProcessing = 0;

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (argv[i][1] == 'l') {
				printLength = 1;
			}
		} else {
			if (printWordsInFile(argv[i], MAX_WORD_LEN, printLength) == 0) {
				fprintf(stderr, "Error: Processing '%s' failed -- exitting\n",
						argv[i]);
				return 1;
			}
			didProcessing = 1;
		}
	}

	if ( ! didProcessing ) {
		fprintf(stderr, "No data processed -- provide the name of"
				" a file on the command line\n");
		return 1;
	}

	return 0;
}
