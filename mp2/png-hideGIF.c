#include "png-hideGIF.h" 

int png_hideGIF(const char *png_filename_source, const char *gif_filename, const char *png_filename_out) {
  // Open the file specified in argv[1] for reading and argv[3] for writing:
  PNG *png = PNG_open(png_filename_source, "r");
  if (!png) { return ERROR_INVALID_FILE; }

  PNG *out = PNG_open(png_filename_out, "w");
  printf("PNG Header written.\n");
  size_t bytesWritten;

  // Read chunks until reaching "IEND" or in invalid chunk:
  while (1) {
    // Read chunk and ensure we get a valid result (exit on error):
    PNG_Chunk chunk;
    if (PNG_read(png, &chunk) == 0) {
      PNG_close(png);
      PNG_close(out);
      return ERROR_INVALID_CHUNK_DATA;
    }

    // Report data about the chunk to the command line:
    bytesWritten = PNG_write(out, &chunk);
    printf("PNG chunk %s written (%lu bytes)\n", chunk.type, bytesWritten);
    if (strcmp(chunk.type, "IHDR") == 0 ) {
      PNG_Chunk hide;
      // open gif to store its data
      printf("here");
      FILE *gif = fopen(gif_filename, "r");
      
      fseek(gif, 0L, SEEK_END);
      hide.len = ftell(gif); // length of gif
      rewind(gif);
      strcpy(hide.type, "uiuc");
      
      
      hide.data = malloc(hide.len);
      fread(hide.data, hide.len, 1, gif); // store gif in data
      bytesWritten = PNG_write(out, &hide);
      fclose(gif);
      PNG_free_chunk(&hide);
    }
    // Check for the "IEND" chunk to exit:
    if ( strcmp(chunk.type, "IEND") == 0 ) {
      PNG_free_chunk(&chunk);
      printf("finished\n");
      break;  
    }
    // Free the memory associated with the chunk we just read:
    PNG_free_chunk(&chunk);
  }
  PNG_close(out);
  PNG_close(png);
  return 0; // Change the to a zero to indicate success, when your implementaiton is complete.
}
