#pragma once
#include "./basic.h"
#include "./file.h"




typedef struct str_t str_t ;


#define         str(STRING)              (str_t ) { .buf = STRING, .len = strlen(STRING), .__is_heap_allocated = false }
str_t           str_init(const char *buffer);
void            str_free(str_t *x);
void            str_print(str_t *str);
u32             str_is_string_in_buffer(str_t *word, str_t *buffer);
u32             str_where_is_string_in_buffer(str_t *word, str_t *buffer);
str_t           str_read_file_to_str(const char *file_path);
str_t           str_cpy_delimiter(str_t *buffer, char ch);
bool            str_cmp(const str_t *a, const str_t *b);
void            str_cpy(str_t *dest, str_t *source);





#ifndef IGNORE_STR_IMPLEMENTATION

#define STR_FMT         "%.*s"
#define STR_ARG(pstr)   (u32)((pstr)->len+1),(pstr)->buf

struct str_t {

    char      *buf;
    size_t    len;

    bool      __is_heap_allocated;

} ;



str_t str_init(const char * const buffer) 
{
    assert(buffer);
    str_t s = {
        .buf = (char *)calloc(1, strlen(buffer) + 1),
        .len = strlen(buffer),
        .__is_heap_allocated = true
    };

    memcpy(s.buf, buffer, strlen(buffer));

    return s;
}

void str_free(str_t *x)
{
    if (x->__is_heap_allocated) {

        free(x->buf);
        x->buf = NULL;

    } else {

        eprint("[!] WARNING: tried to free a stack allocated string");
    }
}

void str_print(str_t *str)
{
    assert(str);
    printf(STR_FMT, STR_ARG(str));
}


void str_cpy(str_t *dest, str_t *source)
{
    assert(dest);
    assert(source);

    memcpy(dest->buf, source->buf, source->len);
    dest->buf[source->len+1] = '\0';
    dest->len = source->len;
}

bool str_cmp(const str_t *a, const str_t *b) 
{
    assert(a);
    assert(b);

    if (a->len != b->len) return false;

    for (size_t i = 0; i < a->len; i++) 
        if (a->buf[i] != b->buf[i]) 
            return false;
    return true;
}

str_t str_cpy_delimiter(str_t *buffer, char ch)
{
    assert(buffer);

    str_t word = {0};
    char bc; size_t i = 0;
    while((bc = buffer->buf[i]) != ch)
        word.buf[i++] = bc;
    word.buf[i] = '\0';
    word.len = i;

    return word;
}

str_t str_read_file_to_str(const char *file_path)
{
    //NOTE: this code works dont tinker
    
    size_t size = file_get_size(file_path); 
    assert(size > 0);

    //NOTE: the +1 hold the null character
    char *buffer = (char *)malloc(size+1);
    if (buffer == NULL) {
        fprintf(stderr, "%s: malloc failed\n", __func__);
        exit(1);
    }

    FILE *fp = fopen(file_path, "r");
    if (fp == NULL) {
        fprintf(stderr, "%s: failed to open file\n", __func__);
        exit(1);
    }

    //NOTE: here the the contents in the file including the
    //null character is copied over to buffer
    fread(buffer, size, 1, fp);

    //NOTE: being extra carefull to ensure its null terminated (optional)
    buffer[size] = '\0';
    fclose(fp);

    return str_init(buffer);
}

// Returns the pos of the word in buffer
u32 str_where_is_string_in_buffer(str_t *word, str_t *buffer)
{
    //TODO: account for null characters
    
    assert(word);
    assert(buffer);
    size_t i = 0;
    while (i < buffer->len) {

        if (buffer->buf[i] == word->buf[0]) {

            for (size_t j = 0, tmp = i; 
                    j < word->len; 
                    j++, tmp++)
            {
                if (word->buf[j] != buffer->buf[tmp]) break;
                else if (j == (word->len - 1)) return i;
            }
        }
        i++;
    }
    return -1;
}

u32 str_is_string_in_buffer(str_t *word, str_t *buffer)
{
    //TODO: account for null characters
    
    assert(word);
    assert(buffer);
    if (buffer->len < word->len) return false;
    size_t i = 0;
    while (i < buffer->len) {

        if (buffer->buf[i] == word->buf[0]) {

            for (size_t j = 0, tmp = i; 
                    j < word->len; 
                    j++, tmp++)
            {
                if (word->buf[j] != buffer->buf[tmp]) break;
                else if (j == (word->len - 1)) return true;
            }
        }
        i++;
    }
    return false;
}



#endif
