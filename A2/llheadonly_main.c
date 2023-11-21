#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "fasta.h"
#include "LLvNode.h"

// FASTArecord * LLNewNode(char *key, void *value) {

// 	FASTArecord *newp = (FASTArecord *) malloc(sizeof(FASTArecord));

// 	if (newp == NULL) {
// 		fprintf(stderr, "ERROR: Memory Allocation failed.\n");
// 		exit(1);
// 	}
// 
// 	/* assign data within new node */
// 	newp->description = key;
// 	//newp->sequence = value;  // is sequence declaration required?

// 	/* make sure we point at nothing */
// 	newp->next = NULL;

// 	return newp;
// }

// FASTArecord * LLAppend(FASTArecord *listp, FASTArecord *newp) {

// 	FASTArecord *p;

// 	if (listp == NULL)
// 		return newp;

// 	for (p = listp; p->next; p = p->next)
// 		;

// 	p->next = newp;
// 	return listp;
// }

// function to free memory allocated in linked list
// void freeLinkedList(FASTArecord *head){
// 	FASTArecord *temp;
// 	while (head != NULL){
// 		temp = head;
// 		head = head->next;
// 		free(temp);
// 	}
// }

void deleteFastaRecord(LLvNode *head, void* data){  // funtion to call fastaDeallocateRecord and free FASTArecord in each iteration of data in linked list
	FASTArecord *fRecord = (FASTArecord *) head->value;
	fastaDeallocateRecord(fRecord);

	FASTArecord *fRecord2 = (FASTArecord *) head->key;
	free(fRecord2);
}

int processFasta(char *filename, double *timeTaken) 
{
	FILE *fp;
	                      // data is read in but not stored
	FASTArecord *fRecord;
	int lineNumber = 0, recordNumber = 0, status;
	int eofSeen = 0;
	clock_t startTime, endTime;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "Failure opening %s : %s\n",
				filename, strerror(errno));
		return -1;
	}

	LLvNode *head = NULL;   // declare head of the list using LLvNode data structure


	//FASTArecord *head = malloc(sizeof(FASTArecord));   
	
	//check if first node is null
	// if (head == NULL) {
	// 	fprintf(stderr, "ERROR: Memory Allocation failed.\n");
	// 	exit(1);
	// }
    // head = LLNewNode(strdup(fRecord.description), NULL);


	/** record the time now, before we do the work */
	startTime = clock();

	do {
		/** print a '.' every 10,000 records so
		* we know something is happening */
		if ((recordNumber % 10000) == 0) {
			printf(".");
			fflush(stdout);
		}

		// fastaInitializeRecord(&fRecord);
		fRecord = fastaAllocateRecord();      // call funciton to allocate and initialize a new FASTA record to NULL for each fRecord loaded in

		status = fastaReadRecord(fp, fRecord);
		if (status == 0) {
			eofSeen = 1;

		} else if (status > 0) {
			lineNumber += status;
			recordNumber++;


		//FASTArecord *newNode = LLNewNode(strdup(fRecord.description), NULL); // old funciton call N/A

		LLvNode *newNode = llNewNode(strdup(fRecord->description), fRecord); // Create new node using llNewNode function and use void* to pass in FASTArecord as value

		head = llAppend(head, newNode); // Append the new node to the end of the list

			///fastaPrintRecord(stdout, &fRecord);
			//fastaClearRecord(fRecord);

			//fastaDeallocateRecord(fRecord); 

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
	
	free(fRecord); // Free memory allocated for fRecord after file pointer is closed

	llFree(head, deleteFastaRecord, NULL); // Free memory allocated in linked list

	return recordNumber;
}


int processFastaRepeatedly(char *filename, long repeatsRequested) // figure out how to reference head in main
{
	double timeThisIterationInSeconds;
	double totalTimeInSeconds = 0;
	int minutesPortion;
	int status;
	long i;

	for (i = 0; i < repeatsRequested; i++) {
		status = processFasta(filename, &timeThisIterationInSeconds);  // figure out how to reference head in main
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
			recordsProcessed = processFastaRepeatedly(argv[i], repeatsRequested);
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

