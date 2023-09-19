#pragma once
#include <string.h>
#ifdef __linux__
    #include <execinfo.h>
#elif _WIN64
    #include <windows.h>
    #include <imagehlp.h>
#endif
#include "./ds/linkedlist.h"

#define DEFAULT_DBG_MEM_LOG_PATH "bin/dbg_mem_log.txt"
const char *IGNORE_FILES[] = { "stb_image.h", "stb_truetype.h" };

// Eprint for for both linux and windows
#define eprint(fmt, ...) do {\
\
     fprintf(stderr, "\n\033[0;37m" "[(%s:%d): %s] " "\033[0;31m\n\t" fmt "\033[0m\n",__FILE__, __LINE__, __func__, ##__VA_ARGS__);\
    stacktrace_print();\
    exit(-1);\
\
} while (0)

/*=============================================================================
                        - MEMORY LEAK CHECKER -
===============================================================================*/

typedef struct dbg_t {

    FILE        *fp;
    const char  *fname;
    llist_t     list;

} dbg_t ;

static dbg_t global_debug;
#if defined(DEBUG)
    void        dbg_init();
    void        dbg_destroy(void);
#else 
    #define     dbg_init()      fprintf(stderr,"[!] DBG IS NOT INITIALIZED (Define DEBUG macro to activate)\n")
    #define     dbg_destroy()   fprintf(stderr,"[!] DBG IS NOT INITIALIZED (Define DEBUG macro to activate)\n")
#endif


/*=============================================================================
                            - STACK TRACE -
===============================================================================*/

#if defined(DEBUG)
    void        stacktrace_print(void);
#else
    #define stacktrace_print() fprintf(stderr, "[❗] Missing DEBUG macro, define DEBUG before including library\n")
#endif



#if !defined(IGNORE_MYDBG_IMPLEMENTATION) && defined(DEBUG)

// Wrapper of common memory allocating function in stdlib 

#define WORD 512

#define malloc(N)       _debug_malloc((N), __FILE__, __LINE__, __func__)
#define calloc(S,N)     _debug_malloc(((N) * (S)), __FILE__, __LINE__, __func__)
#define realloc(P, N)   _debug_realloc((P), (#P), (N), __FILE__, __LINE__, __func__)
#define free(P)         _debug_free((P), (#P), __FILE__, __LINE__, __func__) 


#define MAX_STACKTRACES_IN_NODE 12 

typedef struct dbg_node_info_t {

    void        *value;
    uint32_t    bytes;
    char        filename[WORD];
    char        funcname[WORD];
    uint32_t    linenum;


    char        *stacktraces[MAX_STACKTRACES_IN_NODE];
    int         nstacktraces;

} dbg_node_info_t ;

void __dbg_stacktraces_free(void)
{
#undef free
    llist_t *list = &global_debug.list;

    node_t *cur = list->head;
    while (cur) {
        dbg_node_info_t *info = cur->value;
        for (int i = 0; i < info->nstacktraces; i++) {
            memset(info->stacktraces[i], 0, 1024);
            free(info->stacktraces[i]);
        }
        cur = cur->next;
    }
#define free(P)         _debug_free((P), (#P), __FILE__, __LINE__, __func__) 
}

void __dbg_set_stacktraces(dbg_node_info_t *dn)
{
#undef malloc
#undef calloc
#undef realloc
#undef free

#if defined(_WIN64)
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

    int count = 0;
    for(int index = 2, j = 0; index < frames && j != MAX_STACKTRACES_IN_NODE; index++, j++ )
    {
        dn->stacktraces[j] = calloc(1024, sizeof(char));
        SymFromAddr( process, ( DWORD64 )( stack[ index ] ), 0, symbol );
        sprintf(dn->stacktraces[j], " %02i |  %s [0x%0X]", frames - index, symbol->Name, symbol->Address );
        count++;
    }
    dn->nstacktraces = count;
    free(symbol);

#elif defined(__linux__)

    void *array[10];

    int size = backtrace (array, 10);
    char **strings = backtrace_symbols(array, size);
    int count = 0;
    if (strings != NULL)
    {
        for (int i = 2, j = -1; i < size; i++) 
        {
            dn->stacktraces[++j] = (char *)calloc(1024, sizeof(char));
            sprintf(dn->stacktraces[j], " %02i |  %s", (size - i), strings[i]);
            count++;
        }
    }
    dn->nstacktraces = count;
    free (strings);

#endif

#define malloc(N)       _debug_malloc((N), __FILE__, __LINE__, __func__)
#define calloc(S,N)     _debug_malloc(((N) * (S)), __FILE__, __LINE__, __func__)
#define realloc(P, N)   _debug_realloc((P), (#P), (N), __FILE__, __LINE__, __func__)
#define free(P)         _debug_free((P), (#P), __FILE__, __LINE__, __func__) 
}

