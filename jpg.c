#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "jpg.h"

/*
 * Analyze a JPG file that contains Exif data.
 * If it is a JPG file, print out all relevant metadata and return 0.
 * If it isn't a JPG file, return -1 and print nothing.
 */
void getValue(unsigned int index, unsigned int tagID, unsigned int dataType, unsigned int size, unsigned int count, unsigned int len, unsigned char *data) {
	//printf("in getValue\n");
	//printf("%x\n", tagID);
	unsigned int undefined = 3;
	if (tagID == 0x010d) {
		printf("DocumentName: ");
		undefined = 0;
	} else if (tagID == 0x010e) {
		printf("ImageDescription: ");
		undefined = 0;
	} else if (tagID == 0x010f) {
		printf("Make: ");
		undefined = 0;
	} else if (tagID == 0x0110) {
		printf("Model: ");
		undefined = 0;
	} else if (tagID == 0x0131) {
		printf("Software: ");
		undefined = 0;
	} else if (tagID == 0x0132) {
		printf("DateTime: ");
		undefined = 0;
	} else if (tagID == 0x013b) {
		printf("Artist: ");
		undefined = 0;
	} else if (tagID == 0x013c) {
		printf("HostComputer: ");
		undefined = 0;
	} else if (tagID == 0x8298) {
		printf("Copyright: ");
		undefined = 0;
	} else if (tagID == 0xa004) {
		printf("RelatedSoundFile: ");
		undefined = 0;
	} else if (tagID == 0x9003) {
		printf("DateTimeOriginal: ");
		undefined = 0;
	} else if (tagID == 0x9004) {
		printf("DateTimeDigitized: ");
		undefined = 0;
	} else if (tagID == 0x927c) {
		printf("MakerNote: ");
		undefined = 1;
	} else if (tagID == 0x9286) {
		printf("UserComment: ");
		if (len > index + 7) {
			if (data[index] != 0x41 || data[index + 1] != 0x53 || data[index + 2] != 0x43 
			    || data[index + 3] != 0x49 || data[index + 4] != 0x49 || data[index + 5] != 0x00 
			    || data[index + 6] != 0x00 || data[index + 7] != 0x00) {
				undefined = 3;
			} else {
				index = index + 8;
				undefined = 2;
			}
		}
	} else if (tagID == 0xa420) {
		printf("ImageUniqueID: ");
		undefined = 0;
	}
	if (undefined == 0) {
		unsigned int temp = 0;
		while (temp < count-1 && index + temp < len) {
			printf("%c", data[index + temp]);
			temp += 1;
		}
		printf("\n");
	} else if (undefined == 1) {
		unsigned int temp = 0;
		while (temp < count && index + temp < len) {
			if (data[index + temp] == 0x00) {
				temp = count;
			} else {
				printf("%c", data[index + temp]);
				temp += 1;
			}
		}
		printf("\n");
	} else if (undefined == 2) {
		unsigned int temp = 0;
		while (temp < count - 8 && index + temp < len) {
			if (data[index + temp] == 0x00) {
				temp = count;
			} else {
				printf("%c", data[index + temp]);
				temp += 1;
			}
		}
		printf("\n");
	}
}

void parseIFD(unsigned int index, unsigned int len, unsigned char *data) {
	unsigned int numTags = 256*data[index + 1] + data[index];
	index = index + 2;
	unsigned int counter = 0;
	unsigned int sizeDict[15] = {0,1,1,2,4,8,0,1,2,4,8,4,8};
	//printf("in parse\n");
	while (counter < numTags) {
		//printf("looping\n");
		if (len > index + 7) {
			unsigned int tagID = 256*data[index+1] + data[index];
			index = index + 2;
			//printf("tagID: %x\n", tagID);
			unsigned int dataType = 256*data[index+1] + data[index];
			index = index + 2;
			if (dataType > 15) {
				break;
			}
			//printf("dataType: %x\n", dataType);
			unsigned int count = 65536*data[index+3] + 4096*data[index+2] + 256*data[index+1] + data[index];
			index = index + 4;
			//printf("count: %d\n", count);
			unsigned int dataSize = count * sizeDict[dataType];
			//printf("dataSize: %d\n", dataSize);
			if (tagID == 0x8769) { //is Exif IFD
				//printf("in tagID\n");
				if (len > index + 3) {
					unsigned int offset = 65536*data[index+3] + 4096*data[index+2] + 256*data[index+1] + data[index];
					offset = offset + 6;
					parseIFD(offset, len, data);
					index = index + 4;
				} else {
					break;
				}
			} else if (dataSize <= 4) {
				//printf("in dataSize\n");
				getValue(index, tagID, dataType, sizeDict[dataType], count, len, data);
				index = index + 4;
			} else {
				//printf("in normal\n");
				if (len > index + 3) {
					unsigned int offset = 65536*data[index+3] + 4096*data[index+2] + 256*data[index+1] + data[index];
					offset = offset + 6;
					getValue(offset, tagID, dataType, sizeDict[dataType], count, len, data);
					index = index + 4;
				} else {
					break;
				}
			}
		} else {
			break;
		}
		counter += 1;
	}
}

