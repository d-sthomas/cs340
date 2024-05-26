#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "emoji.h"
#include "emoji-translate.h"


void emoji_init(emoji_t *emoji) {
  emoji->amount = 0;
  for (int i = 0; i < 20; i++) {
    memset(emoji->sources[i], '\0', 50);
  }
  for (int i = 0; i < 20; i++) {
    memset(emoji->translations[i], '\0', 1000);
  }
  //printf(strlen(emoji->emoji_sources));
}

void emoji_add_translation(emoji_t *emoji, const unsigned char *source, const unsigned char *translation) {
  strcpy(emoji->sources[emoji->amount], source); // adding source to array
  strcpy(emoji->translations[emoji->amount], translation); // adding translation to array
  emoji->amount++; // updating amount/index
}
 // checks to see if char/string is actually an emoji
 // given method in test-emoji.cpp
int is_emoji(char *str) {
  unsigned int val = 0;
  for(unsigned i=0; i<strlen(str); i++) {
    val = (val << 8) | ((unsigned int)(str[i]) & 0xFF);
  }
  return (
    (val >= 14844092 /* U+203C */ && val <= 14912153 /* U+3299 */) ||
    (val >= 4036984960 /* U+1F000 */ && val <= 4036996031 /* U+1FAFF */ )
  );
}

char* emoji_translateChar(emoji_t *emoji, char *ptr) {
  if (emoji->amount == 0) return ptr;
  char* translation = malloc(500); // stores translation
  translation[0] = '\0';
  int valid_emoji = 0; // checks whether emoji has definition
  int curr = 0; // used to keep track of how many 
  int possible_translations[20]; // keeps track of which translations are in the string
  int curr_substr = 0; // used to check if there's tiebreaks
  char* sub = malloc(500);
  sub[0] = '\0';
  for (int i = 0; i < emoji->amount; i++) {
      if (strstr(ptr,  emoji->sources[i])) { // checks if emojis are a substring of passed string
        possible_translations[curr] = i;
        curr+=1;
    }
  }
  while (strlen(ptr) != 0) {
    for (int i = 0; i < curr; i++) {
      int curr_trans = possible_translations[i]; // which translation we're looking at rn
      if (strstr(ptr, emoji->sources[curr_trans]) - ptr == 0 && 
          (strlen(emoji->sources[curr_trans]) >= strlen(emoji->sources[curr_substr]) || // if its a nested translation
            strstr(ptr, emoji->sources[curr_substr]) - ptr != 0)) { // or if sources[curr_trans] is longer but not at the start
        curr_substr = curr_trans;
        valid_emoji = 1;
      }
    }
    if (valid_emoji == 1) {
      strcat(translation, emoji->translations[curr_substr]);
    } else {
      strcat(translation, ptr);
    }
    strcpy(sub, emoji->sources[curr_substr]);
    int len = strlen(sub);
    memmove(ptr, ptr + len, strlen(ptr + len) + 1); // removes the substring since we already added the translation 
    
    curr_substr = 0;
    valid_emoji = 0;
  }
  free(sub);
  return translation;
}

void emoji_translateAll(emoji_t *emoji, char *utf8str) {
  if (emoji->amount == 0) return;
  char* tmp = malloc(500); // tmp to store inverted string
  tmp[0] = '\0';
  char* ptr = strtok(utf8str, " "); // read string and ignore white spaces
	while (ptr != NULL) {
    if (is_emoji(ptr)) {
      char* translation = emoji_translateChar(emoji, ptr);
      strcat(tmp, translation);
      free(translation);
    } else {
      strcat(tmp, ptr);
    }
     // concatenate inverted emoji into tmp
    strcat(tmp, " "); // adds space since we're skipping over them while reading 
		ptr = strtok(NULL, " ");
	}
  strcpy(utf8str, tmp); // sets utf8str to tmp (inverted string)
  free(tmp);
  utf8str[strlen(utf8str)-1] = '\0'; // last char is space, deletes it
}

// Translates the emojis contained in the file `fileName`.
const unsigned char *emoji_translate_file_alloc(emoji_t *emoji, const char *fileName) {
  FILE *textfile;
  char line[5000]; // max bytes per line
  line[0] = '\0';
  char* tmp = malloc(1000); // stores inverted file
  tmp[0] = '\0';
  textfile = fopen(fileName, "r"); // reads file
  int mult_lines = 0;
    if(textfile == NULL) {
      free(tmp);
      return NULL;
    }
        
    while(fgets(line, 1000, textfile)){
      if (line[strlen(line)-1] == '\n') {
        line[strlen(line)-1] = '\0';
        mult_lines = 1;
      }
        emoji_translateAll(emoji, line);
        if (mult_lines) {
          line[strlen(line)] = '\n';
        }
        strcat(tmp, line); // concatenate inverted line to tmp
    }
    if (mult_lines) {
      tmp[strlen(tmp)-1] = 0;
    }    
    fclose(textfile);
  return tmp;
}

void emoji_destroy(emoji_t *emoji) {
  // fix this :)
  emoji->amount = 0;
}
