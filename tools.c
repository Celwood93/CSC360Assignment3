#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tools.h"

int test(){
	printf("hi");
	return 1;
}

int getSizeOfFile(char* rootVal){
	unsigned char fileSizeInBytes[4];
	int i = 0;
	while(i<4){
		fileSizeInBytes[i] = rootVal[28+i];
		i++;
	}
	return LEToInt(fileSizeInBytes, 4);
}

unsigned int LEToInt(unsigned char* bytes, int length){
	unsigned int pow2 = 1;
    unsigned int finalInt = 0;
    int i;
    for( i = 0; i < length; i++){
        finalInt += pow2*bytes[i];
        pow2 = pow2*256;
    }
    return finalInt;
}

unsigned char* IntToLE(unsigned int ints, int length){
	char arr[length];
    int i;
    for( i = 0; i < length; i++){
     	arr[i]=ints>>i*8;
    }
    return arr;
}

void readBytesFromDisk(FILE* file, int start, int magnitude, char* dest){
	if(fseek(file, start, SEEK_SET) != 0){
		printf("error at fseek\n");
	}
	fread(dest, 1, magnitude, file);
}

void whatsTheFileName(char* rootVal, char* nameOfFile){
	int count = 0;
	int copyPointer = 0;
	while(count < 12){

		if(count == 8){
			nameOfFile[copyPointer] = '.';
			copyPointer++;
			nameOfFile[copyPointer] = rootVal[count];
			count++;
			copyPointer++;
			continue;
		}
		if(rootVal[count] == ' '){
			count++;
			continue;
		}
		nameOfFile[copyPointer] = rootVal[count];
		count++;
		copyPointer++;
	}
	nameOfFile[count] = '\0';
}

int countAvailableSectors(FILE* file, int numOfSecs){
	char twoEntriesFat[3];
	int totalFreeSectors = 0;
	fseek(file, 514, SEEK_SET);
	int whichSectorAreWeAt = 32; //physics sector numbers follow the forumla 33+ fat entry num -2, so 32 for fat entry 1
	while(whichSectorAreWeAt < numOfSecs){
		fread(twoEntriesFat, 1, 3, file);
		whichSectorAreWeAt++;
		if(twoEntriesFat[0] == 0 && (twoEntriesFat[1] & 0x0F)==0){
			totalFreeSectors++;
		}
		whichSectorAreWeAt++;
		if(whichSectorAreWeAt < numOfSecs && twoEntriesFat[2] == 0 && (twoEntriesFat[1]&0xF0)==0){
			totalFreeSectors++;
		}

	}
	return totalFreeSectors;

}

void upper(char* fileNameFinal){
	int i = 0;
	char c;

	while(fileNameFinal[i]){
		fileNameFinal[i]= toupper(fileNameFinal[i]);
		i++;
	}
	return;
}