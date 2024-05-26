#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

int png_dimensions(const char *png_filename, uint32_t *width, uint32_t *height) {
	FILE* file = fopen(png_filename, "r");
	if (!file) return 1; // if not valid filename	
	unsigned char sign[4];
	fread(sign, 4, 1, file); // reads signature
	
	if (fread(sign, 1, 4, file) != 4) { // if not actual png
		return 1;
	}
	unsigned char len[4]; // reads length
	fread(len, 4, 1, file);
	
	unsigned char type[4]; // reads type of chunk
	fread(type, 4, 1, file);

	unsigned char w[4];
	fread(w, 4, 1, file);
	*width = w[3] + (w[2] << 8) + (w[1] << 16) + (w[0] << 24);
	unsigned char h[4];
	fread(h, 4, 1, file);
	*height = h[3] + (h[2] << 8) + (h[1] << 16) + (h[0] << 24);
	return 0;
}
