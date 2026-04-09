#pragma once
#include "dbg.h"
#include "./common.h"
#include "./file.h"
#include "./arena.h"
#include "./util.h"

typedef struct str_t {

    size_t    len;
    char      *data;
    bool      __is_manually_heap_allocated;

} str_t ;

#define         str(STRING)              (str_t ) { .data = STRING, .len = sizeof(STRING) - 1, .__is_manually_heap_allocated = false }
#define         str_lit(STRING)          { .data = STRING, .len = sizeof(STRING) - 1, .__is_manually_heap_allocated = false }
str_t           str_init(arena_t *arena, const char * const __buffer);
str_t           str__from_cstr(const char *data, const u32 len);
void            str_free(str_t *x);
void            str_print(str_t *str);
void            str_get_data(const str_t *data, char *output);
u32             str_where_is_string_in_buffer(str_t *word, str_t *__buffer);
str_t           str_read_file_to_str(arena_t *arena, const char *file_path);
str_t           str_cpy_delimiter(str_t *__buffer, char ch);
bool            str_cmp(const str_t *a, const str_t *b);
void            str_cpy(str_t *dest, str_t *source);
str_t           str_get_directory_path(const char *string);
str_t           str_join(arena_t *arena, const str_t *part1, const char *part2);

void            cstr_combine_path(const char *path1, const char *path2, char *output, const u32 output_size);
void            cstr_copy(char *dest, const char *source);
void            cstr_get_file_extension(const char *filepath, char output[32]);

#define         STR_FMT         "%.*s"
#define         STR_ARG(pstr)   (u32)((pstr)->len+1),(pstr)->data

#ifndef IGNORE_STR_IMPLEMENTATION

str_t str_init(arena_t *arena, const char * const __buffer) 
{
    assert(__buffer);
    str_t s = {
        .len = strlen(__buffer),
        .data = arena 
            ? arena_reserve_raw(arena, sizeof(char) * (strlen(__buffer) + 1)) 
            : mem_init(NULL, sizeof(char) * strlen(__buffer) + 1),
        .__is_manually_heap_allocated = arena ? false : true
    };

    memcpy(s.data, __buffer, strlen(__buffer));

    return s;
}

void str_free(str_t *x)
{
    if (!x->__is_manually_heap_allocated) 
        return;

    free(x->data);
    x->data = NULL;
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

str_t str_read_file_to_str(arena_t *arena, const char *file_path)
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

    return str_init(arena, __buffer);
}

// Returns the pos of the word in __buffer
u32 str_where_is_string_in_buffer(str_t *word, str_t *__buffer)
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
    const char delimiters[2] = { '/', '\\'};

    str_t o = {
        .len = 0,
        .data = (char *)string,
    };

    for (u32 i = len; i >= 0; i--)
    {
        if (string[i] == delimiters[0] || string[i] == delimiters[1]) {
            o.len = i+1; //NOTE: to include the delimiter also
            break;
        }
    }

    return o;
}

void cstr_combine_path(const char *path1, const char *path2, char *output, const u32 output_size)
{
    u32 len = strlen(path1);
    if (path1[len - 1] == '\\' || path1[len - 1] == '/') 
        len--;
#if defined(_WIN64)
    snprintf(output, output_size, "%.*s\\%s",len, path1, path2);
#else
    snprintf(output, output_size, "%.*s/%s", len, path1, path2);
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

str_t str_join(arena_t *arena, const str_t *part1, const char *part2)
{
    ASSERT(part1);
    ASSERT(part2);

    const u32 part2_len = strlen(part2);

    char *buffer = arena_reserve_raw(arena, sizeof(char) * (part1->len + part2_len + 1));
    ASSERT(buffer);
    sprintf(buffer, "%.*s%s", part1->len, part1->data, part2);
    return (str_t) {
        .len = part1->len + part2_len,
        .data = buffer,
        .__is_manually_heap_allocated = false
    };
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

str_t str__from_cstr(const char *data, const u32 len)
{
    return (str_t) {
        .data = (char *)data,
        .len = len,
        .__is_manually_heap_allocated = false
    };
}

#endif
