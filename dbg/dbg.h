#pragma once
#ifdef __linux__
    #include <execinfo.h>
#elif _WIN64
    #include <windows.h>
    #include <imagehlp.h>
#endif
#include "../ds/linkedlist.h"




/*===============================
    - MEMORY LEAK CHECKER -
===============================*/

#ifdef DEBUG // THE Memory debugger is only active, if DEBUG macro is set

    typedef struct dbg_t dbg_t;

    bool        dbg_init(void);
    void        dbg_destroy(void);

#endif //DEBUG


/*===============================
        - STACK TRACE -
===============================*/

void        stactrace_print(void);







#ifdef DEBUG
#ifndef IGNORE_MYDBG_IMPLEMENTATION

// Wrapper of common memory allocating function in stdlib 

#define WORD 512

#define malloc(N)       _debug_malloc((N), __FILE__, __LINE__, __func__)
#define calloc(S,N)     _debug_malloc(((N) * (S)), __FILE__, __LINE__, __func__)
#define realloc(P, N)   _debug_realloc((P), (#P), (N), __FILE__, __LINE__, __func__)
#define free(P)         _debug_free((P), (#P), __FILE__, __LINE__, __func__) 


struct dbg_t {

    FILE        *fp;
    const char  *fname;
    
    llist_t list;

};

static dbg_t global_debug;

typedef struct dbg_node_info_t {

    void        *value;
    uint32_t         bytes;

    char filename[WORD];
    char funcname[WORD];
    uint32_t linenum;


} dbg_node_info_t ;


// Init function required to start the debugger
bool dbg_init(void)
{
    FILE *fp = fopen("dbg_mem_log.txt", "w");
    assert(fp);

    global_debug.fp = fp;
    global_debug.fname = "dbg_mem_log.txt";
    global_debug.list = llist_init();

    fprintf(stdout, "[*] DBG: INITALIZED\n");

    return true;
}

void debugprint(void *arg)
{
    assert(arg);

    dbg_node_info_t info = *(dbg_node_info_t *)arg;

    const char* fmt = 
        "\033[01;33m%s:%02i\033[0m In function '%s': \033[01;32m %p (%0i bytes)\033[0m\n";

    fprintf(stdout, fmt, info.filename, info.linenum, info.funcname, info.value, info.bytes);


}

void debug_mem_dump(void)
{
    llist_t *list = &global_debug.list;
    assert(list);

    llist_print(list, debugprint);
}


static void * _debug_malloc(const size_t size, const char *file_path, const size_t line_num, const char *func_name)
{
    FILE *fp = global_debug.fp;            // global variable
    llist_t *list = &global_debug.list;

    assert(fp); 
    assert(list);


#undef malloc

    void *malloc_mem = malloc(size);
    memset(malloc_mem, 0, size);

#define malloc(n) _debug_malloc((n), __FILE__, __LINE__, __func__)

    fprintf(fp, "%s: \tIn function '%s':\n", file_path, func_name);
    fprintf(fp, "%s:%li: \t\033[01;31m(%p) ALLOCTED (%0li bytes)\033[0m\n", 
            file_path, line_num,malloc_mem, size);
    fprintf(fp, "\n");

    dbg_node_info_t info;
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

void * _debug_realloc(void *pointer, const char *pointer_name, const size_t size, const char *file_path, const size_t line_num, const char *func_name)
{
    FILE *fp = global_debug.fp; // global variable
    llist_t *list = &global_debug.list;

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

#define realloc(p, n) _debug_realloc((p), (#p), (n), __FILE__, __LINE__, __func__)

    if (!llist_delete_node_by_value(list, pointer, compare_dbg))
    {
        debug_mem_dump();
        printf("%s:%li: (%s) pointer not found %p\n", file_path,line_num, pointer_name, pointer);
        exit(1);
    }

    dbg_node_info_t info;
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


void _debug_free(void *pointer, const char *pointer_name , const char *file_path, const size_t line_num, const char *func_name)
{
    //(void)pointer_name;

    FILE *fp = global_debug.fp;
    llist_t *list = &global_debug.list;

    assert(fp); assert(list);

    fprintf(fp, "%s: \tIn function '%s':\n", file_path, func_name);
    fprintf(fp, "%s:%li: \t\033[01;32m(%p) DEALLOCATED  \033[0m\n" 
            ,file_path, line_num, pointer);
    fprintf(fp, "\n");
    
    if (!llist_delete_node_by_value(list, pointer, compare_dbg)){
        debug_mem_dump();
        printf("(%s) pointer not found %p\n", pointer_name, pointer);
        exit(1);
    }

        

#undef free

    free(pointer);

#define free(p) _debug_free((p), (#p), __FILE__, __LINE__, __func__) 
}


// Close function required to end the debugger
void dbg_destroy(void)
{
    fprintf(stdout, "[!] DBG: CONCLUDED\n");

    if (global_debug.list.count == 0) {

        fprintf(global_debug.fp, 
                "NO MEMORY LEAKS\n");
        fprintf(stdout, 
                "\n\t\033[01;32mNO MEMORY LEAKS \033[0m\n\n");
    } else {

        fprintf(global_debug.fp, 
                "MEMORY LEAK FOUND: (%0li) COUNT\n", global_debug.list.count);
        fprintf(stdout, 
                "\n\t\033[01;31m MEMORY LEAK FOUND\033[0m: (%02li)\n\n", global_debug.list.count);
        debug_mem_dump();
    }

    llist_destroy(&global_debug.list);

    fclose(global_debug.fp);
}
#endif //IGNORE_MYDBG_IMPLEMENTATION
#endif //DEBUG





#ifndef IGNORE_STACKTRACE_IMPLEMENTATION

#ifdef _WIN64

void window_print_trace(void)
{
    unsigned int   i;
    void         * stack[ 100 ];
    unsigned short frames;
    SYMBOL_INFO  * symbol;
    HANDLE         process;

    process = GetCurrentProcess();

    SymInitialize( process, NULL, TRUE );

    frames               = CaptureStackBackTrace( 0, 100, stack, NULL );
    symbol               = ( SYMBOL_INFO * )calloc( sizeof( SYMBOL_INFO ) + 256 * sizeof( char ), 1 );
    symbol->MaxNameLen   = 255;
    symbol->SizeOfStruct = sizeof( SYMBOL_INFO );

    printf("\n[*] Printing stack frames ... \n\n");
    for( i = 0; i < frames; i++ )
    {
        SymFromAddr( process, ( DWORD64 )( stack[ i ] ), 0, symbol );
        printf( " %02i |  %s [0x%0X]\n", frames - i, symbol->Name, symbol->Address );
    }
    printf ("\n[!] Obtained %d stack frames.\n\n", frames);

    free( symbol );
}

#elif __linux__

void linux_print_trace(void)
{
    void *array[10];
    char **strings;
    int size, i;

    size = backtrace (array, 10);
    strings = backtrace_symbols(array, size);
    if (strings != NULL)
    {
        printf("\n[*] Printing stack frames ... \n\n");
        for (i = 0; i < size; i++)
            printf (" %02i |  %s\n", (size - i), strings[i]);
        printf ("\n[!] Obtained %d stack frames.\n", size);
    }

    free (strings);
}

#endif

void stacktrace_print(void)
{

#ifdef __linux__

    linux_print_trace();

#elif _WIN64

    window_print_trace();

#endif
}
#endif //IGNORE_STACKTRACE_IMPLEMENTATION
