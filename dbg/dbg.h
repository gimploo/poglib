/*
 * This header prints debug info when allocating and freeing 
 * memory from the heap
 */
#ifndef _DBG_H_
#define _DBG_H_

#include "../basic.h"
#include "../ds/linkedlist.h"


typedef struct dbg_t dbg_t;
struct dbg_t {

    FILE        *fp;
    const char  *fname;
    
    llist_t list;

};

typedef struct dbg_node_info_t {

    data_type   type;
    void        *value;
    u32         bytes;

    char filename[WORD];
    char funcname[WORD];
    u32 linenum;


} dbg_node_info_t ;


// FIXME:
// Issue in DValMeta filename member, apparantely the filename isnt copied properly to it but in func_name that isnt a problem.


// Global debug meta data struct
extern dbg_t debug; 

bool dgb_init(dbg_t *debug, const char *);
void dbg_destroy();


// Wrapper of common memory allocating function in stdlib 

#define malloc(N)       _debug_malloc((N), __FILE__, __LINE__, __func__)
#define realloc(P, N)   _debug_realloc((P), (N), __FILE__, __LINE__, __func__)
#define free(P)         _debug_free((P), (#P), __FILE__, __LINE__, __func__) 



/*
 *
 *
 *
 * Function Definition
 *
 *
 *
 *
 */

// Init function required to start the debugger
bool dbg_init(dbg_t *debug, const char *fpath)
{
    assert(fpath);
    FILE *fp = fopen(fpath, "w");
    assert(fp);
    assert(debug);
    debug->fp = fp;
    debug->fname = fpath;
    debug->list = llist_init();

    fprintf(stdout, "DBG: Initalized\n");

    return true;
}

void debugprint(void *arg)
{
    if (arg == NULL) eprint("arg is null");

    dbg_node_info_t info = *(dbg_node_info_t *)arg;


    fprintf(stdout, "%s: \tIn function '%s':\n", info.filename, info.funcname);
    fprintf(stdout, "%s:%i: \tADDR :=\033[0;32m %p (%0i bytes)\033[0m\n", info.filename, info.linenum, info.value, info.bytes);

    printf("\n");

}

void debug_mem_dump(void)
{
    llist_t *list = &debug.list;
    assert(list);

    llist_print(list, debugprint);
}


static void * _debug_malloc(size_t size, const char *file_path, size_t line_num, const char *func_name)
{
    FILE *fp = debug.fp;            // global variable
    llist_t *list = &debug.list;

    assert(fp); assert(list);


#undef malloc

    void *malloc_mem = malloc(size);

#define malloc(n) _debug_malloc((n), __FILE__, __LINE__, __func__)

    fprintf(fp, "%s: \tIn function '%s':\n", file_path, func_name);
    fprintf(fp, "%s:%li: \t\033[01;31m(%p) ALLOCTED (%0li bytes)\033[0m\n", 
            file_path, line_num,malloc_mem, size);
    fprintf(fp, "\n");

    dbg_node_info_t info;
    info.type = DT_ptr;
    info.value = malloc_mem;
    info.linenum = line_num;
    info.bytes = size;
    memcpy(info.filename, file_path, sizeof(info.funcname));
    memcpy(info.funcname, func_name, sizeof(info.funcname));

    node_t *node = node_init(&info, sizeof(info));
    assert(node);
    llist_append_node(list, node);
    
    return malloc_mem;
}


bool compare_dbg(node_t *arg01, void *arg02)
{
    return (((dbg_node_info_t *)arg01->value)->value == arg02);
}

static void * _debug_realloc(void *pointer, size_t size, const char *file_path, size_t line_num, const char *func_name)
{
    FILE *fp = debug.fp; // global variable
    llist_t *list = &debug.list;

    assert(fp); assert(list);

    fprintf(fp, "%s: \tIn function '%s':\n", file_path, func_name);
    fprintf(fp, "%s:%li: \t\033[01;34m(%p) REALLOCTED (%0li bytes)\033[0m\n" ,
            file_path, 
            line_num, 
            pointer, 
            size);

    fprintf(fp, "\n");

#undef realloc

    void *realloc_mem = realloc(pointer, size);

#define realloc(p, n) _debug_realloc((p), (n), __FILE__, __LINE__, __func__)

    if (!llist_delete_node_by_value(list, pointer, compare_dbg))
    {
        debug_mem_dump();
        eprint("error: pointer not found %p\n", pointer);
    }

    dbg_node_info_t info;
    info.type = DT_ptr;
    info.value = realloc_mem;
    info.linenum = line_num;
    info.bytes = size;
    memcpy(info.filename, file_path, sizeof(info.funcname));
    memcpy(info.funcname, func_name, sizeof(info.funcname));

    node_t *node = node_init(&info, sizeof(info));
    assert(node);
    llist_append_node(list, node);
    return realloc_mem;
}


static void _debug_free(void *pointer, char *pointer_name , const char *file_path, size_t line_num, const char *func_name)
{
    FILE *fp = debug.fp;
    llist_t *list = &debug.list;

    assert(fp); assert(list);

    fprintf(fp, "%s: \tIn function '%s':\n", file_path, func_name);
    fprintf(fp, "%s:%li: \t\033[01;32m(%p) DEALLOCATED  \033[0m\n" 
            ,file_path, line_num, pointer);
    fprintf(fp, "\n");
    
    if (!llist_delete_node_by_value(list, pointer, compare_dbg)){
        debug_mem_dump();
        eprint("pointer not found %p\n", pointer);
    }

        

#undef free

    free(pointer);

#define free(p) _debug_free((p), (#p), __FILE__, __LINE__, __func__) 
}


// Close function required to end the debugger
void dbg_destroy()
{
    fprintf(stdout, "DBG: Concluded\n");


    if (debug.list.count == 0) {

        fprintf(debug.fp, 
                "NO MEMORY LEAKS\n");
        fprintf(stdout, 
                "NO MEMORY LEAKS\n");
    } else {

        fprintf(debug.fp, 
                "MEMORY LEAK FOUND -> COUNT %0li\n", debug.list.count);
        fprintf(stdout, 
                "MEMORY LEAK FOUND -> COUNT %0li\n", debug.list.count);
        debug_mem_dump();
    }

    llist_destroy(&debug.list);

    fclose(debug.fp);
}

#endif //_DEBUG_H_
