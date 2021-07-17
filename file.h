#ifndef __FILE_H__
#define __FILE_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

typedef struct File File;
struct File {
    const char *name;
    FILE *fp;
    size_t size;
    bool is_closed;
};

File    file_init(const char *file_path);
bool    file_open(File *file, const char *mode);
void    file_destroy(File *file);

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


File file_init(const char *file_path)
{
    File file = {0};
    file.name = file_path;
    file.size = _file_get_size(file_path);
    file.fp = NULL;
    file.is_closed = true;

    return file;
}

bool file_open(File *file, const char *mode)
{
    if (file == NULL) return false;
    file->fp = fopen(file->name, mode);
    if (file->fp == NULL) return false;

    return true;
}

void file_destroy(File *file)
{
    if (file->is_closed == false) {
        fclose(file->fp);
        file->fp = NULL;
        file->is_closed = true;
    }         
}



#endif // __FILE_H__
