#pragma once

#include "../basic.h"
#include "../str.h"

typedef struct hashtable_t {

    buffer_t buffer;

    u64 (*hash_func)(str_t key);


} hashtable_t;
