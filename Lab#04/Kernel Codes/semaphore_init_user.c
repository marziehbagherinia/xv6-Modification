#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char*argv[])
{    
    int i = atoi(argv[1]);
    int v = atoi(argv[2]);
    int m = atoi(argv[3]);
    printf(2, "user level: i, v, m: %d - %d - %d\n", i, v, m);
    semaphore_initialize(i, v, m);
    exit();
}