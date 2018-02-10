#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/mman.h>
#include "tools.h"

#define timeOffset 14 //offset of creation time in directory entry
#define dateOffset 16 //offset of creation date in directory entr


char getFileType(char* rootVal){
	if((rootVal[11]&0x0F) == 0x0F || (rootVal[11]&0x08)==0x08 || rootVal[0] == 0 || rootVal[0] == 0xE5){
		return 'E';
	}else if(rootVal[11]&0x10 != 0){
		return 'D';
	}else{
		return 'F';
	}
}


void getCreationDateAndTime(char* rootVal, char* dateCreated, char* specificTimeCreated){
	
	int time, date;
	int hours, minutes, day, month, year;
	
	time = *(unsigned short *)(rootVal + timeOffset);
	date = *(unsigned short *)(rootVal + dateOffset);

	year = ((date & 0xFE00) >> 9) + 1980;
	month = (date & 0x1E0) >> 5;
	day = (date & 0x1F);
	sprintf(dateCreated, "%d-%02d-%02d ", year, month, day);

	hours = (time & 0xF800) >> 11;
	minutes = (time & 0x7E0) >> 5;
	sprintf(specificTimeCreated, "%02d:%02d", hours, minutes);
	return ;
}


int main(int argc, char *argv[]){
	char typeOfFile;
	int sizeOfFile;
	char nameOfFile[21];
	char dateCreated[32];
	char specificTimeCreated[32];
	char rootVal[32];

	FILE* file;
	file = fopen(argv[1], "r");
    if(file == NULL){
    	printf("Failed to open file\n");
    	return -1;
    }

    fseek(file, 512*19, SEEK_SET);
    int whichFile = 0;
    while(whichFile < 224){
    	fread(rootVal, 1, 32, file);
    	typeOfFile = getFileType(rootVal);
    	whichFile++;
    	if(typeOfFile == 'E'){
    		continue;
    	}
    	sizeOfFile = getSizeOfFile(rootVal);
		whatsTheFileName(rootVal, nameOfFile);
    	getCreationDateAndTime(rootVal, dateCreated, specificTimeCreated);
    	printf("%c %10d %20s %s %s\n", typeOfFile, sizeOfFile, nameOfFile, dateCreated, specificTimeCreated);
    }
}