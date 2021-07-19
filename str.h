#ifndef STR_H
#define STR_H

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#ifdef DEBUG
#include "dbg/dbg.h"
extern dbg_t debug;
#endif

typedef struct {
    char *buf;
    size_t size;
} str_t;

#define STR_FMT "%.*s"
#define STR_ARG(pstr) (int)(pstr->size+1),pstr->buf


static inline str_t new_str(char *buffer) 
{
    return (str_t) {
        .buf = buffer,
        .size = strlen(buffer),
    };
}

static inline void str_print(str_t *str)
{
    assert(str);
    printf(STR_FMT, STR_ARG(str));
}

static inline void pstr_free(str_t *str)
{
    assert(str);

    free(str->buf);    
    str->buf = NULL;

    free(str);
    str = NULL;
}


static str_t * new_pstr(char *buffer) 
{
    str_t *str = (str_t *)malloc(sizeof(str_t));
    assert(str);

    size_t buffer_size = sizeof(char) * (strlen(buffer) + 1);

    str->buf = (char *)malloc(buffer_size);
    assert(str->buf);

    memset(str->buf, 0, buffer_size);

    strncpy(str->buf, buffer, buffer_size-1);


    str->size = buffer_size - 1;

    return str;
}

static inline void str_cpy(str_t *dest, str_t *source)
{
    assert(dest);
    assert(source);

    memcpy(dest->buf, source->buf, source->size);
    dest->buf[source->size+1] = '\0';
    dest->size = source->size;
}

static inline bool str_cmp(str_t *a, str_t *b) 
{
    assert(a);
    assert(b);

    if (a->size != b->size) return false;

    for (size_t i = 0; i < a->size; i++) 
        if (a->buf[i] != b->buf[i]) 
            return false;
    return true;
}

static str_t str_cpy_delimiter(str_t *buffer, char ch)
{
    assert(buffer);

    str_t word = {0};
    char bc; size_t i = 0;
    while((bc = buffer->buf[i]) != ch)
        word.buf[i++] = bc;
    word.buf[i] = '\0';
    word.size = i;

    return word;
}

size_t _file_get_size(const char *file_path)
{
    FILE *fp = fopen(file_path, "r");
    if (fp == NULL) {
        fprintf(stderr, "%s: file failed to open\n", __func__);
        exit(1);
    }
    fseek(fp, 0L, SEEK_END);
    size_t size = ftell(fp);
    fclose(fp);
    return size; // Returns the position of the null character 
}

static inline str_t * str_read_file_to_str(const char *file_path)
{
    //NOTE: this code works dont tinker
    
    size_t size = _file_get_size(file_path); 
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


    str_t *str_file = (str_t *)malloc(sizeof(str_t));
    str_file->buf = buffer;
    str_file->size = size;

    return str_file;
}

// Returns the pos of the word in buffer
static inline int str_where_is_string_in_buffer(str_t *word, str_t *buffer)
{
    //TODO: account for null characters
    
    assert(word);
    assert(buffer);
    size_t i = 0;
    while (i < buffer->size) {

        if (buffer->buf[i] == word->buf[0]) {

            for (size_t j = 0, tmp = i; 
                    j < word->size; 
                    j++, tmp++)
            {
                if (word->buf[j] != buffer->buf[tmp]) break;
                else if (j == (word->size - 1)) return i;
            }
        }
        i++;
    }
    return -1;
}

static inline int str_is_string_in_buffer(str_t *word, str_t *buffer)
{
    //TODO: account for null characters
    
    assert(word);
    assert(buffer);
    if (buffer->size < word->size) return false;
    size_t i = 0;
    while (i < buffer->size) {

        if (buffer->buf[i] == word->buf[0]) {

            for (size_t j = 0, tmp = i; 
                    j < word->size; 
                    j++, tmp++)
            {
                if (word->buf[j] != buffer->buf[tmp]) break;
                else if (j == (word->size - 1)) return true;
            }
        }
        i++;
    }
    return false;
}



#endif
