#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "jpg.h"
#include "utility.h"

/*
 * Analyze a JPG file that contains Exif data.
 * If it is a JPG file, print out all relevant metadata and return 0.
 * If it isn't a JPG file, return -1 and print nothing.
 */
 
 
int validTagId(unsigned char* a) {
	int rv = 0;
	unsigned char least = a[0];
	unsigned char most = a[1];
	//documentName
	if (most == 0x01 && least == 0x0d) {
		printf("DocumentName: ");
		rv = 1;
	}
	//ImageDescription
	else if (most == 0x01 && least == 0x0e) {
		printf("ImageDescription: ");
		rv = 2;
	}
	//Make
	else if (most == 0x01 && least == 0x0f) {
		printf("Make: ");
		rv = 3;
	}
	//Model
	else if (most == 0x01 && least == 0x10) {
		printf("Model: ");
		rv = 4;
	}
	//Software
	else if (most == 0x01 && least == 0x31) {
		printf("Software: ");
		rv = 5;
	}
	//DateTime
	else if (most == 0x01 && least == 0x32) {
		printf("DateTime: ");
		rv = 6;
	}
	//Artist
	else if (most == 0x01 && least == 0x3b) {
		printf("Artist: ");
		rv = 7;
	}
	//HostComputer
	else if (most == 0x01 && least == 0x3c) {
		printf("HostComputer: ");
		rv = 7;
	}
	//Copyright
	else if (most == 0x82 && least == 0x98) {
		printf("Copyright: ");
		rv = 8;
	}
	//RelatedSoundFile
	else if (most == 0xa0 && least == 0x04) {
		printf("RelatedSoundFile: ");
		rv = 9;
	}
	//DateTimeOriginal
	else if (most == 0x90 && least == 0x03) {
		printf("DateTimeOriginal: ");
		rv = 10;
	}
	//DateTimeDigitized
	else if (most == 0x90 && least == 0x04) {
		printf("DateTimeDigitized: ");
		rv = 11;
	}
	//MakerNote
	else if (most == 0x92 && least == 0x7c) {
		printf("MakerNote: ");
		rv = 12;
	}
	//UserComment
	else if (most == 0x92 && least == 0x86) {
		printf("UserComment: ");
		rv = 13;
	}
	//ImageUniqueID
	else if (most == 0xa4 && least == 0x20) {
		printf("ImageUniqueID: ");
		rv = 14;
	}
	return rv;
}
 
 
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
		} 
		else {
			//printf("in standard chunk\n");
			//printf("markerCheck: %x\n", markerCheck);
			//marker is standard
			unsigned char *length;
			length = malloc(2);
			fread(length, 1, 2, f);
			unsigned int len = 256*length[0] + length[1];
			len = len - 2; //length of data
			//printf("len: %u\n", len);
			unsigned char *data;
			data = malloc(len);
			fread(data, 1, len, f);
			if (markerCheck == APP1) {
				//is Tiff file
				printf("APP1 found\n");
				printf("HEX DUMP of Data\n");
				printHex(data, len);
				/*
				printf("PRINT AS CHAR\n\n");
				for (int i = 0; i < len; i++) {
					printf("%c", data[i]);
				}
				*/
				int tiffIndex = 0;
				for (int i = 0; i < len-6; i++) {
					if (data[i] == 0x45 && data[i+1] == 0x78 && 
						data[i+2] == 0x69 && data[i+3] == 0x66 && 
						data[i+4] == 0x00 && data[i+5] == 0x00) {
						tiffIndex = i+6;
					}
				
				}

				printf("\n\nTIFF Index: %d\n", tiffIndex);
				unsigned char* endianness = &data[tiffIndex];
				unsigned char* magicString = &data[tiffIndex+2];
				unsigned char* offset = &data[tiffIndex+4];
				cAreverse(offset);
				unsigned int IFDpos = tiffIndex + cAtoI(offset);
				unsigned int currPos = IFDpos;
				unsigned int numTags = 256*data[IFDpos+1] + data[IFDpos];
				currPos+=2;
				printf("\n\nEndianness: ");
				printHex(endianness, 2);
				printf("Magic String: ");
				printHex(magicString, 2);
				printf("Offset: ");
				printHex(offset, 4);
				printf("IFD start: %u\n", IFDpos);
				printf("# of Tags: %u\n\n", numTags);
								
				unsigned int i = 0;
				while(i<numTags) {
					unsigned char* tagid = &data[currPos];
					currPos+=2;
					//unsigned char* datatype = &data[currPos];
					currPos+=2;
					unsigned char* count = &data[currPos];
					currPos+=4;
					unsigned char* offsetOrValue = &data[currPos];
					currPos+=4;

					if(validTagId(tagid)) {
						/*
						printf("Tag #%u\n", i);
						printf("Tagid: ");
						printHex(tagid, 2);
						printf("DatTy: ");
						printHex(datatype, 2);
						printf("Count: ");
						printHex(count, 4);
						printf("OfVal: ");
						printHex(offsetOrValue, 4);
						printf("\n");
						*/
						cAreverse(count);
						cAreverse(offsetOrValue);
						unsigned int offsetInt = cAtoI(offsetOrValue);
						unsigned int countInt = cAtoI(count);
						//printf("%u\n", offsetInt);
						unsigned dataIndex = tiffIndex + offsetInt;
						for (int k = 0; k < countInt; k++) {
							printf("%c", data[dataIndex+k]);
						}
						printf("\n");
						
					}
					i++;
				}
				return 0;
			} else {
				free(data);
				fread(marker, 1, 2, f);
			}
		}
	}
    return -1;
}
