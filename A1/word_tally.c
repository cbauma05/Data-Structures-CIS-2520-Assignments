#include <stdio.h>
#include <stdlib.h> // for malloc(), free()
#include <string.h>

#include "word_extractor.h"
#include "word_tally.h"

// Forward declarations
static int updateWordInTallyList(LLNode **wordLists, int maxLen, char *word);

// Here we do all the work, processing the
// file and determining what to do for each word as we
// read it.  Once we are done reading, we can then
// examine our wordsToCheck list and see what their
// state is
int tallyWordsInFile(char *filename, LLNode **wordLists, int maxLen)
{
    struct WordExtractor *wordExtractor = NULL;
    char *aWord = NULL;
    // add any more variables that you need

    // Create the extractor and open the file
    wordExtractor = weCreateExtractor(filename, maxLen);

    if (wordExtractor == NULL) {
        fprintf(stderr, "Failed creating extractor for '%s'\n", filename);
        return 0;
    }

    // Ensure that all of the word list heads are NULL
    for (int i = 0; i < maxLen; i++) {
        wordLists[i] = NULL;
    }

    // Read each word from the file using the WordExtractor,
    // and for each tally how often it has been used in
    // the appropriate list

    // In the loop, figure out whether we have seen this
    // word before, and do the appropriate thing if we have

    int totalWordCount = 0;

    while (weHasMoreWords(wordExtractor)) {
        aWord = weGetNextWord(wordExtractor);
        totalWordCount++;

        // Figure out whether we have seen this word before,
        // and do the appropriate thing if we have
        updateWordInTallyList(wordLists, maxLen, aWord);
    }

    // Is there anything else you need to do at the end of the loop?
    printf("Total word count %d\n", totalWordCount);

    // Close the file when we are done
    weDeleteExtractor(wordExtractor);

    return 1;
}

// TODO: Either update the tally in the list, or add it to the list
static int updateWordInTallyList(LLNode **wordListHeads, int maxLen, char *word)
{
    LLNode *currentRefNode;
    int wordLength = strlen(word); // get word length of each wordlist passed in

    // Look up the word in the correct list to see
    // if we have already seen it

    currentRefNode = wordListHeads[wordLength];

    while (currentRefNode != NULL) {

        if (strcmp(currentRefNode->key, word) == 0) {  // Check if word has been seen, then increment tally
            currentRefNode->value++;
            return 1;
        }
        currentRefNode = currentRefNode->next;
    }

    // If the word is in the list, then update the tally
    // in the node and we are done, so return success

    LLNode *newRefNode = llNewNode(strdup(word), 1);   // Make a new subNode and create a new allocated place in memory for the word you are tallying

    // Otherwise, add it to the list
    // Add word to the head of a new list if it hasn't been seen

    newRefNode->next = wordListHeads[wordLength];   // Next node from subNode should point to the LLHead of the specific word length you are iterating through, then that should point back to the node being referenced
    wordListHeads[wordLength] = newRefNode;

    // Return success if no error
    return 1;
}