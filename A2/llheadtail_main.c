#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "fasta.h"
#include "LLvNode.h"

// void deleteFastaRecord(LLvNode *head, LLvNode *tail, void* data);

// void deleteRecord(LLvNode *node, void *data) {
// 	FASTArecord *fRecord1 = (FASTArecord*) node->value;
// 	fastaDeallocateRecord(fRecord1);
// }

int processFasta(char *filename, double *timeTaken)
{
	FILE *fp;
	FASTArecord fRecord;
	int lineNumber = 0, recordNumber = 0, status;
	int eofSeen = 0;
	clock_t startTime, endTime;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "Failure opening %s : %s\n",
				filename, strerror(errno));
		return -1;
	}

	LLvNode *head = NULL;   // declare head of the list
	LLvNode *tail = NULL;   // declare tail of the list

	/** record the time now, before we do the work */
	startTime = clock();

	do {
		/** print a '.' every 10,000 records so
		* we know something is happening */
		if ((recordNumber % 10000) == 0) {
			printf(".");
			fflush(stdout);
		}

		fastaInitializeRecord(&fRecord); // initialize fRecord

		status = fastaReadRecord(fp, &fRecord);
		if (status == 0) {
			eofSeen = 1;

		} else if (status > 0) {
			lineNumber += status;
			recordNumber++;

			LLvNode *newNode = llNewNode(NULL, &fRecord); // create new node for each record

			if (head == NULL) {  // if head is null, then the list is empty
				head = newNode;  // declare head and tail to same memory address 
				tail = newNode;
			} else {
				tail->next = newNode; // if head isn't null, then the list is not empty
				tail = tail->next;   // point tail's next to new node then redeclare tail to keep a reference point of most recent node
			}                        // this way we have a reference point to the most recent node when we want to add a new node or add data rather them chasing through the entire linked list.

			//fastaDeallocateRecord(&fRecord); // deallocate memory for fRecord

			//fastaPrintRecord(stdout, &fRecord);
			//fRecord.description = NULL;
			fastaClearRecord(&fRecord); // clear FASTA record

		} else {
			fprintf(stderr, "status = %d\n", status);
			fprintf(stderr, "Error: failure at line %d of '%s'\n",
					lineNumber, filename);
			return -1;
		}

	} while ( ! eofSeen);
	printf(" %d FASTA records\n", recordNumber);

	/** record the time now, when the work is done,
	 *  and calculate the difference*/
	endTime = clock();

	(*timeTaken) = ((double) (endTime - startTime)) / CLOCKS_PER_SEC;


	fclose(fp);

	//free(&fRecord); // Free memory allocated for fRecord after file pointer is closed

	LLvNode *next;  // used functionality of llFree to free memory allocated in linked list

	for ( ; head != NULL; head = next) {

		/** hang on to the next pointer */
		next = head->next;
		
		/** free the list node itself */
		free(head);
	}

	//llFree(head, deleteRecord, NULL); // Free memory allocated in linked list
	//llFree(tail, deleteRecord, NULL); // Free memory allocated in linked list


	return recordNumber;
}


int processFastaRepeatedly(
		char *filename,
		long repeatsRequested
	)
{
	double timeThisIterationInSeconds;
	double totalTimeInSeconds = 0;
	int minutesPortion;
	int status;
	long i;

	for (i = 0; i < repeatsRequested; i++) {
		status = processFasta(filename, &timeThisIterationInSeconds);
		if (status < 0)	return -1;
		totalTimeInSeconds += timeThisIterationInSeconds;
	}

	printf("%lf seconds taken for processing total\n", totalTimeInSeconds);

	totalTimeInSeconds /= (double) repeatsRequested;

	minutesPortion = (int) (totalTimeInSeconds / 60);
	totalTimeInSeconds = totalTimeInSeconds - (60 * minutesPortion);
	printf("On average: %d minutes, %lf second per run\n",
            minutesPortion, totalTimeInSeconds);

	return status;
}

void usage(char *progname)
{
	fprintf(stderr, "%s [<OPTIONS>] <file> [ <file> ...]\n", progname);
	fprintf(stderr, "\n");
	fprintf(stderr, "Prints timing of loading and storing FASTA records.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Options: \n");
	fprintf(stderr, "-R <REPEATS> : Number of times to repeat load.\n");
	fprintf(stderr, "             : Time reported will be average time.\n");
	fprintf(stderr, "\n");
}

/**
 * Program mainline
 */
int main(int argc, char **argv)
{
	int i, recordsProcessed = 0;
	long repeatsRequested = 1;

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (argv[i][1] == 'R') {
				if (i >= argc) {
					fprintf(stderr,
							"Error: need argument for repeats requested\n");
					return 1;
				}
				if (sscanf(argv[++i], "%ld", &repeatsRequested) != 1) {
					fprintf(stderr,
							"Error: cannot parse repeats requested from '%s'\n",
							argv[i]);
					return 1;
				}
			} else {
				fprintf(stderr,
						"Error: unknown option '%s'\n", argv[i]);
				usage(argv[0]);
			}
		} else {
			recordsProcessed = processFastaRepeatedly(argv[i],
					repeatsRequested);
			if (recordsProcessed < 0) {
				fprintf(stderr, "Error: Processing '%s' failed -- exitting\n",
						argv[i]);
				return 1;
			}
			printf("%d records processed from '%s'\n",
					recordsProcessed, argv[i]);
		}
	}

	if ( recordsProcessed == 0 ) {
		fprintf(stderr,
				"No data processed -- provide the name of"
				" a file on the command line\n");
		usage(argv[0]);
		return 1;
	}

	return 0;
}

