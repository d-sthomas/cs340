#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Return your favorite emoji.  Do not allocate new memory.
// (This should **really** be your favorite emoji, we plan to use this later in the semester. :))
char *emoji_favorite() {
  // U+1F480
  return "\xf0\x9f\x92\x80";
}


// Count the number of emoji in the UTF-8 string `utf8str`, returning the count.  You should
// consider everything in the ranges starting from (and including) U+1F000 up to (and including) U+1FAFF.
int emoji_count(const unsigned char *utf8str) {
  // first break string into emojis (based on \x starting and space ending)
  // then check if encoding is in range; not fully working
  int count = 0;
  char *ptr = strtok(utf8str, " ");
  while (ptr != NULL) {
    unsigned int val = 0;
    for(unsigned i=0; i<strlen(ptr); i++) {
      val = (val << 8) | ((unsigned int)(ptr[i]) & 0xFF);
    }
    if (val >= 4036984960 /* U+1F000 */ && val <= 4036996031 /* U+1FAFF */ ) {
      count ++;
    }
    ptr = strtok(NULL, " ");
  }
  free(ptr);
  return count;
}


// Return a random emoji stored in new heap memory you have allocated.  Make sure what
// you return is a valid C-string that contains only one random emoji.
char *emoji_random_alloc() {
  int random = rand() % 10777 + 4036984960; // range of emojis is 4036984960 - 4036996031
  char* emoji = malloc(5);
  emoji[0] = (random >> 24) & 0xFF;
  emoji[1] = (random >> 16) & 0xFF;
  emoji[2] = (random >> 8) & 0xFF;
  emoji[3] = random & 0xFF;
  emoji[4] = '\0';
  return emoji;
}


// Modify the UTF-8 string `utf8str` to invert the FIRST character (which may be up to 4 bytes)
// in the string if it the first character is an emoji.  At a minimum:
// - Invert "😊" U+1F60A ("\xF0\x9F\x98\x8A") into ANY non-smiling face.
// - Choose at least five more emoji to invert.
void emoji_invertChar(unsigned char *utf8str) {
  if (strcmp(utf8str, "\xF0\x9F\x98\x8A") == 0) { // happy to frown
    strcpy(utf8str, "\xF0\x9F\x99\x81");
  } else if (strcmp(utf8str, "\xf0\x9f\xa5\xb1") == 0) { // tired to surprised
    strcpy(utf8str, "\xf0\x9f\x98\xb3");
  } else if (strcmp(utf8str, "\xf0\x9f\x92\x80") == 0) { // dead to angel
    strcpy(utf8str, "\xf0\x9f\x98\x87");
  } else if (strcmp(utf8str, "\xf0\x9f\x99\x82") == 0) { // smiling to upside down
    strcpy(utf8str, "\xf0\x9f\x99\x83");
  } else if (strcmp(utf8str, "\xf0\x9f\xa5\xb5") == 0) { // hot to cold
    strcpy(utf8str, "\xf0\x9f\xa5\xb6");
  } else if (strcmp(utf8str, "\xf0\x9f\xa4\xa0") == 0) { // cowboy to mustache
    strcpy(utf8str, "\xf0\x9f\xa5\xb8");
  }
}


// Modify the UTF-8 string `utf8str` to invert ALL of the character by calling your
// `emoji_invertChar` function on each character.
void emoji_invertAll(unsigned char *utf8str) {
  char* tmp = malloc(50); // tmp to store inverted string
  tmp[0] = '\0';
  char *ptr = strtok(utf8str, " "); // read string and ignore white spaces

	while (ptr != NULL) {
		emoji_invertChar(ptr);
    strcat(tmp, ptr); // concatenate inverted emoji into tmp
    strcat(tmp, " "); // adds space since we're skipping over them while reading
		ptr = strtok(NULL, " ");
	}
  //printf("%s/n", utf8str);
  strcpy(utf8str, tmp); // sets utf8str to tmp (inverted string)
  free(ptr);
  free(tmp);
  utf8str[strlen(utf8str)-1] = '\0'; // last char is space, deletes it
}


// Reads the full contents of the file `fileName, inverts all emojis, and
// returns a newly allocated string with the inverted fixle's content.
unsigned char *emoji_invertFile_alloc(const char *fileName) {
  FILE *textfile;
  char line[1000]; // max bytes per line
  line[0] = '\0';
  char* tmp = malloc(100); 
  tmp[0] = '\0'; // stores inverted file
  textfile = fopen(fileName, "r"); // reads file
  if(textfile == NULL) {
    free(tmp);
    return NULL;
  }
  while(fgets(line, 1000, textfile)){
    emoji_invertAll(line);
    strcat(tmp, line); // concatenate inverted line to tmp
  }
  fclose(textfile);
  return tmp;
}
