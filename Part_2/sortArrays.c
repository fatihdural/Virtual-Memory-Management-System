#include <stdio.h> 
#include <stdlib.h> 
#include <time.h>  											// libraries
#include <math.h>
#include <unistd.h> 
#include <pthread.h> 
#include <string.h>

void set(unsigned int index, int value, char * tName); 		// set-get functions
int get(unsigned int index, char * tName);

void bubbleSort(int arr[], int n); 							// sorting algorithms
void quickSort(int arr[], int n);
void mergeSort(int arr[], int low, int high);
void mergeSubArrays(int arr[], int low, int middle, int high);
void indexSort(int arr[], int n);

void resetRefBits(); 									// reset reference bits some algorithms

void printAnArray(int arr[], int lengthArr); 			// print functions
void printPageTable();
void printStatics();

void * fillThreadFun(void * var);
void * bubbleThreadFun(void * var); 						// sorting thread that called sorting, get, set functions
void * quickThreadFun(void * var);
void * mergeThreadFun(void * var);
void * indexThreadFun(void * var);
void * checkThreadFun(void * var);

int pageReplace(int index, char * sortAlgo); 			// page replace function

struct pageEntry{ 										// page entry 
	int targetMemoryFrame;
	int prAbBit;
	int modifyBit;
	int refBit;
};
struct pageEntry * pageTable;							// page table that holds page entries

struct statics{  										// statics struct
	int numReads;
	int numWrites;
	int numPageMiss;
	int numPageReplace;
	int numDiskPageWrites;
	int numDiskPageReads;
};
struct statics fillStatic, bubbleStatic, quickStatic, mergeStatic, indexStatic, checkStatic; 	// static variables

int timerReset = 0; 									// some need global variables

int * physicalMemory; 										
int frameSize, memoryFrames, virtualFrames;
int fifoBubble = 0, fifoQucik = 0, fifoMerge = 0, fifoIndex = 0, fifoCheck = 0;
int lruBubble = 0, lruQuick = 0, lruMerge = 0, lruIndex = 0, lruCheck = 0;
int scBubble = 0, scQuick = 0, scMerge = 0, scIndex = 0, scCheck = 0;
int wsBubble = 0, wsQuick = 0, wsMerge = 0, wsIndex = 0, wsCheck = 0;

char allocPoli;
char replaceAlgorithm;

int memoryAccessCount = 0;
int memoryAccessLimit;

const char * globalDiskName;
FILE *fp;

pthread_mutex_t lock; 										// mutex for threads

