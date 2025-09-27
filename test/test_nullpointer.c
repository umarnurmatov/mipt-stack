#include "stack.h"

int main()
{
    STACK_MAKE(stk);
    stack_ctor(NULL, 1);

    stack_push(&stk, 1);

    int a = 0;
    stack_pop(&stk, &a);

    stack_dtor(&stk);
    return 0;
}
