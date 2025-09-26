#include "stack.h"

#include <assert.h>
#include <stdio.h>

static size_t CAPACITY_EXP = 2;

#ifdef _DEBUG

#define IF_DEBUG(statement) statement

#define _STACK_DUMP(STK, ERR, MSG) \
    _stack_dump(stderr, STK, ERR, MSG, __FILE__, __func__, __LINE__)

static void _stack_dump(FILE* stream, stack_t* stk, stack_err_t err, const char* msg, 
                        const char* filename, const char* funcname, int line);

static stack_err_t _stack_validate(stack_t* stk);

const stack_data_t POISON = (stack_data_t)0xCAFEBABE;

#else 

#define IF_DEBUG(statement) 

#endif // _DEBUG

stack_err_t _stack_realloc(stack_t* stk, size_t capacity);

stack_err_t stack_ctor(stack_t* stk, size_t capacity)
{
    stack_err_t err = STACK_ERR_NONE;

    IF_DEBUG(
        err = _stack_validate(stk);
        if(err == STACK_ERR_NULL) {
            _STACK_DUMP(stk, err, "passed null-pointer");
            return err;
        }
    )

    stk->buffer = NULL;
    stk->size   = 0;

    err = _stack_realloc(stk, capacity);
    if(err == STACK_ERR_ALLOC_FAIL) {
        IF_DEBUG(_STACK_DUMP(stk, err, "failed to allocate buffer"));
        return err;
    }
    
    stk->capacity = capacity;

    return err;
}

stack_err_t stack_push(stack_t* stk, stack_data_t val)
{
    stack_err_t err = STACK_ERR_NONE;

    IF_DEBUG(
        err = _stack_validate(stk);

        if     (err == STACK_ERR_NULL)
            _STACK_DUMP(stk, err, "passed null-pointer");
        else if(err == STACK_ERR_BUFFER_NULL)
            _STACK_DUMP(stk, err, "");
        else if(err == STACK_ERR_SIZE_EXCEED_CAPACITY)
            _STACK_DUMP(stk, err, "");

        if(err != STACK_ERR_NONE)
            return err;
    );
    
    if(stk->size == stk->capacity) {
        err = _stack_realloc(stk, stk->capacity * CAPACITY_EXP);
        if(err != STACK_ERR_NONE) {
            IF_DEBUG(_STACK_DUMP(stk, err, ""));
            return err;
        }
    }

    stk->buffer[stk->size++] = val;
    
    return err;
}

stack_err_t stack_pop(stack_t* stk, stack_data_t* val)
{
    stack_err_t err = STACK_ERR_NONE;

    IF_DEBUG(
        err = _stack_validate(stk);

        if     (err == STACK_ERR_NULL)
            _STACK_DUMP(stk, err, "passed null-pointer");
        else if(err == STACK_ERR_BUFFER_NULL)
            _STACK_DUMP(stk, err, "");
        else if(err == STACK_ERR_SIZE_EXCEED_CAPACITY)
            _STACK_DUMP(stk, err, "");

        if(stk->size == 0) {
            err = STACK_ERR_OUT_OF_BOUND;
            _STACK_DUMP(stk, err, "attempted to pop from empty stack");
        }

        if(err != STACK_ERR_NONE)
            return err;
    );

    *val = stk->buffer[stk->size--];

    return err;
}


const char* stack_strerr(const stack_err_t err)
{
    switch(err)
    {
        case STACK_ERR_NONE:
            return "none";
            break;
        case STACK_ERR_NULL:
            return "stack pointer is null";
            break;
        case STACK_ERR_BUFFER_NULL:
            return "stack buffer pointer is null";
            break;
        case STACK_ERR_SIZE_EXCEED_CAPACITY:
            return "size > capacity";
            break;
        case STACK_ERR_ALLOC_FAIL:
            return "memory allocation failed";
            break;
        case STACK_ERR_OUT_OF_BOUND:
            return "boundary exceed";
            break;
        default:
            return "unknown";
            break;
    }
}

void stack_dtor(stack_t* stk)
{
    IF_DEBUG(
        stack_err_t err;
        err = _stack_validate(stk);

        if(err == STACK_ERR_BUFFER_NULL)
            _STACK_DUMP(stk, err, "tried to dereference null pointer");
    );

    free(stk->buffer);

    stk->buffer   = NULL;
    stk->capacity = 0;
    stk->size     = 0;
}


stack_err_t _stack_realloc(stack_t* stk, size_t capacity)
{
    stack_data_t* buffer_tmp = 
        (stack_data_t*)realloc(stk->buffer, capacity * sizeof(stk->buffer[0]));

    if(buffer_tmp == NULL)
        return STACK_ERR_ALLOC_FAIL;

    IF_DEBUG(
        for(size_t i = stk->size; i < capacity; ++i)
            buffer_tmp[i] = POISON;
    );

    stk->buffer = buffer_tmp;
    stk->capacity = capacity;
    return STACK_ERR_NONE;
}

#ifdef _DEBUG

static stack_err_t _stack_validate(stack_t* stk)
{
    if(stk == NULL)
        return STACK_ERR_NULL;
    else if(stk->buffer == NULL)
        return STACK_ERR_BUFFER_NULL;
    else if(stk->size > stk->capacity)
        return STACK_ERR_SIZE_EXCEED_CAPACITY;

    return STACK_ERR_NONE;
}

static void _stack_dump(FILE* stream, stack_t* stk, stack_err_t err, const char* msg, 
                        const char* filename, const char* funcname, int line)
{
    const varinfo_t* varinfo = &stk->varinfo;
    
    fprintf(stream, "what: %s\n", msg);

    fprintf(stream, "stack [%p] (%s)\n", stk, stack_strerr(err));
    fputs("{\n", stream);

    fprintf(stream, "  from: %s:%d %s()\n", filename, line, funcname);
    fprintf(stream, "  init: %s:%d %s(): %s\n", varinfo->filename, varinfo->line, varinfo->funcname, varinfo->varname);

    fprintf(stream, "  capacity: %lu\n", stk->capacity);
    fprintf(stream, "  size: %lu %s\n", stk->size, err == STACK_ERR_SIZE_EXCEED_CAPACITY ? "BAD" : "");

    fprintf(stream, "  buffer [%p]\n", stk->buffer);
    fputs("  {\n", stream);

    for(size_t i = 0; i < stk->capacity; ++i)
        if(stk->buffer[i] == POISON)
            fprintf(stream, "    [%lu] = %d [POISON]\n", i, stk->buffer[i]);
        else
            fprintf(stream, "   *[%lu] = %d\n", i, stk->buffer[i]);

    fputs("  }\n", stream);
    fputs("}\n", stream);
}

#endif // _DEBUG
