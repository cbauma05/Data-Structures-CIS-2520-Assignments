#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "fasta.h"

void doubleArraySize(FASTArecord **array, int *arraySize) // Uses pointers to modify array size and the instance of FASTArecord to modify data outside the function and double the array size
{
	// FASTArecord *newArray;
	// int newSize = (*arraySize) * 2;

	*arraySize *= 2;
	*array = realloc(*array, (*arraySize) * sizeof(FASTArecord));

	if (*array == NULL) {
		fprintf(stderr, "ERROR: Memory Allocation failed.\n");
		exit(1);
	}
}


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


	int initialArraySize = 1000;    // Initial size of the array
	//int *size = &initialArraySize;                                               
	FASTArecord *dynamicArray = malloc(initialArraySize * sizeof(FASTArecord));  // Allocate memory for the array
	if (dynamicArray == NULL) {                                              // Check if memory allocation failed
		fprintf(stderr, "ERROR: Memory Allocation failed.\n");
		exit(1);
	}


	/** record the time now, before we do the work */
	startTime = clock();

	//size_t allocatedMemoryTotal = (*size) * sizeof(FASTArecord); // Calculate total allocated memory
	//size_t memoryUsed = recordNumber * sizeof(FASTArecord); // Calculate memory used
	//size_t wastedMemory = allocatedMemoryTotal - memoryUsed; // Calculate wasted space


	 // Calculate wasted memory (in bytes

	//double *wastedMemoryPercentage = (double *)malloc(sizeof(double)); // Allocate memory for wasted memory percentage variable
	
	// if (wastedMemoryPercentage == NULL) {
    // 	fprintf(stderr, "Error: Memory allocation failed\n");
    // 	exit(1);
	// }

	//*wastedMemoryPercentage = (double)(wastedMemory / allocatedMemoryTotal) * 100; // Calculate wasted memory percentage


	do {
		/** print a '.' every 10,000 records so
		* we know something is happening */
		if ((recordNumber % 10000) == 0) {
			printf(".");
			fflush(stdout);
		}

		fastaInitializeRecord(&fRecord);

		status = fastaReadRecord(fp, &fRecord);
		if (status == 0) {
			eofSeen = 1;

		} else if (status > 0) {
			lineNumber += status;
			recordNumber++;

		if (recordNumber == (initialArraySize - 1)) {   // Check if the array is full
			doubleArraySize(&dynamicArray, &initialArraySize); // Double the array size
		}
		dynamicArray[recordNumber - 1] = fRecord; // Store parsed record into larger array


			//fastaPrintRecord(stdout, &fRecord);
		} else {
			fprintf(stderr, "status = %d\n", status);
			fprintf(stderr, "Error: failure at line %d of '%s'\n",
					lineNumber, filename);
			free(dynamicArray);   // even if there is an error free allocated memory
			return -1;
		}

	} while ( ! eofSeen);

	// size_t singlerecord = sizeof(FASTArecord);  used for printing single record size for tests
	size_t memoryUsed = initialArraySize * sizeof(FASTArecord); // Calculate memory used
	float wastedMemory = (1 - ((float)recordNumber / initialArraySize)) * 100;

	//printf("------single record size: %zu------------\n", singlerecord);
	printf(" %d FASTA records -- %zu allocated (%.3f%% waste)\n",recordNumber, memoryUsed, wastedMemory);  // Print total allocated memory and wasted space
	//printf(" %d FASTA records -- %zu allocated (%.3f%% waste)\n", recordNumber, allocatedMemoryTotal, *wastedMemoryPercentage); 

	/** record the time now, when the work is done,
	 *  and calculate the difference*/
	endTime = clock();

	(*timeTaken) = ((double) (endTime - startTime)) / CLOCKS_PER_SEC;

	fclose(fp);

	// free memory outside of iteration loop
	for (int i = 0; i < recordNumber; i++) {
		fastaClearRecord(&dynamicArray[i]);
	}

	free(dynamicArray);  // Free memory used in dynamic array
	//free(wastedMemoryPercentage); // Free memory used in wasted memory variable

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
