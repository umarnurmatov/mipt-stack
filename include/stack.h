#pragma once

#include <stdlib.h>

#include "varinfo.h"

#ifdef _DEBUG
#define STACK_MAKE(VARNAME)           \
    stack_t VARNAME = {               \
        .buffer   = NULL,             \
        .size     = 0,                \
        .capacity = 0,                \
        .varinfo = {                  \
            .line     = __LINE__,     \
            .filename = __FILE__,     \
            .funcname = __func__,     \
            .varname  = ""#VARNAME""  \
        }                             \
    }
#else
#define STACK_MAKE(VARNAME) \
    stack_t VARNAME = {     \
        .buffer   = NULL,   \
        .size     = 0       \
        .capacity = 0       \
    }
#endif // _DEBUG

typedef int stack_data_t;

typedef enum stack_err_t
{
    STACK_ERR_NONE,
    STACK_ERR_NULL,
    STACK_ERR_BUFFER_NULL,
    STACK_ERR_SIZE_EXCEED_CAPACITY,
    STACK_ERR_ALLOC_FAIL,
    STACK_ERR_OUT_OF_BOUND
} stack_err_t;

typedef struct stack_t
{
    stack_data_t* buffer;
    size_t size;
    size_t capacity;

#ifdef _DEBUG
    const varinfo_t varinfo;
#endif // _DEBUG

} stack_t;

stack_err_t stack_ctor(stack_t* stk, size_t capacity);

stack_err_t stack_push(stack_t* stk, stack_data_t val);

stack_err_t stack_pop(stack_t* stk, stack_data_t* val);

const char* stack_strerr(const stack_err_t err);

void stack_dtor(stack_t* stk);

