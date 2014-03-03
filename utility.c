#include <stdio.h>

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
		printf("%02x ", a[i]);
	}
	printf("\n");
}