#pragma once


typedef enum __val_type {

    VT_U8,
    VT_I8,
    VT_U16,
    VT_I16,
    VT_U32,
    VT_U64,
    VT_I32,
    VT_I64,
    VT_F32,
    VT_F64,
    VT_STRING,

    VT_COUNT

} __val_type;

typedef struct __val_data {

    void            *data;
    unsigned int    size;

} __val_data;

typedef struct generic_data_t {

    __val_type type;
    __val_data data;

} generic_data_t;

#define __val_data_init(data) {data, sizeof(data)}
#define __val_type_init(type) type

#define generic_data_init(data, type) (generic_data_t) { __val_type_init(type), __val_data_init(data) }
