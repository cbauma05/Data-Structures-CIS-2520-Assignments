
#ifndef	__WORD_TALLY_HEADER__
#define	__WORD_TALLY_HEADER__

#include "LLNode.h"


/**
 * count all the words in a file, separating by length
 */
int tallyWordsInFile(char *filename, LLNode **wordLists, int maxLen);

#endif
