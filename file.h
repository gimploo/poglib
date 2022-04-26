#ifndef __MY__FILE__H__
#define __MY__FILE__H__

/*===================================================
 // File handling library
===================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "basic.h"

typedef struct file_t file_t;
struct file_t {

    const char *name;
    FILE        *fp;
    u64         size;
    bool        is_closed;

};

/*------------------------------------------
 // Declarations
------------------------------------------*/


file_t          file_init(const char *file_path);
bool            file_open(file_t *file, const char *mode);
void            file_read_to_buf(file_t *file, char * const buffer, size_t bytes);
void            file_destroy(file_t * const file);

#define file_close(pfile) {\
    fclose((*pfile).fp);\
    (*pfile).is_closed = true;\
}


/*------------------------------------------
 // Implementataion
------------------------------------------*/


size_t file_get_size(const char *file_path)
{
    FILE *fp = fopen(file_path, "r");
    if (fp == NULL) eprint("%s failed to open", file_path);
    
    fseek(fp, 0L, SEEK_END);
    size_t size = ftell(fp);
    fclose(fp);
    return size; // Returns the position of the null character 
}


file_t file_init(const char *file_path)
{
    file_t file = {0};
    file.name = file_path;
    file.size = file_get_size(file_path);
    file.fp = NULL;
    file.is_closed = true;

    return file;
}

bool file_open(file_t *file, const char *mode)
{
    if (file == NULL) return false;
    file->fp = fopen(file->name, mode);
    if (file->fp == NULL) return false;

    file->is_closed = false;
    return true;
}


void file_read_to_buf(file_t *file,  char * const buffer, size_t bytes)
{
    assert(file != NULL);
    assert(buffer != NULL);
    assert(bytes <= file->size);

    if (file->is_closed || file->fp == NULL) 
        if(!file_open(file, "r")) 
            eprint("Error: unable to open file");
    
    fread(buffer, bytes, 1, file->fp);
}

void file_readall(file_t * const file, char *buffer, u64 buffer_size)
{
    assert(file);
    assert(buffer);
    if(buffer_size < file->size) eprint("buffer is too smol, trying to copy %li to %li", file->size, buffer_size);

    file_read_to_buf(file, buffer, file->size);
    buffer[file->size] = '\0';
}

void file_destroy(file_t * const file)
{
    assert(file != NULL);
    if (!file->is_closed) fclose(file->fp);

    file->fp = NULL;
    file->is_closed = true;
}



#endif // __MY__FILE__H__
