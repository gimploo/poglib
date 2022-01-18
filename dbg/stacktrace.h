#pragma once
#include <stdio.h>
#include <stdlib.h>

#ifdef __linux__
    #include <execinfo.h>
#elif _WIN64
    #include <windows.h>
    #include <imagehlp.h>
#endif



void print_stack_trace(void);






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

void print_stack_trace(void)
{

#ifdef __linux__

    linux_print_trace();

#elif _WIN64

    window_print_trace();

#endif

}

#endif
