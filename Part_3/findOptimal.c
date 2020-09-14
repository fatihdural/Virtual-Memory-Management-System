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

void printAnArray(int arr[], int lengthArr); 			// print functions

void * bubbleThreadFun(void *var); 						// sorting thread that called sorting, get, set functions
void * quickThreadFun(void * var);
void * mergeThreadFun(void * var);
void * indexThreadFun(void * var);

int pageReplace(int index, char * sortAlgo); 			// page replace function

struct pageEntry{ 										// page entry 
	int targetMemoryFrame;
	int prAbBit;
	int modifyBit;
	int refBit;
};
struct pageEntry * pageTable;							// page table that holds page entries

int * physicalMemory; 
int * virtualMemory; 	// virtual memory is a integer array, too										
int frameSize, memoryFrames, virtualFrames;
int fifoBubble = 0, fifoQucik = 0, fifoMerge = 0, fifoIndex = 0, fifoCheck = 0;
int pageReplacementBubble = 0, pageReplacementQuick = 0, pageReplacementMerge = 0, pageReplacementIndex = 0;

pthread_mutex_t lock; 										// mutex for threads

int main(int argc, char const *argv[])
{
	int tempBubble = 999999; 		// some temporaries
   	int tempQuick = 999999;
    int tempMerge = 999999;
    int tempIndex = 999999;

    int smallestFrameSizeBubble; 	// this variables gives smallest frame size after loop
    int smallestFrameSizeQuick;
    int smallestFrameSizeMerge;
    int smallestFrameSizeIndex;

	frameSize = 1 * 512; 			// frame size increases with 512 byte
	memoryFrames = (64) * 1024 / frameSize; 		// initial values
	virtualFrames = (1024) * 1024/ frameSize; 

    while( memoryFrames >= 4 ){ 		// loop until memory frame numbers lower than 4.(because we have 4 sorting algorithms)
		int totalMemoryIntegers = frameSize * memoryFrames; 	// memory has frame * memoryframes integers
		physicalMemory = (int*) malloc(sizeof(int) * totalMemoryIntegers);
		for(int i = 0; i < totalMemoryIntegers; i++){
			physicalMemory[i] = 0; 								// initialize memory by zero
		}

		int totalVirtualIntegers = frameSize * virtualFrames; 		
		virtualMemory = (int*) malloc(sizeof(int) * totalVirtualIntegers);
		srand(1000); 										// create same random numbers
		for(int i = 0; i < totalVirtualIntegers; i++){
			int randValue = rand(); 			
			set(i, randValue, "fill");						// set to virtual disk random numbers with "fill" parameter
		} 

		printf("%s\n", "Please Wait......");
		pageTable = (struct pageEntry *) malloc(sizeof(struct pageEntry) * virtualFrames); 		// page elements are changing according to virtual frames size

		for(int i = 0; i < virtualFrames; i++){ 			// page table initialize.
			struct pageEntry ex;
			ex.targetMemoryFrame = 0;
			ex.prAbBit = 0;	
			ex.modifyBit = 0;
			ex.refBit = 0;
			pageTable[i] = ex;
		}
		for(int i = 0; i < memoryFrames; i++){
			pageTable[i].prAbBit = 1;
			pageTable[i].targetMemoryFrame = i;

			for(int j = 0; j < frameSize; j++){ 				// virtual memory to physical memory
				int value = virtualMemory[i * frameSize + j];
				physicalMemory[i*frameSize + j] = value;
			}
		}
		int quarter = ceil(totalVirtualIntegers / 4); 			// quarter gives quarter of whole virtual integers

		if (pthread_mutex_init(&lock, NULL) != 0){ 				// initialize mutex for threads
	        printf("\n mutex init failed\n");
	        return -1;
	    }

		pthread_t thread_idBubble; 
	    pthread_create(&thread_idBubble, NULL, bubbleThreadFun, (void *)&quarter); 	// create threads
		pthread_join(thread_idBubble, NULL);

		pthread_t thread_idQuick; 
	   	pthread_create(&thread_idQuick, NULL, quickThreadFun, (void *)&quarter); 
	   	pthread_join(thread_idQuick, NULL);

		pthread_t thread_idMerge; 
	    pthread_create(&thread_idMerge, NULL, mergeThreadFun, (void *)&quarter); 
	    pthread_join(thread_idMerge, NULL);

		pthread_t thread_idIndex; 
	    pthread_create(&thread_idIndex, NULL, indexThreadFun, (void *)&quarter);
		pthread_join(thread_idIndex, NULL);

		pthread_mutex_destroy(&lock);								// destroy the mutex

		printf("FOR FRAME SIZE : %d BYTE \n", frameSize);
		printf("Bubble Sort Number of Page Replacement: %d \n", pageReplacementBubble);
		printf("Quick  Sort Number of Page Replacement: %d \n", pageReplacementQuick);
		printf("Merge  Sort Number of Page Replacement: %d \n", pageReplacementMerge);
		printf("Index  Sort Number of Page Replacement: %d \n", pageReplacementIndex);
		printf("---------------------------------\n");

		if( pageReplacementBubble <= tempBubble ){
			tempBubble = pageReplacementBubble;
			pageReplacementBubble = 0;
			smallestFrameSizeBubble = frameSize;
		}
		if( pageReplacementQuick <= tempQuick ){
			tempQuick = pageReplacementQuick;
			pageReplacementQuick = 0;
			smallestFrameSizeQuick = frameSize;
		}
		if( pageReplacementMerge <= tempMerge ){
			tempMerge = pageReplacementMerge;
			pageReplacementMerge = 0;
			smallestFrameSizeMerge = frameSize;
		}
		if( pageReplacementIndex <= tempIndex ){
			tempIndex = pageReplacementIndex;
			pageReplacementIndex = 0;
			smallestFrameSizeIndex = frameSize;
		}
		frameSize += 512;	

		memoryFrames = (64) * 1024 / frameSize; // (* 1024)
		virtualFrames = (1024) * 1024/ frameSize; // (* 1024)
    }
    printf("********************************************\n");
    printf("Optimal Page Size for Bubble Sort: %d KB\n", smallestFrameSizeBubble / 1024);
    printf("Optimal Page Size for Quick  Sort: %d KB\n", smallestFrameSizeQuick / 1024);
    printf("Optimal Page Size for Merge  Sort: %d KB\n", smallestFrameSizeMerge / 1024);
    printf("Optimal Page Size for Index  Sort: %d KB\n", smallestFrameSizeIndex / 1024);
	return 0;
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
	quickSort(arrBubble, quarter); 		// sort them
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
	quickSort(arrMerge, quarter);
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
	quickSort(arrIndex, quarter);
	j = 0;
	for(int i = quarter * 3; i < quarter * 4; i++){
		set(i, arrIndex[j], "index");
		j++;
	}
    pthread_mutex_unlock(&lock);
    pthread_exit(NULL); 
}

