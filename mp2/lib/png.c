#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <math.h>

#include "crc32.h"
#include "png.h"

const int ERROR_INVALID_PARAMS = 1;
const int ERROR_INVALID_FILE = 2;
const int ERROR_INVALID_CHUNK_DATA = 3;
const int ERROR_NO_UIUC_CHUNK = 4;


/**
 * Opens a PNG file for reading (mode == "r" or mode == "r+") or writing (mode == "w").
 * 
 * (Note: The function follows the same function prototype as `fopen`.)
 * 
 * When the file is opened for reading this function must verify the PNG signature.  When opened for
 * writing, the file should write the PNG signature.
 * 
 * This function must return NULL on any errors; otherwise, return a new PNG struct for use
 * with further fuctions in this library.
 */
PNG * PNG_open(const char *filename, const char *mode) {
  char sign[8];
  FILE* file = fopen(filename, mode);
  const char *dot = strrchr(filename, '.');
  // if reading file
  if (strcmp(mode, "w") != 0) {
    // signature should be 8 bytes long
    // filename should also end in .png
    if ((!file)|| fread(sign, 1, 8, file) != 8 || strcmp(dot, ".gif") == 0) {
      if (file) fclose(file);
      return NULL;
    }
  } else {
    // if writing file
    unsigned char *buffer = (unsigned char *) malloc(8);
    buffer[0] = 0x89;
    buffer[1] = 0x50;
    buffer[2] = 0x4e;
    buffer[3] = 0x47;
    buffer[4] = 0x0d;
    buffer[5] = 0x0a;
    buffer[6] = 0x1a;
    buffer[7] = 0x0a;
    fwrite(buffer,sizeof(buffer),1, file);
    free(buffer);
  }
  PNG* png = malloc(sizeof *png);
  png->img = file;
  return png;
}

/**
 * Reads the next PNG chunk from `png`.
 * 
 * If a chunk exists, a the data in the chunk is populated in `chunk` and the
 * number of bytes read (the length of the chunk in the file) is returned.
 * Otherwise, a zero value is returned.
 * 
 * Any memory allocated within `chunk` must be freed in `PNG_free_chunk`.
 * Users of the library must call `PNG_free_chunk` on all returned chunks.
 */
size_t PNG_read(PNG *png, PNG_Chunk *chunk) {
  
  int read = 0; // bytes read
  // initialize/reset chunk variables
  chunk->len = 0;
  memset(chunk->type, '\0', 4);
  chunk->crc = 0;
  chunk->data = NULL;
  if (feof(png->img)) return 0;
  
  fread(&chunk->len, 4, 1, png->img);  
  chunk->len = htonl(chunk->len);
  read += 4;
  // gets chunk type
  fread(chunk->type, 4, 1, png->img);
  chunk->type[4] = '\0';
  read += 4;
  // gets chunk data
  chunk->data = calloc(chunk->len, 1); 
  fread(chunk->data, chunk->len, 1, png->img);
  
  read += chunk->len;
  // gets chunk crc
  fread(&chunk->crc, 4, 1, png->img);
  
  chunk->crc = htonl(chunk->crc); // shift the bits
  read += 4;
  
  return read;
}

/**
 * Writes a PNG chunk to `png`.
 * 
 * Returns the number of bytes written. 
 */
size_t PNG_write(PNG *png, PNG_Chunk *chunk) {
  // chunk length to bytes
  uint32_t x = chunk->len;
  unsigned char length[4];
  length[3] = x & 0xFF;
  length[2] = (x >> 8)  & 0xFF;
  length[1] = (x >> 16) & 0xFF;  
  length[0] = (x >> 24) & 0xFF;
  
  fwrite(length, 4, 1, png->img);
  // chunk type to bytes
  unsigned char t[4];
  for (int i = 0; i < 4; i++) {
    t[i] = chunk->type[i];
  }
  fwrite(t, 4, 1, png->img);
  // chunk data to bytes
  unsigned char d[chunk->len];
  for (unsigned int i = 0; i < chunk->len; i++) {
    d[i] = chunk->data[i];
  } 
  fwrite(d,chunk->len, 1, png->img);
  // chunk crc to bytes
  unsigned char tmp[strlen(chunk->type) + chunk->len + 1]; // array to store type and data
  int tmp_counter = 0;  
  for (int i = 0; i < 4; i++) {
    tmp[tmp_counter] = chunk->type[i];
    tmp_counter++;
  }
  for (unsigned int i = 0; i < chunk->len; i++) {
    tmp[tmp_counter] = chunk->data[i];
    tmp_counter++;
  }
  tmp[chunk->len + 4] = 0;
  
  chunk->crc = 0;
  crc32(tmp, chunk->len + 4, &chunk->crc); // converts array to crc
  chunk->crc = ntohl(chunk->crc); // network to host order
  

  fwrite(&chunk->crc, 4, 1, png->img);
  
  if (strcmp(chunk->type, "IEND") == 0) return 0;
  return 12 + chunk->len;
}

/**
 * Frees all memory allocated by this library related to `chunk`.
 */
void PNG_free_chunk(PNG_Chunk *chunk) {
  chunk->len = 0;
  memset(chunk->type, '\0', 4);
  free(chunk->data);
  chunk->crc = 0;
}


/**
 * Closes the PNG file and frees all memory related to `png`.
 */
void PNG_close(PNG *png) {

  fclose(png->img);
  free(png);
}