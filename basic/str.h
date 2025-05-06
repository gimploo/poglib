#pragma once
#include "dbg.h"
#include "./common.h"
#include "./file.h"

typedef struct str_t {

    size_t    len;
    char      *data;
    bool      __is_heap_allocated;

} str_t ;

#define         str(STRING)              (str_t ) { .data = STRING, .len = strlen(STRING), .__is_heap_allocated = false }
str_t           str_init(const char *__buffer);
void            str_free(str_t *x);
void            str_print(str_t *str);
void            str_get_data(const str_t *data, char *output);
u32             str_is_string_in___buffer(str_t *word, str_t *__buffer);
u32             str_where_is_string_in___buffer(str_t *word, str_t *__buffer);
str_t           str_read_file_to_str(const char *file_path);
str_t           str_cpy_delimiter(str_t *__buffer, char ch);
bool            str_cmp(const str_t *a, const str_t *b);
void            str_cpy(str_t *dest, str_t *source);
str_t           str_get_directory_path(const char *string);

void            cstr_combine_path(const char *path1, const char *path2, char output[32]);
void            cstr_copy(char *dest, const char *source);
void            cstr_get_file_extension(const char *filepath, char output[32]);

#define         STR_FMT         "%.*s"
#define         STR_ARG(pstr)   (u32)((pstr)->len+1),(pstr)->data

#ifndef IGNORE_STR_IMPLEMENTATION

str_t str_init(const char * const __buffer) 
{
    assert(__buffer);
    str_t s = {
        .len = strlen(__buffer),
        .data = (char *)calloc(sizeof(char), strlen(__buffer) + 1),
        .__is_heap_allocated = true
    };

    memcpy(s.data, __buffer, strlen(__buffer));

    return s;
}

void str_free(str_t *x)
{
    if (x->__is_heap_allocated) {

        free(x->data);
        x->data = NULL;

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

    memcpy(dest->data, source->data, source->len);
    dest->data[source->len+1] = '\0';
    dest->len = source->len;
}

bool str_cmp(const str_t *a, const str_t *b) 
{
    assert(a);
    assert(b);

    if (a->len != b->len) return false;

    for (size_t i = 0; i < a->len; i++) 
        if (a->data[i] != b->data[i]) 
            return false;
    return true;
}

str_t str_cpy_delimiter(str_t *__buffer, char ch)
{
    assert(__buffer);

    str_t word = {0};
    char bc; size_t i = 0;
    while((bc = __buffer->data[i]) != ch)
        word.data[i++] = bc;
    word.data[i] = '\0';
    word.len = i;

    return word;
}

str_t str_read_file_to_str(const char *file_path)
{
    //NOTE: this code works dont tinker
    
    size_t size = file_get_size(file_path); 
    assert(size > 0);

    //NOTE: the +1 hold the null character
    char *__buffer = (char *)malloc(size+1);
    if (__buffer == NULL) {
        fprintf(stderr, "%s: malloc failed\n", __func__);
        exit(1);
    }

    FILE *fp = fopen(file_path, "r");
    if (fp == NULL) {
        fprintf(stderr, "%s: failed to open file\n", __func__);
        exit(1);
    }

    //NOTE: here the the contents in the file including the
    //null character is copied over to __buffer
    fread(__buffer, size, 1, fp);

    //NOTE: being extra carefull to ensure its null terminated (optional)
    __buffer[size] = '\0';
    fclose(fp);

    return str_init(__buffer);
}

// Returns the pos of the word in __buffer
u32 str_where_is_string_in___buffer(str_t *word, str_t *__buffer)
{
    //TODO: account for null characters
    
    assert(word);
    assert(__buffer);
    size_t i = 0;
    while (i < __buffer->len) {

        if (__buffer->data[i] == word->data[0]) {

            for (size_t j = 0, tmp = i; 
                    j < word->len; 
                    j++, tmp++)
            {
                if (word->data[j] != __buffer->data[tmp]) break;
                else if (j == (word->len - 1)) return i;
            }
        }
        i++;
    }
    return -1;
}

u32 str_is_word_in___buffer(str_t *word, str_t *__buffer)
{
    //TODO: account for null characters
    
    assert(word);
    assert(__buffer);
    if (__buffer->len < word->len) return false;
    size_t i = 0;
    while (i < __buffer->len) {

        if (__buffer->data[i] == word->data[0]) {

            for (size_t j = 0, tmp = i; 
                    j < word->len; 
                    j++, tmp++)
            {
                if (word->data[j] != __buffer->data[tmp]) break;
                else if (j == (word->len - 1)) return true;
            }
        }
        i++;
    }
    return false;
}

str_t str_get_directory_path(const char *string)
{
    assert(string);
    const u32 len = strlen(string);

    str_t o = {
        .len = 0,
        .data = (char *)string,
    };

#if defined(_WIN64)
    const char delimiter = '\\';
#else
    const char delimiter = '/';
#endif

    for (u32 i = len; i >= 0; i--)
    {
        if (string[i] == delimiter) {
            o.len = i;
            break;
        }
    }

    return o;
}

void cstr_combine_path(const char *path1, const char *path2, char output[32])
{
    memset(output, 0, sizeof(char [32]));

    u32 len = strlen(path1);
    if (path1[len - 1] == '\\' || path1[len - 1] == '/') 
        len--;
#if defined(_WIN64)
    snprintf(output, sizeof(char [32]), "%.*s\\%s",len, path1, path2);
#else
    snprintf(output, sizeof(char [32]), "%.*s/%s", len, path1, path2);
#endif
}

void cstr_copy(char *dest, const char *source)
{
    memcpy(dest, source, strlen(source) + 1);
}

void str_get_data(const str_t *data, char *output)
{
    memcpy(output, data->data, data->len);
}

//credit: gunslinger
void cstr_get_file_extension(const char *filepath, char output[32])
{
    uint32_t str_len = strlen(filepath);
    const char* at = (filepath + str_len - 1);
    while (*at != '.' && at != filepath)
    {
        at--;
    }

    if (*at == '.')
    {
        at++;
        uint32_t i = 0; 
        while (*at)
        {
            char c = *at;
            output[i++] = *at++;
        }
        output[i] = '\0';
    }
}

#endif