int analyze_jpg(FILE *f) {
    /* YOU WRITE THIS PART */
	unsigned int superLowBound = 0xffd0;
	unsigned int superHighBound = 0xffda;
	unsigned int APP1 = 0xffe1;
	
	//get first marker
	unsigned char *marker;
	marker = malloc(2);
	if (marker == NULL) {
		return -1;
	}

	if (fread(marker, 1, 2, f) != 2){
		free(marker);
		return -1;
	}
	unsigned int markerCheck;
	while(!feof(f)){
		//check if chunk super or standard
		markerCheck = 256*marker[0] + marker[1];
		//printf("markerCheck: %x\n", markerCheck);
		if (markerCheck >= superLowBound && markerCheck <= superHighBound) {
			//printf("in super chunk\n");
			//is super chunk
			int endOfChunk = 0;
			unsigned char* currByte;
			currByte = malloc(1);
			if (currByte == NULL) {
				free(marker);
				return -1;
			}
			unsigned char* nextByte;
			nextByte = malloc(1);
			if (nextByte == NULL) {
				free(marker);
				free(currByte);
				return -1;
			}
			while (!endOfChunk) { //run through super chunk to find next one
				if (fread(currByte, 1, 1, f) != 1){
					free(currByte);
					free(nextByte);
					free(marker);
					return -1;
				}
				if (*currByte == 0xff) {
					if (fread(nextByte, 1, 1, f) != 1){
						free(currByte);
						free(nextByte);
						free(marker);
						return -1;
					}
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
			if (length == NULL) {
				free(marker);
				return -1;
			}
			if (fread(length, 1, 2, f) != 2){
				free(length);
				free(marker);
				return -1;
			}
			unsigned int len = 256*length[0] + length[1];
			len = len - 2; //length of data
			//printf("len: %x\n", len);
			unsigned char *data;
			data = malloc(len);
			if (data == NULL) {
				free(marker);
				free(length);
				return -1;
			}
			if (fread(data, 1, len, f) != len){
				free(data);
				free(length);
				free(marker);
				return -1;
			}
			if (markerCheck == APP1 && len > 14) {
				//is Tiff file
				//printf("%x\n", data[0]);
				//printf("found APP1\n");
				if (data[0] != 0x45 || data[1] != 0x78 || data[2] != 0x69 || data[3] != 0x66 || data[4] != 0x00 || data[5] != 0x00) {
					//printf("in if\n");
					free(data);
					free(marker);
					free(length);
					return -1;
				} else {
					//printf("in else\n");
					int index = 6;
					if (data[6] != 0x49 || data[7] != 0x49 || data[8] != 0x2a || data[9] != 0x00) { //not proper tiff
						//printf("not tiff\n");
						free(data);
						free(marker);
						free(length);
						return -1;
					} else { //is proper tiff
						//printf("tiff\n");
						index = 65536*data[13] + 4096*data[12] + 256*data[11] + data[10];
						index = index + 6;
						//printf("index: %d\n", index);
						parseIFD(index, len, data);						
					}
				}
				free(data);
				free(marker);
				free(length);
				return 0;
			} else {
				free(data);
				free(length);
				if (fread(marker, 1, 2, f) != 2){
					free(marker);
					return -1;
				}
			}
		}
	}
	free(marker);
	return -1;
}
