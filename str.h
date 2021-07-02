#ifndef STR_H
#define STR_H

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

// my debug header file
#ifdef DEBUG
#include "dbg.h"
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
        .size = strlen(buffer)
    };
}

static inline void str_print(str_t *str)
{
    printf(STR_FMT, STR_ARG(str));
}

static inline void str_free(str_t *pstr)
{
    free(pstr->buf);    
}

static inline void pstr_free(str_t *pstr)
{
    free(pstr->buf);    
    free(pstr);
}

static str_t * new_pstr(char *buffer) 
{
    str_t *str = (str_t *)malloc(sizeof(str_t));
    assert(str);
    str->size = strlen(buffer);

    str->buf = (char *)malloc(sizeof(char) * (str->size +1));
    assert(str->buf);

    strcpy(str->buf, buffer);

    return str;
}

static inline bool str_cmp(str_t *a, str_t *b) 
{
    if (a->size != b->size) return false;

    for (size_t i = 0; i < a->size; i++) 
        if (a->buf[i] != b->buf[i]) 
            return false;
    return true;
}

size_t _file_get_size(char *file_path)
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

static inline str_t str_read_file_to_str(char *file_path)
{
    size_t size = _file_get_size(file_path); 
    assert(size > 0);
    char *buffer = (char *)malloc(size);
    if (buffer == NULL) {
        fprintf(stderr, "%s: malloc failed\n", __func__);
        exit(1);
    }

    FILE *fp = fopen(file_path, "r");
    if (fp == NULL) {
        fprintf(stderr, "%s: failed to open file\n", __func__);
        exit(1);
    }
    fread(buffer, size, 1, fp);
    fclose(fp);

    return (str_t) {
        .buf = buffer,
        .size = size-1 
    };
}

// Returns the pos of the word in buffer
static inline int str_is_string_in_buffer(str_t *word, str_t *buffer)
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



#endif
