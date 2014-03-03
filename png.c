#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zlib.h>
#include "png.h"

/*
 * Analyze a PNG file.
 * If it is a PNG file, print out all relevant metadata and return 0.
 * If it isn't a PNG file, return -1 and print nothing.
 */
 
// converts length-4 unsigned char array to an unsigned int 
unsigned int cAtoI(unsigned char* a){
	return 65536*a[0] + 4096*a[1] + 256*a[2] + a[3];
}
// reverses a length-4 unsigned char array
void cAreverse(unsigned char* a) {
	unsigned char temp = a[0];
	a[0] = a[3];
	a[3] = temp;
	temp = a[1];
	a[1] = a[2];
	a[2] = temp;
}
// prints an unsigned char array in hex 
void printHex(unsigned char* a, size_t len) {
	int i = 0;
	for (i = 0; i < len; i++) {
		printf("%02x", a[i]);
	}
	printf("\n");
}
 
int analyze_png(FILE *f) {

	//====================
	// Get the file size
	//====================
	long size;
    fseek (f, 0, SEEK_END);
    size=ftell (f);
	int currSize = 0;
    rewind(f);
	//====================
	// Check if PNG is valid (PNG and IEND present)
	//====================
    unsigned char* chars = malloc(8);
    unsigned char* IEND = malloc(12);
    fseek(f, -12, SEEK_END);
    if(fread(IEND, 1, 12, f) != 12) {
    	return -1;
    }
    if (IEND[0] != 0x00 || IEND[1] != 0x00 || IEND[2] != 0x00 || IEND[3] != 0x00 || 
    	IEND[4] != 0x49 || IEND[5] != 0x45 || IEND[6] != 0x4e || IEND[7] != 0x44 || 
    	IEND[8] != 0xae || IEND[9] != 0x42 || IEND[10] != 0x60 || IEND[11] != 0x82) {
    	printf("No IEND chunk\n");
    	return -1;
    }
    rewind(f);
    if(fread(chars, 1, 8, f) != 8) {
    	return -1;
    }
    else{
    	currSize += 8;
    }
    if (chars[0] != 0x89 || chars[1] != 0x50 ||	chars[2] != 0x4e || chars[3] != 0x47 ||
    	chars[4] != 0x0d ||	chars[5] != 0x0a ||	chars[6] != 0x1a ||	chars[7] != 0x0a) {	
    	return -1;
    }
    free(chars);
    free(IEND);
    unsigned char* length;
    unsigned char* typeData;
    unsigned char* checksum;
    char tIME = 0; // tIME counter => if > 1, invalid PNG 
    
    while(!feof(f) && currSize < size-12){

    	//====================
    	// Parse the length
    	//====================
		length = malloc(4);
		if (length == NULL) {
			printf("Malloc error\n");
			exit(1);
		}
		if (fread(length, 1, 4, f) != 4) {
			printf("Fread error, CURRSIZE: %u SIZE: %ld\n", currSize, size);
			return -1;
		}
		else{
			currSize += 4;
		}
		int len = cAtoI(length);
		if (len + currSize > size) {
			printf("Invalid size\n");
			return -1;
		}
    	//====================
    	// Parse the type and data
    	//====================
    	typeData = malloc(4 + len);
    	if (typeData == NULL) {
    		printf("Malloc error\n");
			exit(1);
		}
		if (fread(typeData, 1, 4 + len, f) != len+4) {
			return -1;
		}
		else{
			currSize += len+4;
		}
		//====================
    	// Parse the checksum
    	//====================
		checksum = malloc(4);
		if (checksum == NULL) {
			printf("Malloc error\n");
			exit(1);
		}
		if (fread(checksum, 1, 4, f) != 4) {
			return -1;
		}	
		else{
			currSize += 4;
		}
		cAreverse(checksum);
		//====================
    	// If the type is recognized
    	//====================
    	//type tExt
		if (typeData[0] == 0x74 && typeData[1] == 0x45 && 
			typeData[2] == 0x58 && typeData[3] == 0x74) {
		   	//====================
			// Compare the checksum	
	    	//====================
    		int getData = 0;
    		uLong crc = crc32(0L, Z_NULL, 0);
    		crc = crc32(crc, typeData, 4 + len);	
			if (cAtoI(checksum) != cAtoI((unsigned char *)&crc)) {
				printf("CHECKSUM mismatch\n");
				return -1;
			}
			else{
    			getData = len;
	    	}
    		//====================
    		// assert that the data contains a 0x00 nul char
	    	//====================
    		char nulFound = 0;
    		int i = 0;
    		for (i = 4; i < len; i++) {
    			if (typeData[i] == 0x00) {
    				nulFound = 1;
    			}
    		}
    		if (!nulFound) {
    			return -1;
    		}
	    	//====================
	    	// Loop over data and output
    		//====================
    		int index = 4;
	    	while((getData) && index < 4 + len){
    			if (typeData[index] == 0x00) {
	    			printf(": ");
	    		}
	    		else{
    				printf("%c", typeData[index]);
				}
				getData--;
	    		index++;
    		}
    		printf("\n");		
		}
		//type zTXt
		else if (typeData[0] == 0x7a && typeData[1] == 0x54 && 
				typeData[2] == 0x58 && typeData[3] == 0x74) {
		   	//====================
			// Compare the checksum	
	    	//====================
    		int getData = 0;
    		uLong crc = crc32(0L, Z_NULL, 0);
    		crc = crc32(crc, typeData, 4 + len);	
			if (cAtoI(checksum) != cAtoI((unsigned char *)&crc)) {
				printf("CHECKSUM mismatch\n");
				return -1;
			}
			else{
    			getData = len;
	    	}
	    	//====================
    		// assert that the data contains two sequential 0x00 nul chars
	    	//====================
    		char nulFound = 0;
    		int i = 0;
    		for (i = 4; i < len-1; i++) {
    			if (typeData[i] == 0x00 && typeData[i+1] == 0x00) {
    				nulFound++;
    			}
    		}
    		if (!nulFound) {
    			return -1;
    		}
			//====================
			// Loop over data and output
			//====================
			if (getData){
				int index = 4;
				while(typeData[index] != 0x00) {
					printf("%c", typeData[index]);
					index++;
				}
				printf(": ");
				index+=2;
				uLongf size = len+4;
				unsigned char *value = malloc(size);
				if (value == NULL) {
					printf("Malloc error\n");
					exit(1);
				}
				while(uncompress(value, &size, typeData+index, len+4-index) != Z_OK) {
					free(value);
					value = malloc(size*2);
					if (value == NULL) {
						printf("Malloc error\n");
						exit(1);
					}
					size *= 2;
				}
				int i = 0;
				for (i = 0; i < size; i++) {
					printf("%c", value[i]);
				}
				printf("\n");
				free(value);
    		}
		}
		// type tIME
		else if (typeData[0] == 0x74 && typeData[1] == 0x49 && 
			typeData[2] == 0x4d && typeData[3] == 0x45){
			// checks for invalidity
			if (tIME > 1 || len != 7) {
				return -1;
			}
			tIME++;
		   	//====================
			// Compare the checksum	
	    	//====================
    		int getData = 0;
    		uLong crc = crc32(0L, Z_NULL, 0);
    		crc = crc32(crc, typeData, 4 + len);	
			if (cAtoI(checksum) != cAtoI((unsigned char *)&crc)) {
				printf("CHECKSUM mismatch\n");
				return -1;
			}
			else{
    			getData = len;
	    	}	
	    	//====================
	    	// Loop over data and output
    		//====================
			int index = 4;
    		printf("Timestamp: ");
    		if (getData) {
    			//printf("%u %u\n", typeData[index], typeData[index+1]);
    			unsigned int year = 256*typeData[index]+typeData[index+1];
    			unsigned int month = (unsigned int)typeData[index+2];
    			unsigned int day = (unsigned int)typeData[index+3];
    			unsigned int hour = (unsigned int)typeData[index+4];
    			unsigned int minute = (unsigned int)typeData[index+5];
    			unsigned int second = (unsigned int)typeData[index+6];
    			printf("%u/%u/%u %u:%u:%u", month, day, year, hour, minute, second);
			}
    		printf("\n");		
		}
	free(length);
	free(typeData);
	free(checksum);
    }
    return 0;
}
