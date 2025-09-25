#include "stack.h"

int main()
{
    STACK_MAKE(stk);
    stack_ctor(&stk, 1);

    stack_push(&stk, 1);
    stack_push(&stk, 1);

    int a = 0;

    stack_dtor(&stk);
    return 0;
}
