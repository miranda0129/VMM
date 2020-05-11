#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#define PAGE_TABLE_SIZE  256 // SAMLL TABLE WITH 256 ENTRIES
#define ADDRESS_SIZE 16
#define PAGE_SIZE 8  //KB


int memoryAddress;
int current = 0;
int previous = 0;
int hits = 0;
int memoryTotal = 0;
char PageTable[PAGE_TABLE_SIZE];
char PhysicalMemory[PAGE_TABLE_SIZE][PAGE_TABLE_SIZE]; 

typedef struct TLB_table {// Translation Lookaside Buffer (TLB) keeps track of recently used transactions
	// Non-contiguous memory allocation
	// represention of Paging. 
	unsigned char page[ADDRESS_SIZE]; //entries that have been most recently used . aka secondary memory 
	unsigned char frame[ADDRESS_SIZE]; // main memory 
	int lookaside_index;
}tb;
//CPU always generates a logical address.
//A physical address is needed to access the main memory.
// 
struct TLB_table table;	


int readMemoryAddress ( char *PhysicalMemory ,int* pageOffset,int pageAddress){
    char memory[PAGE_TABLE_SIZE];
    FILE *fd;
    fd = fopen("BACKING_STORE.bin", "rb");
	memset(memory, 0, sizeof(memory));
    //SEEK_SET – It moves file pointer position to the beginning of the file.
	if (fseek(fd, pageAddress * PAGE_TABLE_SIZE, SEEK_SET)!=0)
		printf("error \n");
	if (fread(memory, sizeof(char), PAGE_TABLE_SIZE, fd)==0)
		printf("error \n");

    int index = 0;
    while (index < PAGE_TABLE_SIZE){
        int address = (*pageOffset)*PAGE_TABLE_SIZE + index; 
		*(PhysicalMemory+address) = memory[index];
        index++;
	}
	(*pageOffset)++;

	return (*pageOffset)-1;
}

int printPhysicalAddress(int  address,unsigned char pageOffset,char* PhysicalMemory){
int index = ((unsigned char)address*PAGE_TABLE_SIZE)+pageOffset;
int value = *(PhysicalMemory+index);
	printf("Physical address: %d\t Value: %d\n",index, value);	
return 0;
}


int FillTables(){
	memset(PageTable, -1, sizeof(PageTable));	
	memset(table.page, -1, sizeof(table.page));
	memset(table.frame, -1, sizeof(table.frame));
	table.lookaside_index = 0;
	return 0;
}

int printVirtualAddress(int n){
	printf("Virtual adress: %d\t", n);
    return 0;
}


int Statistics(int previous,int memoryTotal,int hits ){
    float FaultRate;
	float HitRate;
    FaultRate = (float)previous / (float)memoryTotal;
	HitRate = (float)hits / (float)memoryTotal;
	printf("Number of addresses translated %d\n", memoryTotal);
	printf("Page Faults %d\n",previous);
	printf("Page Faults Rate = %.4f\n",FaultRate);
	printf("TLB Hits %d \n",hits);
	printf("TLB Hit Rate= %.4f\n", HitRate);

return 0;
}

int findPage(unsigned char pageNumber){
	//TLB is used to reduce effective memory access time as it is a high speed associative cache.
	table.page[table.lookaside_index] = pageNumber;
	table.frame[table.lookaside_index] = PageTable[pageNumber];
	table.lookaside_index = (table.lookaside_index + 1)%ADDRESS_SIZE;
return 0;
}


int main (int argc, char* argv[]){
	FillTables();
	FILE *fd;
	fd = fopen(argv[1], "r");
	while (fscanf(fd, "%d", &memoryAddress)==1){
		unsigned char pageNumber;
	    bool isHit = false;
        unsigned char bitMasking = 0xFF;
	    unsigned char pageOffset;
	    int n = 0;
	    int blackOut = 0;
        printVirtualAddress(memoryAddress);
	    pageNumber = (memoryAddress >> 8) & bitMasking;
	    pageOffset = memoryAddress & bitMasking;

		
		int i = 0;
    	while (i < ADDRESS_SIZE){
        	if(table.page[i] == pageNumber){
			n = table.frame[i];
			isHit = true;
			hits++;
		    }
        i++;
        }

	    if (isHit == false){// checked in TLB, present.  
		    if (PageTable[pageNumber] != -1){
		    }
		    else{ // checked in TLB, not present. 
			blackOut = readMemoryAddress((char*) PhysicalMemory, &current, pageNumber);
			//Now the page number is matched to page table residing in main memory
			PageTable[pageNumber] = blackOut;
			//Corresponding frame number is retrieved, which now tells where in the main memory page lies.
			previous++;
		    }
			n = PageTable[pageNumber];
			findPage(pageNumber);
			//he TLB is updated with new PTE 

	    }

	    printPhysicalAddress(n, pageOffset,(char*) PhysicalMemory);
		memoryTotal++;
	}
    Statistics(previous,memoryTotal,hits);
	return 0;

}
