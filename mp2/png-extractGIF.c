#include "png-extractGIF.h"

int png_extractGIF(const char *png_filename, const char *gif_filename) {
    // Open the file specified in argv[1] for reading:
  PNG *png = PNG_open(png_filename, "r");
  if (!png) { return ERROR_INVALID_FILE; }
  printf("PNG Header: OK\n");  
  // used as bool to make sure program runs properly
  int found = 1;
  // Read chunks until reaching "IEND" or an invalid chunk:
  while (1) {
    // Read chunk and ensure we get a valid result (exit on error):
    PNG_Chunk chunk;
    if (PNG_read(png, &chunk) == 0) {
      PNG_close(png);
      return ERROR_INVALID_CHUNK_DATA;
    }

    // Report data about the chunk to the command line:
    printf("Chunk: %s (%d bytes of data)\n", chunk.type, chunk.len);
    
    if (strcmp(chunk.type, "uiuc") == 0) { // if chunk type is uiuc
      FILE* f = fopen(gif_filename, "w");
      // write data to gif file
      fwrite(chunk.data, chunk.len, 1, f);
      fclose(f);    
      found = 0;
    }
    // Check for the "IEND" chunk to exit:
    if (strcmp(chunk.type, "IEND") == 0 ) {
      PNG_free_chunk(&chunk);
      break;  
    }
    // Free the memory associated with the chunk we just read:
    PNG_free_chunk(&chunk);
  }
  PNG_close(png);
  return found;  // Change the to a zero to indicate success, when your implementaiton is complete.
}

