#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/mman.h>
#include "tools.h"

int getRootInfo(FILE* file, char* fileName, int* clusterNum, int* fileSize){
	char rootVal[32];
	char currFileName[16];
	char numClusterBytes[2];

	fseek(file, 512*19, SEEK_SET);

	int count = 0;

	while(count < 224){
		fread(rootVal, 1, 32, file);
		whatsTheFileName(rootVal, currFileName);
		if(strcmp(currFileName, fileName) == 0){
			numClusterBytes[0] = rootVal[26];
			numClusterBytes[1] = rootVal[27];
			*clusterNum = LEToInt(numClusterBytes, 2) + 31;
			*fileSize = getSizeOfFile(rootVal);
			return 1;
		}
		count++;
	}
	return -1;
}


void getClusterInfo(FILE* file, char* buffer, int clusterNum){
	fseek(file, clusterNum*512, SEEK_SET);
	fread(buffer, 1, 512, file);
}

void nextCluster(int* clusterNum, FILE* file){

	int fatIndex = *clusterNum - 31;
	unsigned char twoFatEntry[3];
	unsigned char readableFatEntry[2];

	fseek(file, 512+(3*(fatIndex/2)), SEEK_SET);
	fread(twoFatEntry, 1, 3, file);

	if(fatIndex%2 == 1){
		readableFatEntry[0] = (twoFatEntry[2] << 4) | (twoFatEntry[1] >> 4) ;
        readableFatEntry[1] = (twoFatEntry[2] >> 4);

	}else{
		readableFatEntry[0] = twoFatEntry[0];
		readableFatEntry[1] = twoFatEntry[1]&0x0F;
	}
	*clusterNum = LEToInt(readableFatEntry,2)+31;
	return;
}




int main(int argc, char *argv[]){
	if(argc != 3){
		printf("3 args are required, there are %d given.", argc);
		return -1;
	}

	char fileName[16];
	strncpy(fileName, argv[2], 16);
	char buffer[512];
	int clusterNum;
	int fileSize;

	FILE* file;
	file = fopen(argv[1], "r");
    if(file == NULL){
    	printf("Failed to open file\n");
    	return -1;
    }


    upper(fileName);


    if(getRootInfo(file, fileName, &clusterNum, &fileSize) == -1){
    	printf("Failed to find file: %s", fileName);
    }

    FILE* fileWrite;
	fileWrite = fopen(fileName, "w");
    if(fileWrite == NULL){
    	printf("Failed to write to file\n");
    	return -1;
    }

    int size = 512;
    while(fileSize != 0){
    	getClusterInfo(file, buffer, clusterNum);
    	if(fileSize < 512){
    		size = fileSize;
    	}

    	fwrite(buffer, 1, size, fileWrite);

    	fileSize-=size;
    	if(fileSize != 0){
    		nextCluster(&clusterNum, file);
    	}
    }
    fclose(file);
    fclose(fileWrite);


}