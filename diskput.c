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


int createFatEntry(int current, int next, char * disk){
	disk += 512;
	int offset = ((3*current) / 2);

	if ((current % 2) == 0) {
		disk[513 + offset] = (next >> 8) & 0x0F;
		disk[512 + offset] = next & 0xFF;
	} else {
		disk[512 + offset] = (next << 4) & 0xF0;
		disk[513 + offset] = (next >> 4) & 0xFF;
	}
}

int fatEntry(int index, char *image){
	int result;
	int firstByte;
	int secondByte;
	int offset = ((3*index) / 2);

	if ((index% 2) == 0) {
		firstByte = image[513 + offset] & 0x0F;
		secondByte = image[512 + offset] & 0xFF;
		result = (firstByte << 8) + secondByte;
	} else {
		firstByte = image[512 + offset] & 0xF0;
		secondByte = image[513 + offset] & 0xFF;
		result = (firstByte >> 4) + (secondByte << 4);
	}

	return result;
}

int nextAvailableFatIndex(char * diskImage){
	diskImage = diskImage + 512;

	int index = 2;
	while (fatEntry(index, diskImage) != 0x000) {
		index++;
	}

	return index;
}

int updateRootDirectory(char* fileName, int fileSize, int current, char* diskImage){
	diskImage = diskImage+512*19; //move to root
	char fileNameShort[9];
	char extension[4];

	char buff[10];
	strncpy(buff, fileName, 8);
	upper(buff);
	strcpy(fileNameShort, strtok(buff, "."));
	strcpy(extension, strtok(NULL, "."));
	extension[3] = 0;
	
	char compareFileName[10];
	while(diskImage[0] != 0){
		strncpy(compareFileName, diskImage, 8);
		if(strcmp(fileName, compareFileName)==0){
			return -1;
		}
		diskImage+=32;
	}
	int i;
	int done = 1;
	for(i=0; i< 8; i++){
		if(done){
			diskImage[i] = fileNameShort[i];
			if(fileNameShort[i+1] == '\0'){
				done = 0;
			}
		}else{
			diskImage[i] = ' ';
		}
	}
	done = 1;
	for(i=0; i< 3; i++){
		if(done){
			diskImage[8+i] = extension[i];
			if(extension[i+1] == '\0'){
				done = 0;
			}
		}else{
			diskImage[i] = ' ';
		}

	}

	diskImage[11] = 0x00; //attribute is set to 0

	for(i=0; i< 25; i++){ //set all the time stuff to 0 -> ignore it
		if(i != 12 && i!= 13 && i!= 20 && i!= 21){
			diskImage[i+14] = 0;
		}
	}

	diskImage[26] = (current - (diskImage[27] << 8)) & 0xFF;
	diskImage[27] = (current - diskImage[26]) >> 8;

	diskImage[28] = (fileSize & 0x000000FF);
	diskImage[29] = (fileSize & 0x0000FF00) >> 8;
	diskImage[30] = (fileSize & 0x00FF0000) >> 16;
	diskImage[31] = (fileSize & 0xFF000000) >> 24;

	return 0;
}

void copyFileToDisk(char * diskImage, char * fileImage, char * fileName, int fileSize){
	int bytesRemaining = fileSize;
	int current = nextAvailableFatIndex(diskImage);
	if(updateRootDirectory(fileName, fileSize, current, diskImage) == -1){
		printf("File already exists on the disk\n");
		exit(1);
	}

	while (bytesRemaining > 0) {
		int physicalAddress = 512 * (31 + current);
		
		int i;
		for (i = 0; i < 512; i++) {
			if (bytesRemaining == 0) {
				createFatEntry(current, 0xFFF, diskImage);
				return;
			}
			diskImage[i + physicalAddress] = fileImage[fileSize - bytesRemaining];
			bytesRemaining--;
		}
		createFatEntry(current, 0x69, diskImage);
		int next = nextAvailableFatIndex(diskImage);
		createFatEntry(current, next, diskImage);
		current = next;
	}
}


int main(int argc, char *argv[]){
	if(argc != 3){
		printf("3 args are required, there are %d given.", argc);
		return -1;
	}

	int sizeOfFile;
	int spaceAvailable;
	char totalSecCount[2];
	int secCountTotal;


	FILE* file;
	file = fopen(argv[1], "r+");
    if(file == NULL){
    	printf("Failed to open file\n");
    	return -1;
    }

    FILE* readFile;
    readFile = fopen(argv[2], "r");
    if(readFile == NULL){
    	printf("Failed to read file\n");
    	return -1;
    }

    if(fseek(readFile, 0, SEEK_END)==0){
    	sizeOfFile = ftell(readFile);
    	if(sizeOfFile == -1){
    		printf("error in file size\n");
    		return -1;
    	}
    }else{
    	printf("error getting to end of file\n");
    	return -1;
    }

    readBytesFromDisk(file, 19, 2, totalSecCount);
    secCountTotal = LEToInt(totalSecCount,2);
    spaceAvailable = 512*countAvailableSectors(file, secCountTotal);

    if(sizeOfFile > spaceAvailable){
    	printf("Not enough free space in disk image");
    }

    fclose(file);
    fclose(readFile);


    int file1 = open(argv[1], O_RDWR);
	if (file1 < 0) {
		printf("failed to read disk\n");
		exit(1);
	}

	int readFile1 = open(argv[2], O_RDWR);
	if (readFile1 < 0) {
		printf("failed to read file\n");
		exit(1);
	}
    struct stat buf;
	fstat(file1, &buf);
	char* p = mmap(NULL, buf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, file1, 0);
	if (p == MAP_FAILED) {
		printf("Mapping disk to memory failed\n");
		exit(1);
	}

	struct stat buf2;
	fstat(readFile1, &buf2);
	char* p2 = mmap(NULL, buf2.st_size, PROT_READ, MAP_SHARED, readFile1, 0);
	if (p2 == MAP_FAILED) {
		printf("Mapping file to memory failed\n");
		exit(1);
	}
	copyFileToDisk(p, p2, argv[2], sizeOfFile);

	munmap(p, buf.st_size);
	munmap(p2, sizeOfFile);
	close(file1);
	close(readFile1);

}