#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char*argv[])
{    
    int i = atoi(argv[1]);
    semaphore_acquire(i);
    exit();
}