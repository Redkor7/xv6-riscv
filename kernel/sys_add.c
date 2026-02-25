#include "types.h"
#include "riscv.h"
#include "defs.h"
uint64 sys_add (void)
{
    int k, p;
    argint(0, &k);
    argint(1, &p);
    
    return k + p;
}