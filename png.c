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
 
unsigned int cAtoI(unsigned char* a, size_t len) {
	unsigned int rv = 0x00000000;
	for (int i = 0; i < len; i++) {
		rv = rv<<2;
		rv |= a[i];
	}
	return rv;
}

 
void cAreverse(unsigned char* a) {
	unsigned char temp = a[0];
	a[0] = a[3];
	a[3] = temp;
	temp = a[1];
	a[1] = a[2];
	a[2] = temp;
}
 
void printHex(unsigned char* a, size_t len) {
	for (int i = 0; i < len; i++) {
		printf("%02x", a[i]);
	}
	printf("\n");
}
 
int analyze_png(FILE *f) {
    
    //====================
    // Check if it's a PNG file
    //====================
    unsigned char chars[8];
    fread(chars, 1, 8, f);
    if (chars[0] != 0x89 || chars[1] != 0x50 ||	chars[2] != 0x4e || chars[3] != 0x47 ||
    	chars[4] != 0x0d ||	chars[5] != 0x0a ||	chars[6] != 0x1a ||	chars[7] != 0x0a) {	
    	return -1;
    }
    unsigned char* length;
    unsigned char* typeData;
    unsigned char* checksum;
    
    
    while(!feof(f)){
    	//====================
    	// Parse the length
    	//====================
		length = malloc(4);
		fread(length, 1, 4, f);
    	//====================
    	// Parse the type and data
    	//====================
    	typeData = malloc(4 + cAtoI(length, 4));
		fread(typeData, 1, 4 + cAtoI(length, 4), f);
		//====================
    	// Parse the checksum
    	//====================
		checksum = malloc(4);
		fread(checksum, 1, 4, f);
		cAreverse(checksum);

		//====================
    	// Parse the checksum if the type is recognized
    	//====================
    	//type tExt
		if (typeData[0] == 0x74 && typeData[1] == 0x45 && 
			typeData[2] == 0x58 && typeData[3] == 0x74) {

			int len = cAtoI(length, 4);
		   	//====================
			// Compare the checksum	
	    	//====================
    		int getData = 0;
    		uLong crc = crc32(0L, Z_NULL, 0);
    		crc = crc32(crc, typeData, 4 + len);	
			if (cAtoI(checksum, 4) != cAtoI((unsigned char *)&crc, 4)) {
				printf("CHECKSUM mismatch\n");
				return -1;
			}
			else{
    			getData = (int)length;
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

			int len = cAtoI(length, 4);
		   	//====================
			// Compare the checksum	
	    	//====================
    		int getData = 0;
    		uLong crc = crc32(0L, Z_NULL, 0);
    		crc = crc32(crc, typeData, 4 + len);
			if (cAtoI(checksum, 4) != cAtoI((unsigned char *)&crc, 4)) {
				printf("CHECKSUM mismatch\n");
				return -1;
			}
			else{
    			getData = (int)length;
	    	}

	    	//====================
	    	// Loop over data and output
    		//====================
    		int index = 4;
    		while(typeData[index] != 0x00) {
    			printf("%c", typeData[index]);
    			index++;
    		}
    		printf(": ");
    		index+=2;
    		uLongf size = len+4;
    		
    		unsigned char *value = malloc(size);
    		
    		while(uncompress(value, &size, typeData+index, len+4-index) != Z_OK) {
				free(value);
				value = malloc(size*2);
				size *= 2;
    		}
    		for (int i = 0; i < size; i++) {
    			printf("%c", value[i]);
    		}
    		printf("\n");		
		}
		// type tIME
		else if (typeData[0] == 0x74 && typeData[1] == 0x49 && 
			typeData[2] == 0x4d && typeData[3] == 0x45) {

			int len = cAtoI(length, 4);
		   	//====================
			// Compare the checksum	
	    	//====================
    		int getData = 0;
    		uLong crc = crc32(0L, Z_NULL, 0);
    		crc = crc32(crc, typeData, 4 + len);
			if (cAtoI(checksum, 4) != cAtoI((unsigned char *)&crc, 4)) {
				printf("CHECKSUM mismatch\n");
				return -1;
			}
			else{
    			getData = (int)length;
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
    }
    return 0;
}