int main(int argc, char const *argv[])
{
	if( argc != 8 ){
		printf("Wrong number of parameters!!!\n"); 			// the number of arguments is fixed, it must be 8
		return 0;
	}

	char frameSizeBitsStr[strlen(argv[1])];
	strcpy(frameSizeBitsStr, argv[1]);
	for(int i = 0; i < strlen(frameSizeBitsStr); i++ ){
		if( ! (frameSizeBitsStr[i] > 47 && frameSizeBitsStr[i] < 58) ){
			printf("ARGUMENT ERROR!!!! PLEASE CHECK COMMANDS!!\n");
			return 0;
		}
	}

	char memoryFramesBitsStr[strlen(argv[2])];
	strcpy(memoryFramesBitsStr, argv[2]);
	for(int i = 0; i < strlen(memoryFramesBitsStr); i++ ){
		if( ! (memoryFramesBitsStr[i] > 47 && memoryFramesBitsStr[i] < 58) ){
			printf("ARGUMENT ERROR!!!! PLEASE CHECK COMMANDS!!\n");
			return 0;
		}
	}	

	char virtualFramesStr[strlen(argv[3])];
	strcpy(virtualFramesStr, argv[3]);
	for(int i = 0; i < strlen(virtualFramesStr); i++ ){
		if( ! (virtualFramesStr[i] > 47 && virtualFramesStr[i] < 58) ){
			printf("ARGUMENT ERROR!!!! PLEASE CHECK COMMANDS!!\n");
			return 0;
		}
	}	

	int frameSizeBits = atoi(argv[1]); 						// get arguments, if casting to integer is needed, use atoi
	int memoryFramesBits = atoi(argv[2]);
	int virtualFramesBits = atoi(argv[3]);

	const char * replaceAlgo = argv[4]; 					// page replace algorithms
	if(strcmp(replaceAlgo, "NRU") == 0){
		replaceAlgorithm = 'N';
	}
	else if( strcmp(replaceAlgo, "FIFO") == 0 ){
		replaceAlgorithm = 'F';
	}
	else if( strcmp(replaceAlgo, "SC") == 0 ){
		replaceAlgorithm = 'S';
	}
	else if( strcmp(replaceAlgo, "LRU") == 0 ){
		replaceAlgorithm = 'L';
	}
	else if( strcmp(replaceAlgo, "WSClock") == 0 ){
		replaceAlgorithm = 'W';
	}
	else{
		printf("Wrong page replacement algorithm!!!\n");
		return 0;
	}

	const char * allocPolicy = argv[5];
	if( strcmp(allocPolicy, "local") == 0){ 				// allocation policy
		allocPoli = 'L';
	}
	else if( strcmp(allocPolicy, "global") == 0 ){
		allocPoli = 'G';
	}
	else{
		printf("Wrong allocation policy!!! You can enter 'local' or 'global'\n");
		return 0;
	}


	char screenLimitStr[strlen(argv[6])];
	strcpy(screenLimitStr, argv[6]);
	for(int i = 0; i < strlen(screenLimitStr); i++ ){
		if( ! (screenLimitStr[i] > 47 && screenLimitStr[i] < 58) ){
			printf("ARGUMENT ERROR!!!! PLEASE CHECK COMMANDS!!\n");
			return 0;
		}
	}	
	
	int screenLimit = atoi(argv[6]); 						
	memoryAccessLimit = screenLimit;
	if( frameSizeBits <= 0 || memoryFramesBits <= 0 || virtualFramesBits <= 0 || screenLimit <= 0 ){ 		// numbers must bigger than 0
		printf("%s\n", "PARAMETERS MUST BIGGER THAN 0!!!!");
		return 0;
	}

	const char * diskName = argv[7]; 						// disk name
	globalDiskName = diskName;
	fp = fopen(diskName, "w+");
	if( fp == NULL){
		perror("Error!");
		return 0;
	}

	fillStatic.numReads = 0; 								// initialize of fill statics
	fillStatic.numWrites = 0;
	fillStatic.numPageMiss = 0;
	fillStatic.numPageReplace = 0;
	fillStatic.numDiskPageWrites = 0;
	fillStatic.numDiskPageReads = 0;

	bubbleStatic.numReads = 0; 								// initialize of bubble statics
	bubbleStatic.numWrites = 0;
	bubbleStatic.numPageMiss = 0;
	bubbleStatic.numPageReplace = 0;
	bubbleStatic.numDiskPageWrites = 0;
	bubbleStatic.numDiskPageReads = 0;

	quickStatic.numReads = 0; 								// initialize of qucik statics
	quickStatic.numWrites = 0;
	quickStatic.numPageMiss = 0;
	quickStatic.numPageReplace = 0;
	quickStatic.numDiskPageWrites = 0;
	quickStatic.numDiskPageReads = 0;

	mergeStatic.numReads = 0; 								// initialize of merge statics
	mergeStatic.numWrites = 0;
	mergeStatic.numPageMiss = 0;
	mergeStatic.numPageReplace = 0;
	mergeStatic.numDiskPageWrites = 0;
	mergeStatic.numDiskPageReads = 0;

	indexStatic.numReads = 0; 								// initialize of index statics
	indexStatic.numWrites = 0;
	indexStatic.numPageMiss = 0;
	indexStatic.numPageReplace = 0;
	indexStatic.numDiskPageWrites = 0;
	indexStatic.numDiskPageReads = 0;

	checkStatic.numReads = 0; 								// initialize of chech statics
	checkStatic.numWrites = 0;
	checkStatic.numPageMiss = 0;
	checkStatic.numPageReplace = 0;
	checkStatic.numDiskPageWrites = 0;
	checkStatic.numDiskPageReads = 0;

	frameSize = (int) pow(2, frameSizeBits);  				// power of 2 gives frames sizes
	memoryFrames = (int) pow(2, memoryFramesBits);
	virtualFrames = (int) pow(2, virtualFramesBits);

	int totalMemoryIntegers = frameSize * memoryFrames; 	// memory has frame * memoryframes integers
	physicalMemory = (int*) malloc(sizeof(int) * totalMemoryIntegers);
	for(int i = 0; i < totalMemoryIntegers; i++){
		physicalMemory[i] = 0; 								// initialize memory by zero
	}

	printf("%s\n", "Please Wait......");

	int totalVirtualIntegers = frameSize * virtualFrames; 		

	if( virtualFrames < memoryFrames ){ 					// if virtual frames is lower than memory frames
		pageTable = (struct pageEntry *) malloc(sizeof(struct pageEntry) * memoryFrames); 	
		virtualFrames = memoryFrames; 						// virtual frames variable becomes memory frames to avoid problems with the algorithm
		totalVirtualIntegers = frameSize * virtualFrames;
	}
	else{
		pageTable = (struct pageEntry *) malloc(sizeof(struct pageEntry) * virtualFrames); 		// page elements are changing according to virtual frames size
	}

	pthread_t thread_idFill; 
    pthread_create(&thread_idFill, NULL, fillThreadFun, (void *)&totalVirtualIntegers); 	// fill thread
    pthread_join(thread_idFill, NULL);

	for(int i = 0; i < virtualFrames; i++){ 			// page table initialize.
		struct pageEntry ex;
		ex.targetMemoryFrame = 0;
		ex.prAbBit = 0;	
		ex.modifyBit = 0;
		ex.refBit = 0;
		pageTable[i] = ex;
	}

	if( allocPoli == 'L' ){ 						// local policy
		if( memoryFrames < 4){
			printf("%s\n", "Memory must have at least 4 frames!!!(We have 4 sorting algorithm)"); 
			return 0;
		}
		for(int i = 0; i < memoryFrames; i++){ 					// this loop create initial page table
			int whichSortPart = i / (int) (memoryFrames / 4); 
			int temp;
			if(whichSortPart == 0){ 							// some needs operations are done
				temp = i;
			}
			else{
				temp = i % whichSortPart;
			}
			int whichPage = ((virtualFrames / 4) * (whichSortPart))  + temp;
			pageTable[whichPage].prAbBit = 1; 					// entire page table entry which one is correct
			pageTable[whichPage].targetMemoryFrame = i;

			for(int j = 0; j < frameSize; j++){ 				// virtual disk to physical memory
				int value;
				fseek(fp, sizeof(int) * ((whichPage * frameSize) + j), SEEK_SET);
				fread(&value, sizeof(int), 1, fp);
				physicalMemory[i*frameSize + j] = value;
			}

			if( i < memoryFrames / 4 ){ 						// initial page table statics
				bubbleStatic.numPageMiss = bubbleStatic.numPageMiss + 1;
				bubbleStatic.numDiskPageReads = bubbleStatic.numDiskPageReads + 1;
				bubbleStatic.numPageMiss = bubbleStatic.numPageMiss + 1; 
				bubbleStatic.numWrites = bubbleStatic.numWrites + frameSize;
			}
			else if( i < memoryFrames / 2 ){
				quickStatic.numPageMiss = quickStatic.numPageMiss + 1;
				quickStatic.numDiskPageReads = quickStatic.numDiskPageReads + 1;
				quickStatic.numPageMiss = quickStatic.numPageMiss + 1; 
				quickStatic.numWrites = quickStatic.numWrites + frameSize;
			}
			else if( i < (memoryFrames / 4) * 3){
				mergeStatic.numPageMiss = mergeStatic.numPageMiss + 1;
				mergeStatic.numDiskPageReads = mergeStatic.numDiskPageReads + 1;
				mergeStatic.numPageMiss = mergeStatic.numPageMiss + 1; 
				mergeStatic.numWrites = mergeStatic.numWrites + frameSize;					
			}
			else if( i < memoryFrames ){
				indexStatic.numPageMiss = indexStatic.numPageMiss + 1;
				indexStatic.numDiskPageReads = indexStatic.numDiskPageReads + 1;
				indexStatic.numPageMiss = indexStatic.numPageMiss + 1; 
				indexStatic.numWrites = indexStatic.numWrites + frameSize;						
			}
		} 
	}

	else if( allocPoli == 'G' ){
		if( memoryFrames < 4){
			printf("%s\n", "Memory must have at least 4 frames!!!(We have 4 sorting algorithm)");
			fclose(fp);
			return 0;
		}
		for(int i = 0; i < memoryFrames; i++){
			pageTable[i].prAbBit = 1;
			pageTable[i].targetMemoryFrame = i;

			for(int j = 0; j < frameSize; j++){ 			// virtual memory to physical memory
				int value;
				fseek(fp, sizeof(int) * ((i * frameSize) + j), SEEK_SET);
				fread(&value, sizeof(int), 1, fp);
				physicalMemory[i*frameSize + j] = value;
			}

			if( i < memoryFrames / 4 ){ 					// initial page table statics
				bubbleStatic.numPageMiss = bubbleStatic.numPageMiss + 1;
				bubbleStatic.numDiskPageReads = bubbleStatic.numDiskPageReads + 1;
				bubbleStatic.numPageMiss = bubbleStatic.numPageMiss + 1; 
				bubbleStatic.numWrites = bubbleStatic.numWrites + frameSize;
			}
			else if( i < memoryFrames / 2 ){
				quickStatic.numPageMiss = quickStatic.numPageMiss + 1;
				quickStatic.numDiskPageReads = quickStatic.numDiskPageReads + 1;
				quickStatic.numPageMiss = quickStatic.numPageMiss + 1; 
				quickStatic.numWrites = quickStatic.numWrites + frameSize;
			}
			else if( i < (memoryFrames / 4) * 3){
				mergeStatic.numPageMiss = mergeStatic.numPageMiss + 1;
				mergeStatic.numDiskPageReads = mergeStatic.numDiskPageReads + 1;
				mergeStatic.numPageMiss = mergeStatic.numPageMiss + 1; 
				mergeStatic.numWrites = mergeStatic.numWrites + frameSize;					
			}
			else if( i < memoryFrames ){
				indexStatic.numPageMiss = indexStatic.numPageMiss + 1;
				indexStatic.numDiskPageReads = indexStatic.numDiskPageReads + 1;
				indexStatic.numPageMiss = indexStatic.numPageMiss + 1; 
				indexStatic.numWrites = indexStatic.numWrites + frameSize;						
			}
		} 			
	}

	int quarter = ceil(totalVirtualIntegers / 4); 			// quarter gives quarter of whole virtual integers

	if (pthread_mutex_init(&lock, NULL) != 0){ 				// initialize mutex for threads
        printf("\n mutex init failed\n");
        return -1;
    }

	pthread_t thread_idBubble; 
    pthread_create(&thread_idBubble, NULL, bubbleThreadFun, (void *)&quarter); 	// create threads
		
	pthread_t thread_idQuick; 
    pthread_create(&thread_idQuick, NULL, quickThreadFun, (void *)&quarter); 
	
	pthread_t thread_idMerge; 
    pthread_create(&thread_idMerge, NULL, mergeThreadFun, (void *)&quarter); 
	
	pthread_t thread_idIndex; 
    pthread_create(&thread_idIndex, NULL, indexThreadFun, (void *)&quarter);     

	pthread_join(thread_idBubble, NULL); 						// mutex provides waiting for threads, but wait to whole processes are done
	pthread_join(thread_idQuick, NULL);
	pthread_join(thread_idMerge, NULL);
	pthread_join(thread_idIndex, NULL);

	pthread_t thread_idCheck; 
    pthread_create(&thread_idCheck, NULL, checkThreadFun, (void *)&quarter); 	// check thread
    pthread_join(thread_idCheck, NULL);

    pthread_mutex_destroy(&lock);								// destroy the mutex


	fclose(fp); 												// close the file pointer that pointing file
	printStatics(); 											// print whole statics
	if( virtualFrames == memoryFrames ){
		printf("NOTE: VIRTUAL MEMORY IS EQUAL OR LOWER THAN PHYSICAL MEMORY!!!\n"); 		// print a note if virtual memory is lower than physical memory
	}
	return 0;
}

