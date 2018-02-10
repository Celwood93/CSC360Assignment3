#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tools.h"


void getLabel(FILE *file, char *labelOfDisk){
	char rootDir[32];
    char attr;
    fseek(file, 512*19, SEEK_SET);
    int i = 0;
    while (i < 224){
        fread(rootDir,1 ,32,file);

        attr = rootDir[11];
        if((attr & 0x08) == 0x08 && (attr & 0xF5) == 0){
            strncpy(labelOfDisk, rootDir, 8);
            labelOfDisk[8] = '\0';
            break;
        }
        i++;
    }
    return;
}


int countRootDirFiles(FILE* file){

	fseek(file, 512*19, SEEK_SET);

	char currRootEntry[32];
	int rootFileCount = 0;
	int numEntries = 0;
	int count = 0;

	while(count < 224){
		fread(currRootEntry, 1, 32, file);
		if(currRootEntry[11] != 0x0F && (currRootEntry[11]&0x08)==0 && (currRootEntry[0]& 0xE5)!=0){
			if(currRootEntry[0] == 0 ){
				break;
			}
			numEntries++;
		}
		count++;
	}
	return numEntries;

}

int main(int argc, char *argv[]){
    char OS_Name[8];
    char labelOfDisk[11];
    char totalSecCount[2];
    char bytesPerSec[2];
    int sizeOfDisk = 0;
    int freeSizeOfDisk = 0;
    int rootDirFiles = 0;
    char fatCopies[1];
    char SpFat[2];

    FILE* file;
  

    file = fopen(argv[1], "r");
    if(file == NULL){
    	printf("Failed to open file\n");
    	return -1;
    }
    readBytesFromDisk(file, 3, 8, OS_Name);
    OS_Name[8] = 0;

    getLabel(file, labelOfDisk);

    readBytesFromDisk(file, 19, 2, totalSecCount);
    int secCountTotal = LEToInt(totalSecCount,2);
    sizeOfDisk = 512*secCountTotal;

    readBytesFromDisk(file, 16, 1, fatCopies);
    readBytesFromDisk(file, 22, 2, SpFat);

    freeSizeOfDisk = countAvailableSectors(file, secCountTotal);
    rootDirFiles = countRootDirFiles(file);

    printf("OSName: %s\n", OS_Name);
    printf("Label of the disk: %s\n", labelOfDisk);
    printf("Total size of the disk: %d\n", sizeOfDisk);

    printf("Free size of disk: %d\n", freeSizeOfDisk*512);


    printf("\n==============\n");
    printf("The number of files in the root directory (not including subdirectories): %d\n", rootDirFiles);


    printf("\n==============\n");
 	printf("Number of FAT copies: %d\n", fatCopies[0]);
 	printf("Sectors per Fat: %d\n", LEToInt(SpFat, 2));

 	fclose(file);

}