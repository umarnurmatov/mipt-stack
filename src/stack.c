#include "stack.h"

#include <assert.h>
#include <stdio.h>

#define BEGIN    do
#define GOTO_END break
#define END      while(0)

#ifdef _DEBUG

#define STACK_DUMP(STK, ERR, MSG) \
    _stack_dump(stderr, STK, ERR, MSG, __FILE__, __func__, __LINE__)
#define CANARY_SIZE(size) (size + 2)
#define CANARY_INDEX(index) (index + 1)

static void _stack_dump(FILE* stream, stack_t* stk, stack_err_t err, const char* msg, 
                        const char* filename, const char* funcname, int line);

static stack_err_t _stack_validate(stack_t* stk);

const stack_data_t POISON       = (stack_data_t)0xCAFEBABE;
const stack_data_t CANARY_BEGIN = (stack_data_t)0x8BADF00D;
const stack_data_t CANARY_END   = (stack_data_t)0xDEADC0DE;

#else

#define CANARY_SIZE(size) size
#define CANARY_INDEX(index) index

#endif // _DEBUG

static size_t CAPACITY_EXP = 2;

stack_err_t _stack_realloc(stack_t* stk, size_t capacity);

stack_err_t stack_ctor(stack_t* stk, size_t capacity)
{
    stack_err_t err = STACK_ERR_NONE;

    IF_DEBUG(
        err = _stack_validate(stk);
        if(err == STACK_ERR_NULL) {
            STACK_DUMP(stk, err, "passed null-pointer");
            return err;
        }
    )

    stk->buffer = NULL;
    stk->size   = 0;

    err = _stack_realloc(stk, capacity);
    if(err == STACK_ERR_ALLOC_FAIL) {
        IF_DEBUG(STACK_DUMP(stk, err, "failed to allocate buffer"));
        return err;
    }

    IF_DEBUG(
        stk->canary_begin = CANARY_BEGIN;
        stk->canary_end   = CANARY_END;
    )

    return err;
}

stack_err_t stack_push(stack_t* stk, stack_data_t val)
{
    stack_err_t err = STACK_ERR_NONE;

    IF_DEBUG(
        err = _stack_validate(stk);

        if     (err == STACK_ERR_NULL)
            STACK_DUMP(stk, err, "passed null-pointer");

        else if(err == STACK_ERR_CANARY_ESCAPED)
            STACK_DUMP(stk, err, "");

        else if(err == STACK_ERR_BUFFER_NULL)
            STACK_DUMP(stk, err, "");

        else if(err == STACK_ERR_SIZE_EXCEED_CAPACITY)
            STACK_DUMP(stk, err, "");

        if(err != STACK_ERR_NONE)
            return err;
    );
    
    if(stk->size == stk->capacity) {
        err = _stack_realloc(stk, stk->capacity * CAPACITY_EXP);
        if(err != STACK_ERR_NONE) {
            IF_DEBUG(STACK_DUMP(stk, err, ""));
            return err;
        }
    }

    stk->buffer[CANARY_INDEX(stk->size++)] = val;
    
    return err;
}

stack_err_t stack_pop(stack_t* stk, stack_data_t* val)
{
    stack_err_t err = STACK_ERR_NONE;

    IF_DEBUG(
        err = _stack_validate(stk);

        if     (err == STACK_ERR_NULL)
            STACK_DUMP(stk, err, "passed null-pointer");

        else if(err == STACK_ERR_CANARY_ESCAPED)
            STACK_DUMP(stk, err, "");

        else if(err == STACK_ERR_BUFFER_NULL)
            STACK_DUMP(stk, err, "");

        else if(err == STACK_ERR_SIZE_EXCEED_CAPACITY)
            STACK_DUMP(stk, err, "");

        else if(stk->size == 0) {
            err = STACK_ERR_OUT_OF_BOUND;
            STACK_DUMP(stk, err, "attempted to pop from empty stack");
        }

        if(err != STACK_ERR_NONE)
            return err;
    );

    *val = stk->buffer[CANARY_INDEX(stk->size--)];

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
        case STACK_ERR_CANARY_ESCAPED:
            return "canary value changed";
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
            STACK_DUMP(stk, err, "tried to dereference null pointer");
    );

    free(stk->buffer);

    stk->buffer   = NULL;
    stk->capacity = 0;
    stk->size     = 0;
}