void * fillThreadFun(void * var){
    pthread_mutex_lock(&lock); 				// lock the mutex
    int *myVar = (int *) var;
    int totalVirtualIntegers = *myVar; 					// get parameter

	srand(1000); 										// create same random numbers
	for(int i = 0; i < totalVirtualIntegers; i++){
		int randValue = rand(); 			
		set(i, randValue, "fill");						// set to virtual disk random numbers with "fill" parameter
	}
	fillStatic.numDiskPageWrites = virtualFrames;	

    pthread_mutex_unlock(&lock); 			// unlock the mutex
    pthread_exit(NULL);
}

void * bubbleThreadFun (void * var)  		// bubble thread function 
{ 
    pthread_mutex_lock(&lock); 				// lock the mutex
    int *myVar = (int *) var;
    int quarter = *myVar; 					// get parameter
	int arrBubble[quarter];
	int j = 0;
	for(int i = 0; i < quarter; i++){
		arrBubble[j] = get(i, "bubble"); 	// get first quarter of integers
		j++;
	}
	bubbleSort(arrBubble, quarter); 		// sort them
	j = 0;
	for(int i = 0; i < quarter; i++){
		set(i, arrBubble[j], "bubble"); 	// set back elements
		j++;
	}
    pthread_mutex_unlock(&lock); 			// unlock the mutex
    pthread_exit(NULL); 
}
void * quickThreadFun (void * var)  	 	// the same operations were done for quick thread function
{ 
    pthread_mutex_lock(&lock);
    int *myVar = (int *) var;
    int quarter = *myVar;
	int arrQuick[quarter];
	int j = 0;
	for(int i = quarter; i < quarter * 2; i++){
		arrQuick[j] = get(i, "quick");
		j++;
	}
	quickSort(arrQuick, quarter);
	j = 0;
	for(int i = quarter; i < quarter * 2; i++){
		set(i, arrQuick[j], "quick");
		j++;
	}
    pthread_mutex_unlock(&lock);
    pthread_exit(NULL); 
}
void * mergeThreadFun (void * var) 			// the same operations were done for merge thread function
{
    pthread_mutex_lock(&lock);
    int *myVar = (int *) var;
    int quarter = *myVar;
	int arrMerge[quarter];
	int j = 0;
	for(int i = quarter * 2; i < quarter * 3; i++){
		arrMerge[j] = get(i, "merge");
		j++;
	}
	mergeSort(arrMerge, 0, quarter-1);
	j = 0;
	for(int i = quarter * 2; i < quarter * 3; i++){
		set(i, arrMerge[j], "merge");
		j++;
	}
    pthread_mutex_unlock(&lock);
    pthread_exit(NULL); 
}
void * indexThreadFun (void * var)  		// the same operations were done for index thread function
{ 
    pthread_mutex_lock(&lock);
    int *myVar = (int *) var;
    int quarter = *myVar;
	int arrIndex[quarter];
	int j = 0;
	for(int i = quarter * 3; i < quarter * 4; i++){
		arrIndex[j] = get(i, "index");
		j++;
	}
	indexSort(arrIndex, quarter);
	j = 0;
	for(int i = quarter * 3; i < quarter * 4; i++){
		set(i, arrIndex[j], "index");
		j++;
	}
    pthread_mutex_unlock(&lock);
    pthread_exit(NULL); 
}

void * checkThreadFun(void * var){
    pthread_mutex_lock(&lock); 				// lock the mutex
    int *myVar = (int *) var;
    int quarter = *myVar; 					// get parameter

	int controlValue = 0;
	for(int i = 0; i < quarter * 4; i++){ 				// control all integers are sorted by their own part
		if( i == quarter || i == quarter * 2 || i == quarter * 3 || i == quarter * 4 ){
			controlValue = 0;
			//printf("%s\n", "***** NEW QUARTER *****");
		}
		int value = get(i, "check"); 		// get function with check parameter
		//printf("Element %d ---> %d\n", i, value);
		if( value < controlValue ){
			printf("%s\n", "Sorted Problem Occured. Some elements are not sorted!");
			break;
		} 
	}

	for(int i = 0; i < virtualFrames; i++){ 	// in last, modified pages write back to disk
		if( pageTable[i].modifyBit == 1 ){
			int targetMemoryFrame = pageTable[i].targetMemoryFrame;
			for(int j = 0; j < frameSize; j++){
				int value = physicalMemory[targetMemoryFrame * frameSize + j];	

				fseek(fp, sizeof(int) * ((i * frameSize) + j), SEEK_SET);
				fwrite(&value, sizeof(int), 1, fp);
			}	
		}
	}

    pthread_mutex_unlock(&lock); 			// unlock the mutex
    pthread_exit(NULL); 
}

