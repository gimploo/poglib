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
const char *    file_readall(file_t * const file);
void            file_destroy(file_t * const file);

#define file_close(pfile) {\
    fclose((*pfile).fp);\
    (*pfile).is_closed = true;\
}


/*------------------------------------------
 // Implementataion
------------------------------------------*/


size_t __file_get_size(const char *file_path)
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


file_t file_init(const char *file_path)
{
    file_t file = {0};
    file.name = file_path;
    file.size = __file_get_size(file_path);
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
    file_destroy(file);
}

const char * file_readall(file_t * const file)
{
    //NOTE: this code works dont tinker
    
    assert(file);

    char *buffer = (char *)malloc(file->size+1);
    if (buffer == NULL) {
        fprintf(stderr, "%s: malloc failed\n", __func__);
        exit(1);
    }

    file_read_to_buf(file, buffer, file->size);
    buffer[file->size] = '\0';

    return buffer;
}

void file_destroy(file_t * const file)
{
    assert(file != NULL);
    if (!file->is_closed) fclose(file->fp);

    file->fp = NULL;
    file->is_closed = true;
}



#endif // __MY__FILE__H__
