#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef INCLUDE_H

#define INCLUDE_H

extern int test();
extern unsigned int LEToInt(unsigned char* bytes, int length);
extern void readBytesFromDisk(FILE* file, int start, int magnitude, char* dest);
extern void whatsTheFileName(char* rootVal, char* nameOfFile);
extern int getSizeOfFile(char* rootVal);
extern int countAvailableSectors(FILE* file, int numOfSecs);
extern void upper(char* fileNameFinal);

#endif