void printStatics(){ 							
	printf("%s\n", "****************************************************"); 	// print statics
	printf("Program Statics \n");
	printf("%s\n", "Fill Statics");
	printf("Number of reads : %d \n", fillStatic.numReads);
	printf("Number of writes : %d \n", fillStatic.numWrites);
	printf("Number of page misses : %d \n", fillStatic.numPageMiss);
	printf("Number of page replacement : %d \n", fillStatic.numPageReplace);
	printf("Number of disk page writes : %d \n", fillStatic.numDiskPageWrites);
	printf("Number of disk page reads : %d \n", fillStatic.numDiskPageReads);
	printf("%s\n", "-----------------");

	printf("%s\n", "Buuble Statics");
	printf("Number of reads : %d \n", bubbleStatic.numReads);
	printf("Number of writes : %d \n", bubbleStatic.numWrites);
	printf("Number of page misses : %d \n", bubbleStatic.numPageMiss);
	printf("Number of page replacement : %d \n", bubbleStatic.numPageReplace);
	printf("Number of disk page writes : %d \n", bubbleStatic.numDiskPageWrites);
	printf("Number of disk page reads : %d \n", bubbleStatic.numDiskPageReads);
	printf("%s\n", "-----------------");

	printf("%s\n", "Quick Statics");
	printf("Number of reads : %d \n", quickStatic.numReads);
	printf("Number of writes : %d \n", quickStatic.numWrites);
	printf("Number of page misses : %d \n", quickStatic.numPageMiss);
	printf("Number of page replacement : %d \n", quickStatic.numPageReplace);
	printf("Number of disk page writes : %d \n", quickStatic.numDiskPageWrites);
	printf("Number of disk page reads : %d \n", quickStatic.numDiskPageReads);
	printf("%s\n", "-----------------");

	printf("%s\n", "Merge Statics");
	printf("Number of reads : %d \n", mergeStatic.numReads);
	printf("Number of writes : %d \n", mergeStatic.numWrites);
	printf("Number of page misses : %d \n", mergeStatic.numPageMiss);
	printf("Number of page replacement : %d \n", mergeStatic.numPageReplace);
	printf("Number of disk page writes : %d \n", mergeStatic.numDiskPageWrites);
	printf("Number of disk page reads : %d \n", mergeStatic.numDiskPageReads);
	printf("%s\n", "-----------------");

	printf("%s\n", "Index Statics");
	printf("Number of reads : %d \n", indexStatic.numReads);
	printf("Number of writes : %d \n", indexStatic.numWrites);
	printf("Number of page misses : %d \n", indexStatic.numPageMiss);
	printf("Number of page replacement : %d \n", indexStatic.numPageReplace);
	printf("Number of disk page writes : %d \n", indexStatic.numDiskPageWrites);
	printf("Number of disk page reads : %d \n", indexStatic.numDiskPageReads);
	printf("%s\n", "-----------------");


	printf("%s\n", "Check Statics");
	printf("Number of reads : %d \n", checkStatic.numReads);
	printf("Number of writes : %d \n", checkStatic.numWrites);
	printf("Number of page misses : %d \n", checkStatic.numPageMiss);
	printf("Number of page replacement : %d \n", checkStatic.numPageReplace);
	printf("Number of disk page writes : %d \n", checkStatic.numDiskPageWrites);
	printf("Number of disk page reads : %d \n", checkStatic.numDiskPageReads);
	printf("%s\n", "-----------------");
	printf("%s\n", "****************************************************");
}

void printPageTable(){ 							// print page table
	printf("%s\n", "****************************************************");
	printf("Page table has %d elements.\n", virtualFrames); 
	for(int i = 0; i < virtualFrames; i++){
		printf("%s\n", "-----------------");
		printf("Page %d \n", i);
		printf("Target memory frame : %d \n", pageTable[i].targetMemoryFrame);
		printf("Present/Absent Bit : %d \n", pageTable[i].prAbBit);
		printf("Modift Bit : %d \n", pageTable[i].modifyBit);
		printf("Reference Bit %d\n", pageTable[i].refBit);
	}
	printf("%s\n", "****************************************************");
}

