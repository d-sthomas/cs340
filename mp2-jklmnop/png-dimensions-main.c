#include "png-dimensions.h"

int main(int argc, char *argv[]) {
  // Ensure the correct number of arguments:
  if (argc != 2) {
    printf("Usage: %s <PNG File>\n", argv[0]);
    return 1;
  }

  // Find the PNG's dimensions:
	uint32_t w, h;
  int result = png_dimensions(argv[1], &w, &h);

  // Write them out on success:
	if (result == 0) {
		printf("%dx%d\n", w, h);
	}
	return result;
}