bool __is_file_in_ignore_files(const char *filepath)
{
    char *pfile = filepath + strlen(filepath);
    for (; pfile > filepath; pfile--)
    {
        if ((*pfile == '\\') || (*pfile == '/'))
        {
            pfile++;
            break;
        }
    }
    for (int i = 0; i < (sizeof(IGNORE_FILES) / sizeof(IGNORE_FILES[0])); i++)
    {
        if (strcmp(IGNORE_FILES[i], pfile) == 0) return true;
    }

    return false;
}

// Init function required to start the debugger

#ifdef _WIN64
LONG WINAPI TopLevelExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo)
{
    /* std::stringstream s;
       s << "Fatal: Unhandled exception 0x" << std::hex << pExceptionInfo->ExceptionRecord->ExceptionCode 
       << std::endl; */

    eprint("APPLICATION SEGFAULTED\n");

}
#endif

void dbg_init(void)
{
    FILE *fp = fopen(DEFAULT_DBG_MEM_LOG_PATH, "w");
    if (!fp) {
        printf("%s not found \n", DEFAULT_DBG_MEM_LOG_PATH);
        fp = fopen("dbg_mem_log.txt", "w");
    }

    global_debug.fp = fp;
    global_debug.fname = DEFAULT_DBG_MEM_LOG_PATH;
    global_debug.list = llist_init();

    fprintf(stdout, "[*] DBG: INITALIZED\n");
}

void debugprint(void *arg)
{
    assert(arg);
    dbg_node_info_t info = *(dbg_node_info_t *)arg;

    const char* fmt = 
        "\033[01;33m%s:%02i\033[0m In function '%s': \033[01;32m %p (%0i bytes)\033[0m\n";

    fprintf(stdout, fmt, info.filename, info.linenum, info.funcname, info.value, info.bytes);
    for (int i = 0; i < info.nstacktraces; i++)
        fprintf(stdout, "\t %s\n", info.stacktraces[i]);

}

void debug_mem_dump(void)
{
    llist_t *list = &global_debug.list;
    assert(list);
    llist_print(list, debugprint);
}


static void * _debug_malloc(const size_t size, const char *file_path, const size_t line_num, const char *func_name)
{
#undef malloc

    FILE *fp = global_debug.fp;            // global variable
    llist_t *list = &global_debug.list;

    if(!fp) {
        fprintf(stderr, "You forgot to add dbg_init() in your code"); 
        exit(0);
    }

    assert(list);

    void *malloc_mem = malloc(size);
    memset(malloc_mem, 0, size);

    if (__is_file_in_ignore_files(file_path)) {
        fprintf(fp, "%s file ignored\n", file_path);   
        return malloc_mem;
    }

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

    __dbg_set_stacktraces(&info);

    node_t *node = node_init(&info, sizeof(info));
    assert(node);
    llist_append_node(list, node);
    
#define malloc(n) _debug_malloc((n), __FILE__, __LINE__, __func__)

    return malloc_mem;
}


bool compare_dbg(node_t *arg01, void *arg02)
{
    dbg_node_info_t *info = (dbg_node_info_t *)arg01->value;
    return (info->value == arg02);
}