stack_err_t _stack_realloc(stack_t* stk, size_t capacity)
{
    stack_data_t* buffer_tmp = 
        (stack_data_t*)realloc(
            stk->buffer, 
            CANARY_SIZE(capacity) * sizeof(stk->buffer[0])
        );

    if(buffer_tmp == NULL)
        return STACK_ERR_ALLOC_FAIL;

    IF_DEBUG(
        for(size_t i = CANARY_INDEX(stk->size); i < CANARY_SIZE(capacity); ++i)
            buffer_tmp[i] = POISON;

        buffer_tmp[0                        ] = CANARY_BEGIN;
        buffer_tmp[CANARY_SIZE(capacity) - 1] = CANARY_END;
    );

    stk->buffer   = buffer_tmp;
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

    else if(stk->canary_begin != CANARY_BEGIN)
        return STACK_ERR_CANARY_ESCAPED;

    else if(stk->canary_end != CANARY_END)
        return STACK_ERR_CANARY_ESCAPED;

    else if(stk->buffer[0] != CANARY_BEGIN)
        return STACK_ERR_CANARY_ESCAPED;

    else if(stk->buffer[CANARY_SIZE(stk->capacity) - 1] != CANARY_END)
        return STACK_ERR_CANARY_ESCAPED;

    else if(stk->size > stk->capacity)
        return STACK_ERR_SIZE_EXCEED_CAPACITY;

    return STACK_ERR_NONE;
}

static void _stack_dump(FILE* stream, stack_t* stk, stack_err_t err, const char* msg, 
                        const char* filename, const char* funcname, int line)
{
    const varinfo_t* varinfo = &stk->varinfo;
    
    fputs("================================\n", stream);
    fprintf(stream, "what: %s\n", msg);

    fprintf(
        stream, 
        "from: %s:%d %s()\n\n", 
        filename, 
        line, 
        funcname
    );

    BEGIN {
        if(err == STACK_ERR_NULL) {
            fprintf(stream, "stack [NULL]\n");
            GOTO_END;
        }

        fprintf(stream, "stack [%p] (%s)\n", stk, stack_strerr(err));

        fputs("{\n", stream);

        fprintf(
            stream, 
            "  init: %s:%d %s(): %s\n", 
            varinfo->filename, 
            varinfo->line, 
            varinfo->funcname, 
            varinfo->varname
        );

        fprintf(
            stream, 
            "  capacity: %lu\n", 
            stk->capacity
        );

        fprintf(
            stream, 
            "  size: %lu %s\n", 
            stk->size, 
            err == STACK_ERR_SIZE_EXCEED_CAPACITY ? "(BAD)" : ""
        );

        if(err == STACK_ERR_BUFFER_NULL) {
            fputs("  buffer [NULL]\n", stream);
            GOTO_END;
        }

        fprintf(stream, "  buffer [%p]\n", stk->buffer);
        fputs("  {\n", stream);

        fprintf(
            stream, 
            "    [#] = %x [CANARY] %s\n", 
            (unsigned)stk->buffer[0],
            err == STACK_ERR_CANARY_ESCAPED ? "(BAD)" : ""
        );

        for(size_t i = CANARY_INDEX(0); i < CANARY_INDEX(stk->capacity); ++i)
            if(stk->buffer[i] == POISON)
                fprintf(stream, "    [%lu] = %d [POISON]\n", i, stk->buffer[i]);
            else
                fprintf(stream, "   *[%lu] = %d\n", i, stk->buffer[i]);

        fprintf(
            stream, 
            "    [#] = %x [CANARY] %s\n", 
            (unsigned)stk->buffer[0],
            err == STACK_ERR_CANARY_ESCAPED ? "(BAD)" : ""
        );

        fputs("}\n", stream);
    } END;

    fputs("================================\n\n", stream);
}

#endif // _DEBUG
