#pragma once
#include "basic.h"

/*===================================================
 // File handling library
===================================================*/

typedef struct file_t {

    const char *name;
    FILE        *fp;
    const char *mode;
    u64         size;
    bool        is_closed;

} file_t ;

/*-----------------------------------------------------------------------------
 // Declarations
-------------------------------------------------------------------------------*/

file_t *        file_init(const char *file_path, const char *mode);
void            file_readall(const file_t * const file, char *buffer, const u64 buffer_size);
void            file_readline(const file_t *file, char *buffer, u64 buffersize);
void            file_writeline(file_t *file, const char *line, const u64 linesize);
void            file_readbytes(const file_t *file, void * const buffer, const u64 buffersize);
void            file_writebytes(const file_t *file, void * const buffer, const u64 buffersize);
void            file_destroy(file_t * const file);

#define file(NAME, MODE)\
    for (file_t *file = file_init(NAME, MODE);\
            !file->is_closed;\
            file_destroy(file)) 

/*------------------------------------------
 // Implementataion
------------------------------------------*/

void file_readbytes(const file_t *file, void * const buffer, const u64 buffersize)
{
    assert(buffer);
    assert(buffersize > 0);
    assert(!file->is_closed);

    fread(buffer, buffersize, 1, file->fp);
}

void file_writebytes(const file_t *file, void * const buffer, const u64 buffersize)
{
    assert(buffer);
    assert(buffersize > 0);
    assert(!file->is_closed);

    fwrite(buffer, buffersize, 1, file->fp);
}

u64 file_get_size(const char *filepath)
{    
    assert(filepath);

    FILE *fp = fopen(filepath, "r");
    if (!fp) eprint("file (%s) not found", filepath);

    fseek(fp, 0L, SEEK_END);
    const u64 size = ftell(fp);
    fclose(fp);
    return size;
}

void __file_set_size(file_t *file)
{
    FILE *old_pos = file->fp;
    if (strcmp("w", file->mode) == 0)
        return;

    fseek(file->fp, 0L, SEEK_END);
    const u64 size = ftell(file->fp);
    fseek(file->fp, 0L, SEEK_SET);

    file->fp = old_pos;
    file->size = size;

}

bool file_check_exist(const char *filepath)
{
    assert(filepath);
    FILE *tmp = fopen(filepath, "r");
    if (tmp == NULL) return false;

    fclose(tmp);
    return true;
}


file_t * file_init(const char *file_path, const char *mode)
{
    assert(file_path);
    assert(mode);
    file_t file = {0};
    file.name       = file_path;
    file.mode       = mode;
    file.is_closed  = false;

    file.fp         = fopen(file_path, mode) ;
    if (!file.fp) 
        eprint("unable to open file");

    __file_set_size(&file);

    file_t *tmp = (file_t *)calloc(1, sizeof(file_t ));
    assert(tmp);
    *tmp = file;

    return tmp;
}

void file_readall(const file_t * const file, char *buffer, const u64 buffer_size)
{
    assert(file);
    assert(buffer);
    if(buffer_size < file->size) 
        eprint("buffer is too smol, trying to copy %li to %li", file->size, buffer_size);

    fread(buffer, buffer_size, 1, file->fp);
    buffer[file->size - 1] = '\0';
}

void file_writeline(file_t *file, const char *line, const u64 linesize)
{
    if(file->is_closed) eprint("`%s` file is closed\n", file->name);;
    assert(line);

    fwrite(line, linesize, 1, file->fp);
}



void file_readline(const file_t *file, char *buffer, u64 buffersize)
{
    assert(buffersize != 8);
    assert(!file->is_closed);

    FILE *ptr = file->fp;

    for (u32 i = 0; i < buffersize; i++)
    {
        char c = fgetc(ptr);
        if (c != '\n')
            buffer[i] = c;
        else 
            break;
    }
}

void file_destroy(file_t * const file)
{
    assert(file != NULL);

    fclose(file->fp);
    file->mode = NULL;
    file->size = 0;
    file->fp = NULL;
    file->is_closed = true;
    free(file);
}



