#pragma once

#include <stdlib.h>

#include "varinfo.h"

#ifdef _DEBUG
// FIXME varname in the beginning
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

#define IF_DEBUG(statement) statement

#else

#define STACK_MAKE(VARNAME) \
    stack_t VARNAME = {     \
        .buffer   = NULL,   \
        .size     = 0,      \
        .capacity = 0       \
    } 

#define IF_DEBUG(statement) 

#endif // _DEBUG

typedef int stack_data_t;

typedef enum stack_err_t
{
    STACK_ERR_NONE,
    STACK_ERR_NULL,
    STACK_ERR_BUFFER_NULL,
    STACK_ERR_SIZE_EXCEED_CAPACITY,
    STACK_ERR_ALLOC_FAIL,
    STACK_ERR_OUT_OF_BOUND,
    STACK_ERR_CANARY_ESCAPED
} stack_err_t;

typedef struct stack_t
{
    IF_DEBUG(stack_data_t canary_begin);

    stack_data_t* buffer;
    size_t size;
    size_t capacity;

    IF_DEBUG(const varinfo_t varinfo);

    IF_DEBUG(stack_data_t canary_end);
} stack_t;

stack_err_t stack_ctor(stack_t* stk, size_t capacity);

stack_err_t stack_push(stack_t* stk, stack_data_t val);

stack_err_t stack_pop(stack_t* stk, stack_data_t* val);

const char* stack_strerr(const stack_err_t err);

void stack_dtor(stack_t* stk);

