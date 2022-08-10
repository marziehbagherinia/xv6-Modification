// condition variable lock in process context

#include "spinlock.h"

struct condvar{
    uint locked;
    struct spinlock1 lk;
    int pid;
};