int pageReplace(int index, char * sortAlgo){ 		// this function done whole page replacement operations for global or local with sorting algorithm
	int lowerLimit = 0;
	int upperLimit = virtualFrames;
	int fifoCur;
	if( strcmp(sortAlgo, "bubble") == 0 ){ 		// recognize the sorting algorithm
		fifoCur = fifoBubble;
		pageReplacementBubble++;
	}
	else if( strcmp(sortAlgo, "quick") == 0 ){
		fifoCur = fifoQucik;
		pageReplacementQuick++;
	}
	else if( strcmp(sortAlgo, "merge") == 0 ){
		fifoCur = fifoMerge;
		pageReplacementMerge++;
	}
	else if( strcmp(sortAlgo, "index") == 0){
		fifoCur = fifoIndex;
		pageReplacementIndex++;
	}
	else if( strcmp(sortAlgo, "check") == 0){
		fifoCur = fifoCheck;
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
		int targetMemoryFrame = pageTable[fifoCur].targetMemoryFrame; 		
		for(int i = 0; i < frameSize; i++){ 	
			int value = physicalMemory[targetMemoryFrame*frameSize + i]; 		// physical memory page is moved to virtual disk on same page(page table index)
			virtualMemory[(fifoCur * frameSize) + i] = value;
		}
	}				
	pageTable[fifoCur].prAbBit = 0; 						// set values
	pageTable[fifoCur].modifyBit = 0;
	int targetMemoryFrame = pageTable[fifoCur].targetMemoryFrame;
	pageTable[whichVirtualFrame].prAbBit = 1;
	pageTable[whichVirtualFrame].modifyBit = 0;
	pageTable[whichVirtualFrame].targetMemoryFrame = targetMemoryFrame;

	for(int j = 0; j < frameSize; j++){ 					// page replacement, disk to ram necessary page
		int value = virtualMemory[(whichVirtualFrame * frameSize) + j];
		physicalMemory[targetMemoryFrame*frameSize + j] = value;
	}
	fifoCur++;
	if(fifoCur == upperLimit){
		fifoCur = lowerLimit;
	}
	// end of FIFO
	return 0;	
}

void set(unsigned int index, int value, char * tName){ 			// set function sets a value to specifying index
	int pageReplaceFlag = 0;
	if( strcmp(tName, "fill") == 0){
		virtualMemory[index] = value;
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