int pageReplace(int index, char * sortAlgo){ 		// this function done whole page replacement operations for global or local with sorting algorithms
	int writeToDiskFlag = 0;
	if( replaceAlgorithm == 'F'){ 	// if replace algorithm is FIFO
		int lowerLimit;
		int upperLimit;	
		int fifoCur;
		if( strcmp(sortAlgo, "bubble") == 0 ){ 		// recognize the sorting algorithm
			lowerLimit = 0;
			upperLimit = virtualFrames / 4; 		// specify the upper and lower limit of sorting algorithms
			fifoCur = fifoBubble;
		}
		else if( strcmp(sortAlgo, "quick") == 0 ){
			lowerLimit = virtualFrames / 4;
			upperLimit = virtualFrames / 2;
			fifoCur = fifoQucik;
		}
		else if( strcmp(sortAlgo, "merge") == 0 ){
			lowerLimit = virtualFrames / 2;
			upperLimit = (virtualFrames / 4) * 3;
			fifoCur = fifoMerge;
		}
		else if( strcmp(sortAlgo, "index") == 0){
			lowerLimit = (virtualFrames / 4) * 3;
			upperLimit = virtualFrames;
			fifoCur = fifoIndex;
		}
		else if( strcmp(sortAlgo, "check") == 0){
			lowerLimit = 0;
			upperLimit = virtualFrames;
			fifoCur = fifoCheck;
		}

		if(allocPoli == 'G'){ 						// if allocation policy is GLOBAL
			lowerLimit = 0; 						// upper and lower limits are top and bottom
			upperLimit = virtualFrames;
		}

		int whichVirtualFrame = (index / frameSize); 	// virtual page is index devided by frame size
		if(fifoCur == 0){
			fifoCur = lowerLimit; 						// if fifo variable is zero, it's evaluated to lower limit
		}

		if( whichVirtualFrame >= upperLimit ){ 			// all algorithms only access their part 
			printf("%s\n", "Wrong access!!");
			return -1;
		} 

		fifoCur++; 										// increment fifo
		if(fifoCur >= upperLimit){
			fifoCur = lowerLimit;
		}
			 
		if( pageTable[fifoCur].prAbBit == 0 ){ 			// if page is zero. find the element that present/absent bit is 1
			while(pageTable[fifoCur].prAbBit != 1){
				fifoCur++;
				if(fifoCur >= upperLimit){
					fifoCur = lowerLimit;
				}				
				if( pageTable[fifoCur].prAbBit == 1 ){
					break;
				}
			}
		}
			
		if( pageTable[fifoCur].modifyBit == 1 ){ 		// if page is modify 		
			writeToDiskFlag = 1;	
			int targetMemoryFrame = pageTable[fifoCur].targetMemoryFrame; 		
			for(int i = 0; i < frameSize; i++){ 		
				int value = physicalMemory[targetMemoryFrame*frameSize + i]; 		// physical memory page is moved to virtual disk on same page(page table index)
				fseek(fp, sizeof(int) * ((fifoCur * frameSize) + i), SEEK_SET);
				fwrite(&value, sizeof(int), 1, fp);
			}
		}
				
		pageTable[fifoCur].prAbBit = 0; 						// set values
		pageTable[fifoCur].modifyBit = 0;
		int targetMemoryFrame = pageTable[fifoCur].targetMemoryFrame;
		pageTable[whichVirtualFrame].prAbBit = 1;
		pageTable[whichVirtualFrame].modifyBit = 0;
		pageTable[whichVirtualFrame].targetMemoryFrame = targetMemoryFrame;

		for(int j = 0; j < frameSize; j++){ 					// page replacement, disk to ram necessary page
			int value;
			fseek(fp, sizeof(int) * ((whichVirtualFrame * frameSize) + j), SEEK_SET);
			fread(&value, sizeof(int), 1, fp);
			physicalMemory[targetMemoryFrame*frameSize + j] = value;
		}

		fifoCur++;
		if(fifoCur == upperLimit){
			fifoCur = lowerLimit;
		}
		// end of FIFO
	}

	else if( replaceAlgorithm == 'L' ){ 					// LRU
		int lowerLimit; 
		int upperLimit;										// Last Recently Used page is found helping of lru variable
		int lruCur;
		if( strcmp(sortAlgo, "bubble") == 0 ){ 				// recognize the sorting algorithm
			lowerLimit = 0;
			upperLimit = virtualFrames / 4;
			lruCur = lruBubble;
		}
		else if( strcmp(sortAlgo, "quick") == 0 ){
			lowerLimit = virtualFrames / 4;
			upperLimit = virtualFrames / 2;
			lruCur = lruQuick;
		}
		else if( strcmp(sortAlgo, "merge") == 0 ){
			lowerLimit = virtualFrames / 2;
			upperLimit = (virtualFrames / 4) * 3;
			lruCur = lruMerge;
		}
		else if( strcmp(sortAlgo, "index") == 0){
			lowerLimit = (virtualFrames / 4) * 3;
			upperLimit = virtualFrames;
			lruCur = lruIndex;
		}
		else if( strcmp(sortAlgo, "check") == 0){
			lowerLimit = 0;
			upperLimit = virtualFrames;
			lruCur = lruCheck;
		}

		if(allocPoli == 'G'){ 							// global situation control same all replacements
			lowerLimit = 0;
			upperLimit = virtualFrames;
		}

		int whichVirtualFrame = (index / frameSize);
		if(lruCur == 0){
			lruCur = upperLimit;
		}

		if( whichVirtualFrame >= upperLimit ){
			printf("%s\n", "Wrong access!!");
			return -1;
		} 

		if( pageTable[lruCur].prAbBit == 0 ){
			while(pageTable[lruCur].prAbBit != 1){
				lruCur--; 							// go in reverse
				if(lruCur < lowerLimit){
					lruCur = upperLimit;
				}				
				if( pageTable[lruCur].prAbBit == 1 ){
					break;
				}
			}	
		}

		if( pageTable[lruCur].modifyBit == 1 ){ 		// if page is modified
			writeToDiskFlag = 1;
			int targetMemoryFrame = pageTable[lruCur].targetMemoryFrame;
			for(int i = 0; i < frameSize; i++){ 				// back up to disk
				int value = physicalMemory[targetMemoryFrame*frameSize + i];
				fseek(fp, sizeof(int) * ((lruCur * frameSize) + i), SEEK_SET);
				fwrite(&value, sizeof(int), 1, fp);
			}
		}
				
		pageTable[lruCur].prAbBit = 0;
		pageTable[lruCur].modifyBit = 0;
		int targetMemoryFrame = pageTable[lruCur].targetMemoryFrame;
		pageTable[whichVirtualFrame].prAbBit = 1;
		pageTable[whichVirtualFrame].modifyBit = 0;
		pageTable[whichVirtualFrame].targetMemoryFrame = targetMemoryFrame;


		for(int j = 0; j < frameSize; j++){ 			// disk to ram
			int value;
			fseek(fp, sizeof(int) * ((whichVirtualFrame * frameSize) + j), SEEK_SET);
			fread(&value, sizeof(int), 1, fp);
			physicalMemory[targetMemoryFrame*frameSize + j] = value;
		}
		lruCur--;
		if(lruCur < lowerLimit){
			lruCur = upperLimit;
		}
		// end of LRU	
	}

	else if( replaceAlgorithm == 'S' ){ 		// SECOND-CHANCE(SC)
		int lowerLimit;
		int upperLimit;							// this algorithm gives a second chance to page
		int scCur;
		if( strcmp(sortAlgo, "bubble") == 0 ){
			lowerLimit = 0;
			upperLimit = virtualFrames / 4;
			scCur = scBubble;
		}
		else if( strcmp(sortAlgo, "quick") == 0 ){
			lowerLimit = virtualFrames / 4;
			upperLimit = virtualFrames / 2;
			scCur = scQuick;
		}
		else if( strcmp(sortAlgo, "merge") == 0 ){
			lowerLimit = virtualFrames / 2;
			upperLimit = (virtualFrames / 4) * 3;
			scCur = scMerge;
		}
		else if( strcmp(sortAlgo, "index") == 0){
			lowerLimit = (virtualFrames / 4) * 3;
			upperLimit = virtualFrames;
			scCur = scIndex;
		}
		else if( strcmp(sortAlgo, "check") == 0){
			lowerLimit = 0;
			upperLimit = virtualFrames;
			scCur = scCheck;
		}

		if(allocPoli == 'G'){
			lowerLimit = 0;
			upperLimit = virtualFrames;
		}
		int whichVirtualFrame = (index / frameSize);
		if(scCur == 0){
			scCur = lowerLimit;
		}

		if( whichVirtualFrame >= upperLimit ){
			printf("%s\n", "Wrong access!!");
			return -1;
		} 

		if( pageTable[scCur].prAbBit == 1 && pageTable[scCur].refBit == 1 ){
			pageTable[scCur].refBit = 0;
			scCur++;
		}

		if( pageTable[scCur].prAbBit == 0 ){
			while(pageTable[scCur].prAbBit != 1){
				scCur++;
				if(scCur >= upperLimit){
					scCur = lowerLimit;
				}				
				if( pageTable[scCur].prAbBit == 1 ){
					if( pageTable[scCur].refBit == 1 ){ 	// give a second chance
						pageTable[scCur].refBit = 0;
					} 
					else{
						break;	
					}
				}
			}
		}
		if( pageTable[scCur].modifyBit == 1 ){ 		// if modify ram to disk.
			writeToDiskFlag = 1;
			int targetMemoryFrame = pageTable[scCur].targetMemoryFrame;

			for(int i = 0; i < frameSize; i++){ 			// ram to disk
				int value = physicalMemory[targetMemoryFrame*frameSize + i];
				fseek(fp, sizeof(int) * ((scCur * frameSize) + i), SEEK_SET);
				fwrite(&value, sizeof(int), 1, fp);
			}
		}
				
		pageTable[scCur].prAbBit = 0;
		pageTable[scCur].modifyBit = 0;
		int targetMemoryFrame = pageTable[scCur].targetMemoryFrame;
		pageTable[whichVirtualFrame].prAbBit = 1;
		pageTable[whichVirtualFrame].modifyBit = 0;
		pageTable[whichVirtualFrame].targetMemoryFrame = targetMemoryFrame;

		for(int j = 0; j < frameSize; j++){ 			// disk to ram
			int value;
			fseek(fp, sizeof(int) * ((whichVirtualFrame * frameSize) + j), SEEK_SET);
			fread(&value, sizeof(int), 1, fp);
			physicalMemory[targetMemoryFrame*frameSize + j] = value;
		}
		scCur++;
		if(scCur == upperLimit){
			scCur = lowerLimit;
		}
		// end of SECOND-CHANCE(SC)	
	}

	else if( replaceAlgorithm == 'N' ){ 		// NOT RECENTLY USED
		int lowerLimit;
		int upperLimit;	
		int nruIndex = -1;
		
		timerReset++; 							// timerReset variable is used for reset reference bits as  a timer.
		if( timerReset == 100){
			timerReset = 0;
			resetRefBits();
		}

		if( strcmp(sortAlgo, "bubble") == 0 ){
			lowerLimit = 0;
			upperLimit = virtualFrames / 4;
		}
		else if( strcmp(sortAlgo, "quick") == 0 ){
			lowerLimit = virtualFrames / 4;
			upperLimit = virtualFrames / 2;
		}
		else if( strcmp(sortAlgo, "merge") == 0 ){
			lowerLimit = virtualFrames / 2;
			upperLimit = (virtualFrames / 4) * 3;
		}
		else if( strcmp(sortAlgo, "index") == 0){
			lowerLimit = (virtualFrames / 4) * 3;
			upperLimit = virtualFrames;
		}
		else if( strcmp(sortAlgo, "check") == 0){
			lowerLimit = 0;
			upperLimit = virtualFrames;
		}

		if(allocPoli == 'G'){
			lowerLimit = 0;
			upperLimit = virtualFrames;
		}

		int whichVirtualFrame = (index / frameSize);
		if( whichVirtualFrame >= upperLimit ){
			printf("%s\n", "Wrong access!!");
			return -1;
		} 
		for(int i = lowerLimit; i < upperLimit; i++){
			if( pageTable[i].prAbBit == 1 && pageTable[i].refBit == 0 && pageTable[i].modifyBit == 0 ){ 			// CASE 0
				nruIndex = i;
				break;
			} 
		}
		if(nruIndex == -1){
			for(int i = lowerLimit; i < upperLimit; i++){
				if( pageTable[i].prAbBit == 1 && pageTable[i].refBit == 0 && pageTable[i].modifyBit == 1 ){ 		// CASE 1
					nruIndex = i;
					break;
				} 
			}		
		}

		if( nruIndex == -1 ){
			for(int i = lowerLimit; i < upperLimit; i++){
				if( pageTable[i].prAbBit == 1 && pageTable[i].refBit == 1 && pageTable[i].modifyBit == 0 ){ 		// CASE 2
					nruIndex = i;
					break;
				} 
			}			
		}

		if( nruIndex == -1 ){
			for(int i = lowerLimit; i < upperLimit; i++){
				if( pageTable[i].prAbBit == 1 && pageTable[i].refBit == 1 && pageTable[i].modifyBit == 1 ){ 		// CASE 3
					nruIndex = i;
					break;
				} 
			}				
		}
		if( pageTable[nruIndex].modifyBit == 1 ){ 		// if modify ram to disk.
			writeToDiskFlag = 1;	
			int targetMemoryFrame = pageTable[nruIndex].targetMemoryFrame;
			for(int i = 0; i < frameSize; i++){ 			// ram to disk
				int value = physicalMemory[targetMemoryFrame*frameSize + i];
				fseek(fp, sizeof(int) * ((nruIndex * frameSize) + i), SEEK_SET);
				fwrite(&value, sizeof(int), 1, fp);
			}
		}
				
		pageTable[nruIndex].prAbBit = 0;
		pageTable[nruIndex].modifyBit = 0;
		pageTable[nruIndex].refBit = 0;
		int targetMemoryFrame = pageTable[nruIndex].targetMemoryFrame;
		pageTable[whichVirtualFrame].prAbBit = 1;
		pageTable[whichVirtualFrame].modifyBit = 0;
		pageTable[whichVirtualFrame].targetMemoryFrame = targetMemoryFrame;

		for(int j = 0; j < frameSize; j++){ 			// disk to ram
			int value;
			fseek(fp, sizeof(int) * ((whichVirtualFrame * frameSize) + j), SEEK_SET);
			fread(&value, sizeof(int), 1, fp);
			physicalMemory[targetMemoryFrame*frameSize + j] = value;
		}
		// end of NOT RECENTLY USED
	}

	else if( replaceAlgorithm == 'W' ){ 		// WSClock
		int lowerLimit;
		int upperLimit;	
		int wsCur;
		if( strcmp(sortAlgo, "bubble") == 0 ){
			lowerLimit = 0;
			upperLimit = virtualFrames / 4;
			wsCur = wsBubble;
		}
		else if( strcmp(sortAlgo, "quick") == 0 ){
			lowerLimit = virtualFrames / 4;
			upperLimit = virtualFrames / 2;
			wsCur = wsQuick;
		}
		else if( strcmp(sortAlgo, "merge") == 0 ){
			lowerLimit = virtualFrames / 2;
			upperLimit = (virtualFrames / 4) * 3;
			wsCur = wsMerge;
		}
		else if( strcmp(sortAlgo, "index") == 0){
			lowerLimit = (virtualFrames / 4) * 3;
			upperLimit = virtualFrames;
			wsCur = wsIndex;
		}
		else if( strcmp(sortAlgo, "check") == 0){
			lowerLimit = 0;
			upperLimit = virtualFrames;
			wsCur = wsCheck;
		}
		if(allocPoli == 'G'){
			lowerLimit = 0;
			upperLimit = virtualFrames;
		}

		int whichVirtualFrame = (index / frameSize);
		if(wsCur == 0){
			wsCur = lowerLimit;
		}
		if( whichVirtualFrame >= upperLimit ){
			printf("%s\n", "Wrong access!!");
			return -1;
		} 
		if( pageTable[wsCur].prAbBit == 1 && pageTable[wsCur].refBit == 1 ){
			pageTable[wsCur].refBit = 0;
			wsCur++;
		}
		if( pageTable[wsCur].prAbBit == 0 ){
			while(pageTable[wsCur].prAbBit != 1){
				wsCur++;
				if(wsCur >= upperLimit){
					wsCur = lowerLimit;
				}				
				if( pageTable[wsCur].prAbBit == 1 ){
					if( pageTable[wsCur].refBit == 1 ){ 	
						pageTable[wsCur].refBit = 0;
					} 
					else{
						break;	
					}
				}
			}
		}
		if( pageTable[wsCur].modifyBit == 1 ){ 		// if modify ram to disk.
			writeToDiskFlag = 1;	
			int targetMemoryFrame = pageTable[wsCur].targetMemoryFrame;
			for(int i = 0; i < frameSize; i++){ 			// ram to disk
				int value = physicalMemory[targetMemoryFrame*frameSize + i];
				fseek(fp, sizeof(int) * ((wsCur * frameSize) + i), SEEK_SET);
				fwrite(&value, sizeof(int), 1, fp);
			}
		}
				
		pageTable[wsCur].prAbBit = 0;
		pageTable[wsCur].modifyBit = 0;
		pageTable[wsCur].refBit = 0;
		int targetMemoryFrame = pageTable[wsCur].targetMemoryFrame;
		pageTable[whichVirtualFrame].prAbBit = 1;
		pageTable[whichVirtualFrame].modifyBit = 0;
		pageTable[whichVirtualFrame].targetMemoryFrame = targetMemoryFrame;
		for(int j = 0; j < frameSize; j++){ 			// disk to ram
			int value;
			fseek(fp, sizeof(int) * ((whichVirtualFrame * frameSize) + j), SEEK_SET);
			fread(&value, sizeof(int), 1, fp);
			physicalMemory[targetMemoryFrame*frameSize + j] = value;
		}
		wsCur++;
		if(wsCur == upperLimit){
			wsCur = lowerLimit;
		}
		// end of WSClock	
	}

	if( strcmp(sortAlgo, "fill") == 0 ){ 						// some statics are added 
		fillStatic.numDiskPageReads = fillStatic.numDiskPageReads + 1;	
		if(writeToDiskFlag == 1){ 							// if writing page to disk is happened
			fillStatic.numDiskPageWrites = fillStatic.numDiskPageWrites + 1;
		}
	}	
	else if( strcmp(sortAlgo, "bubble") == 0 ){
		bubbleStatic.numDiskPageReads = bubbleStatic.numDiskPageReads + 1;	
		if(writeToDiskFlag == 1){
			bubbleStatic.numDiskPageWrites = bubbleStatic.numDiskPageWrites + 1;
		}
	}	
	else if( strcmp(sortAlgo, "quick") == 0 ){
		quickStatic.numDiskPageReads = quickStatic.numDiskPageReads + 1;	
		if(writeToDiskFlag == 1){
			quickStatic.numDiskPageWrites = quickStatic.numDiskPageWrites + 1;
		}
	}	
	else if( strcmp(sortAlgo, "merge") == 0 ){
		mergeStatic.numDiskPageReads = mergeStatic.numDiskPageReads + 1;	
		if(writeToDiskFlag == 1){
			mergeStatic.numDiskPageWrites = mergeStatic.numDiskPageWrites + 1;
		}
	}	
	else if( strcmp(sortAlgo, "index") == 0 ){
		indexStatic.numDiskPageReads = indexStatic.numDiskPageReads + 1;	
		if(writeToDiskFlag == 1){
			indexStatic.numDiskPageWrites = indexStatic.numDiskPageWrites + 1;
		}
	}	
	else if( strcmp(sortAlgo, "check") == 0 ){
		checkStatic.numDiskPageReads = checkStatic.numDiskPageReads + 1;	
		if(writeToDiskFlag == 1){
			checkStatic.numDiskPageWrites = checkStatic.numDiskPageWrites + 1;
		}
	}	
	return 0;	
}

