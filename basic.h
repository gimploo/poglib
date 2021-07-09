#ifdef _BASIC_H_
#define _BASIC_H_

#include <stdio.h>
#include <sdlib.h>
#include <stdbool.h>


typedef u_int8_t uint8;
typedef u_int16_t uint16;
typedef u_int32_t uint32;
typedef u_int64_t uint64;



#define error(fmt, arg) {                       \
    fprintf(stderr, "%s:" fmt, __func__, arg);  \
    exit(1);                                    \
}




#endif // _BASIC_H_
