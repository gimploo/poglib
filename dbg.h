/*
 * This header prints debug info when allocating and freeing 
 * memory from the heap
 */
#ifndef _DBG_H_
#define _DBG_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#define KB 1024

typedef struct dbg_t dbg_t;
struct dbg_t {

    //TODO: more fields maybe
    FILE *fp;
    const char *fname;
    size_t count;


    //List *plist;

};


// Global debug meta data struct
extern dbg_t debug; 


// Init function required to start the debugger
static inline bool dbg_init(char *fpath)
{
    assert(fpath);
    FILE *fp = fopen(fpath, "w");
    assert(fp);
    debug = (dbg_t) {
        .fp = fp,
        .fname = fpath,
    };
    fprintf(stdout, "DBG: Initalized\n");

    return true;
}

// Close function required to end the debugger
static inline void dbg_destroy()
{
    fprintf(stdout, "DBG: Concluded\n");
    fclose(debug.fp);
}


// Wrapper of common memory allocating function in stdlib 

#define malloc(n) debug_malloc((n), __FILE__, __LINE__, __func__)
#define realloc(p, n) debug_realloc((p), (n), __FILE__, __LINE__, __func__)
#define free(p) debug_free((p), (#p), __FILE__, __LINE__, __func__) 



static void * debug_malloc(size_t size, char *file_path, size_t line_num, const char *func_name)
{
    FILE *fp = debug.fp; // global variable
    fprintf(fp, "%s: \tIn function '%s':\n", file_path, func_name);
    fprintf(fp, "%s:%li: \t\033[0;32mMEMORY ALLOCTED (%0li bytes)\033[0m\n", 
            file_path, line_num, size);
    fprintf(fp, "\n");

#undef malloc
    return malloc(size);
#define malloc(n) debug_malloc((n), __FILE__, __LINE__, __func__)
}

static void * debug_realloc(void *pointer, size_t size, char *file_path, size_t line_num, const char *func_name)
{
    FILE *fp = debug.fp; // global variable
    fprintf(fp, "%s: \tIn function '%s':\n", file_path, func_name);
    fprintf(fp, "%s:%li: \tMEMORY \033[0;32m REALLOCTED \033[0m OF SIZE:%0li\n" ,
            file_path, line_num, size);
    fprintf(fp, "\n");
#undef realloc
    return realloc(pointer, size);
#define realloc(p, n) debug_realloc((p), (n), __FILE__, __LINE__, __func__)

}

static void debug_free(void *pointer, char *pointer_name , char *file_path, size_t line_num, const char *func_name)
{
    FILE *fp = debug.fp;
    fprintf(fp, "%s: \tIn function '%s':\n", file_path, func_name);
    fprintf(fp, "%s:%li: \t\033[01;34m'%s' DEALLOCATED\033[0m\n" 
            ,file_path, line_num, pointer_name);
    fprintf(fp, "\n");
#undef free
    free(pointer);
#define free(p) debug_free((p), (#p), __FILE__, __LINE__, __func__) 
}

#endif //_DEBUG_H_
