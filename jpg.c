#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "jpg.h"

/*
 * Analyze a JPG file that contains Exif data.
 * If it is a JPG file, print out all relevant metadata and return 0.
 * If it isn't a JPG file, return -1 and print nothing.
 */
int analyze_jpg(FILE *f) {
    /* YOU WRITE THIS PART */
	unsigned int superLowBound = 0xffd0;
	unsigned int superHighBound = 0xffda;
	unsigned int APP1 = 0xffe1;
	
	//get first marker
	unsigned char *marker;
	marker = malloc(2);
	fread(marker, 1, 2, f);
	while(!feof(f)){
		//check if chunk super or standard
		unsigned int markerCheck = 256*marker[0] + marker[1];
		//printf("markerCheck: %x\n", markerCheck);
		if (markerCheck >= superLowBound && markerCheck <= superHighBound) {
			//printf("in super chunk\n");
			//is super chunk
			int endOfChunk = 0;
			unsigned char* currByte;
			currByte = malloc(1);
			unsigned char* nextByte;
			nextByte = malloc(1);
			while (!endOfChunk) { //run through super chunk to find next one
				fread(currByte, 1, 1, f);
				if (*currByte == 0xff) {
					fread(nextByte, 1, 1, f);
					if (*nextByte != 0x00) {//at next marker
						marker[0] = *currByte;
						marker[1] = *nextByte;
						endOfChunk = 1;
						free(currByte);
						free(nextByte);
					}
				}
			}
		} else {
			//printf("in standard chunk\n");
			//printf("markerCheck: %x\n", markerCheck);
			//marker is standard
			unsigned char *length;
			length = malloc(2);
			fread(length, 1, 2, f);
			unsigned int len = 256*length[0] + length[1];
			len = len - 2; //length of data
			//printf("len: %x\n", len);
			unsigned char *data;
			data = malloc(len);
			fread(data, 1, len, f);
			if (markerCheck == APP1) {
				//is Tiff file
				printf("found APP1\n");
				return 0;
			} else {
				free(data);
				fread(marker, 1, 2, f);
			}
		}
	}
		
	
	
    return -1;
}
