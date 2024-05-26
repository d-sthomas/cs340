#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _emoji_t {
    char sources[20][50];
    char translations[20][1000];
    int amount;
    
} emoji_t;



void emoji_init(emoji_t *emoji);
void emoji_add_translation(emoji_t *emoji, const unsigned char *source, const unsigned char *translation);
int is_emoji(char *str);
char* emoji_translateChar(emoji_t *emoji, char *utf8str);
void emoji_translateAll(emoji_t *emoji,char *utf8str);
const unsigned char *emoji_translate_file_alloc(emoji_t *emoji, const char *fileName);
void emoji_destroy(emoji_t *emoji);

#ifdef __cplusplus
}
#endif