void * _debug_realloc(void *pointer, const char *pointer_name, const size_t size, const char *file_path, const size_t line_num, const char *func_name)
{
#undef realloc
#undef free

    FILE *fp = global_debug.fp; // global variable
    llist_t *list = &global_debug.list;

    if(!fp) {
        fprintf(stderr, "You forgot to add dbg_init() in your code"); 
        exit(0);
    }
    assert(list);

    void *realloc_mem = realloc(pointer, size);

    if (__is_file_in_ignore_files(file_path)) {
        fprintf(fp, "%s file ignored\n", file_path);
        return realloc_mem;
    }

    fprintf(fp, "%s: \tIn function '%s':\n", file_path, func_name);
    fprintf(fp, "%s:%li: \t\033[01;34m(%p) REALLOCTED (%0li bytes)\033[0m\n" ,
            file_path, 
            line_num, 
            pointer, 
            size);

    fprintf(fp, "\n");

    node_t *tmp = llist_get_node_by_value(list, pointer, compare_dbg);

    if (!tmp) {

        printf("%s:%li: TRYED TO REALLOC\n", file_path,line_num);
        char **sts = ((dbg_node_info_t *)tmp->value)->stacktraces;
        dbg_node_info_t *j = tmp->value;
        for (int i = 0; i < j->nstacktraces; i++) 
        {
            printf("\t %s\n", sts[i]); 
            free(sts[i]);
        }

        debug_mem_dump();
        printf("%s:%li: (%s) pointer not found %p\n", file_path,line_num, pointer_name, pointer);
        exit(1);

    }
    dbg_node_info_t *j = tmp->value;
    for (int i = 0; i < j->nstacktraces; i++) 
        free(j->stacktraces[i]);

    llist_delete_node(list, tmp);


    dbg_node_info_t info;
    info.value = realloc_mem;
    info.linenum = line_num;
    info.bytes = size;
    memcpy(info.filename, file_path, sizeof(info.funcname));
    memcpy(info.funcname, func_name, sizeof(info.funcname));
    __dbg_set_stacktraces(&info);

    node_t *node = node_init(&info, sizeof(info));
    assert(node);
    llist_append_node(list, node);

#define free(P) _debug_free((P), (#P), __FILE__, __LINE__, __func__) 
#define realloc(p, n) _debug_realloc((p), (#p), (n), __FILE__, __LINE__, __func__)

    return realloc_mem;
}


void _debug_free(void *pointer, const char *pointer_name , const char *file_path, const size_t line_num, const char *func_name)
{
#undef free
    FILE *fp = global_debug.fp;
    if(!fp) {
        fprintf(stderr, "You forgot to add dbg_init() in your code"); 
        exit(0);
    }

    llist_t *list = &global_debug.list;
    assert(list);

    if (__is_file_in_ignore_files(file_path)) {
            fprintf(fp,"%s file ignored\n", file_path);
            free(pointer);
#define free(P) _debug_free((P), (#P), __FILE__, __LINE__, __func__) 
            return;
    }

    fprintf(fp, "%s: \tIn function '%s':\n%s:%li: \t\033[01;32m(%p) DEALLOCATED  \033[0m\n\n" 
            , file_path, func_name, file_path, line_num, pointer);
    
    node_t *node = llist_get_node_by_value(list, pointer, compare_dbg);

    if (list->count > 0) {
    
        if (!node) {

            printf("%s:%li: TRYED TO FREE\n", file_path,line_num);
            dbg_node_info_t * tmp = (dbg_node_info_t *)node->value;
            char **sts = tmp->stacktraces;
            for (int i = 0; i < tmp->nstacktraces; i++) {
                printf("\t %s\n", sts[i]); 
                free(sts[i]);
            }

            debug_mem_dump();

            if (!pointer) 
                fprintf(stderr, "[DBG] `%s` is a null pointer\n", pointer_name);
            else 
                fprintf(stderr, "[DBG] `%s` is a pointer to address %x\n", pointer);

            stacktrace_print();
            exit(1);

        } else {

//NOTE: dont know why but the undef statment needs to be there for this to work properly 
//and not smash the stack from recursive calls
#undef free
            {
                dbg_node_info_t *node_value = node->value;
                for (int i = 0; i < node_value->nstacktraces; i++) {
                    free(node_value->stacktraces[i]);
                }
                llist_delete_node(list, node);
            }
        }

    } else {

        printf("[!] Warning DBG tried to delete pointer %p from the list but the list is empty\n");
    }

    free(pointer);
    pointer = NULL;

#define free(P) _debug_free((P), (#P), __FILE__, __LINE__, __func__) 

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





#if !defined(IGNORE_STACKTRACE_IMPLEMENTATION) && defined(DEBUG)

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
#if defined(__linux__)
    linux_print_trace();
#elif defined(_WIN64)
    window_print_trace();
#endif

    if (global_debug.fp != NULL) dbg_destroy();
}
#endif //IGNORE_STACKTRACE_IMPLEMENTATION
       
