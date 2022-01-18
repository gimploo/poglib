#include "../../basic.h"

void bar()
{
    eprint("yo");
}

void foo()
{
    bar();
}

int main(void)
{
    foo();
    return 0;
}