void set(unsigned int index, int value, char * tName){ 			// set function sets a value to specifying index
	int pageReplaceFlag = 0;
	if( strcmp(tName, "fill") == 0){
		fseek(fp, sizeof(int) * index, SEEK_SET); 				// if thread is fill, add value to disk directly
		fwrite(&value, sizeof(int), 1, fp);
	}
	else if( strcmp(tName, "bubble") == 0  || strcmp(tName, "quick") == 0
			 || strcmp(tName, "merge") == 0|| strcmp(tName, "index") == 0){ 	

		struct pageEntry pageTemp = pageTable[index / frameSize];
		if( pageTemp.prAbBit == 0 ){ 		// if prAbbit is zero, call page replacement
			pageReplace(index, tName); 	
			physicalMemory[ ((pageTable[index / frameSize].targetMemoryFrame) * frameSize) + (index % frameSize)] = value; 	// add to physical memory
			pageTable[index / frameSize].modifyBit = 1;
			pageReplaceFlag = 1;
		}
		else if( pageTemp.prAbBit == 1 ){ 	// if prAbbit is one, add directly to target adress
			pageTemp.refBit = 1;
			int ramFrame = pageTemp.targetMemoryFrame;
			int ramAdress = ( ramFrame * frameSize ) + (index % frameSize);
			physicalMemory[ramAdress] = value;
			pageTable[index / frameSize].modifyBit = 1;
		}
	}
	if( strcmp(tName, "fill") == 0 ){ 			// incremenet some statics according to page replacement is happened or not
		fillStatic.numWrites = fillStatic.numWrites + 1;	
		if(pageReplaceFlag == 1){
			fillStatic.numPageReplace = fillStatic.numPageReplace + 1;
			fillStatic.numPageMiss = fillStatic.numPageMiss + 1;
		}
	}	
	else if( strcmp(tName, "bubble") == 0 ){
		bubbleStatic.numWrites = bubbleStatic.numWrites + 1;	
		if(pageReplaceFlag == 1){
			bubbleStatic.numPageReplace = bubbleStatic.numPageReplace + 1;
			bubbleStatic.numPageMiss = bubbleStatic.numPageMiss + 1;	
		}
	}	
	else if( strcmp(tName, "quick") == 0 ){
		quickStatic.numWrites = quickStatic.numWrites + 1;	
		if(pageReplaceFlag == 1){
			quickStatic.numPageReplace = quickStatic.numPageReplace + 1;
			quickStatic.numPageMiss = quickStatic.numPageMiss + 1;
		}
	}	
	else if( strcmp(tName, "merge") == 0 ){
		mergeStatic.numWrites = mergeStatic.numWrites + 1;	
		if(pageReplaceFlag == 1){
			mergeStatic.numPageReplace = mergeStatic.numPageReplace + 1;
			mergeStatic.numPageMiss = mergeStatic.numPageMiss + 1;
		}
	}	
	else if( strcmp(tName, "index") == 0 ){
		indexStatic.numWrites = indexStatic.numWrites + 1;	
		if(pageReplaceFlag == 1){
			indexStatic.numPageReplace = indexStatic.numPageReplace + 1;
			indexStatic.numPageMiss = indexStatic.numPageMiss + 1;	
		}
	}	

	memoryAccessCount++;
	if( memoryAccessCount == memoryAccessLimit ){ 		// if memory access is limit, print page table
		printPageTable();
		memoryAccessCount = 0;
	}
}
int get(unsigned int index, char * tName){ 				// this function get a value by index
	int returnedValue;
	int pageReplaceFlag = 0;
	struct pageEntry pageTemp = pageTable[index / frameSize];
	if( pageTemp.prAbBit == 0 ){
		pageReplace(index, tName); 	 		// page replace were happened
		returnedValue =  physicalMemory[ ((pageTable[index / frameSize].targetMemoryFrame) * frameSize) + (index % frameSize)];
		pageReplaceFlag = 1;
	}
	else if( pageTemp.prAbBit == 1 ){
		pageTemp.refBit = 1;
		int ramFrame = pageTemp.targetMemoryFrame; 	
		int ramAdress = ( ramFrame * frameSize ) + (index % frameSize);
		returnedValue = physicalMemory[ramAdress]; 		// directly access
	}
	if( strcmp(tName, "bubble") == 0 ){
		bubbleStatic.numReads = bubbleStatic.numReads + 1;	
		if(pageReplaceFlag == 1){
			bubbleStatic.numPageReplace = bubbleStatic.numPageReplace + 1;
			bubbleStatic.numPageMiss = bubbleStatic.numPageMiss + 1;	
		}
	}	
	else if( strcmp(tName, "quick") == 0 ){
		quickStatic.numReads = quickStatic.numReads + 1;	
		if(pageReplaceFlag == 1){
			quickStatic.numPageReplace = quickStatic.numPageReplace + 1;
			quickStatic.numPageMiss = quickStatic.numPageMiss + 1;
		}
	}	
	else if( strcmp(tName, "merge") == 0 ){
		mergeStatic.numReads = mergeStatic.numReads + 1;	
		if(pageReplaceFlag == 1){
			mergeStatic.numPageReplace = mergeStatic.numPageReplace + 1;
			mergeStatic.numPageMiss = mergeStatic.numPageMiss + 1;
		}
	}	
	else if( strcmp(tName, "index") == 0 ){
		indexStatic.numReads = indexStatic.numReads + 1;	
		if(pageReplaceFlag == 1){
			indexStatic.numPageReplace = indexStatic.numPageReplace + 1;
			indexStatic.numPageMiss = indexStatic.numPageMiss + 1;	
		}
	}	
	else if( strcmp(tName, "check") == 0 ){
		checkStatic.numReads = checkStatic.numReads + 1;	
		if(pageReplaceFlag == 1){
			checkStatic.numPageReplace = checkStatic.numPageReplace + 1;
			checkStatic.numPageMiss = checkStatic.numPageMiss + 1;
		}
	}	


	memoryAccessCount++; 						// if memory access is limit, print page table
	if( memoryAccessCount == memoryAccessLimit ){
		printPageTable();
		memoryAccessCount = 0;
	}
	return returnedValue;
}
void bubbleSort(int arr[], int n){              // bubble sort function
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n-i-1; j++){         // loop until n-i-1(further is unnecessary)
            if( arr[j] > arr[j+1]){
                int temp = arr[j];              // swap elements for sorting them
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            } 
        }
    }
}
void quickSort(int arr[], int n){ 				// quick sort function
	if ( n >= 2 ){
		int pivot = arr[n / 2]; 	// pivot element is selected as middle element
		int i = 0, j = n - 1;
		for (;; i++, j--){
	    	while (arr[i] < pivot){ 	// find element that is on left-side but higher than pivot
	   			i++;	
	    	} 
	    	while (arr[j] > pivot){ 	// find element that is on right-side but lower than pivot
	    		j--;	
	    	} 
			if (i >= j){
				break;	
			} 
			int temp = arr[i];			// swap elements according to pivot element
			arr[i] = arr[j];
	    	arr[j] = temp;
		}
		quickSort(arr, i); 		// left partition
		quickSort(arr + i, n - i);	// right partition
	}
}
void mergeSort(int arr[], int low, int high){ 		// mergeSort function
	int middle; 									// middle element	
	if(low < high){ 								// a control
		middle = low + (high - low) / 2;
		mergeSort(arr, low, middle);				// left-side mergesort
		mergeSort(arr, middle + 1, high);			// right-side mergesort
		mergeSubArrays(arr, low, middle, high);		// merging of two sorted sub-arrays
	}
}
void mergeSubArrays(int arr[], int low, int middle, int high){
	int tempArr[virtualFrames * frameSize];			// merging temporary array
	int i, j, k; 				// some variables
	i = low;					// starting of first list on left-side
	j = middle + 1;				// starting of second list on left-side
	k = 0;
	while(i <= middle && j <= high){		// if elements are same both list
		if(arr[i] < arr[j]){
			tempArr[k] = arr[i++];
			k++;
		}
		else{
			tempArr[k] = arr[j++];
			k++;
		}
	}
	while(i <= middle){						// copying of first list
		tempArr[k] = arr[i];
		k++;
		i++;
	}				
	while(j <= high){						// copying of second list
		tempArr[k] = arr[j];
		k++;
		j++;
	}						
	for(i = low, j = 0; i <= high; i++, j++){	// copy temp array to main array
		arr[i] = tempArr[j];
	}
}
void indexSort(int arr[], int n){
    int writeNum = 0; 		
    for( int indexStart = 0; indexStart <= n - 1; indexStart++){ 
        int element = arr[indexStart];  			
        int index = indexStart; 
        for( int i = indexStart + 1; i < n; i++){ 		
            if (arr[i] < element){
                index++; 
            }
        }  
        if(index != indexStart){
	        if(index != indexStart){  			
	            int temp = element;
	            element = arr[index];
	            arr[index] = temp;            
	            writeNum++; 
	        } 
	        while(index != indexStart){ 
	        	index = indexStart; 
	            for (int i = indexStart + 1; i < n; i++){
	            	if (arr[i] < element){
	                	index += 1; 	
	            	}
				}
	            if (element != arr[index]) { 
	            	int temp = element;
	            	element = arr[index];
	            	arr[index] = temp;
	                writeNum++; 
	            } 
	        } 
        }
    } 
}
void printAnArray(int arr[], int lengthArr){             // print an array according to its size
    for(int i = 0; i < lengthArr; i++){                  // iterates all elements 
        printf("Element %d : %d\n", i+1, arr[i]);   // print them
    } 
}

void resetRefBits(){
	for(int i = 0; i < virtualFrames; i++){
		pageTable[i].refBit = 0;
	}
}

