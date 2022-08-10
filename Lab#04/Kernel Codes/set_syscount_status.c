#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
    int status = atoi(argv[1]);
    int result = trace_syscalls(status);
    printf(2, "trace syscalls result: %d\n", result);
    exit();
}
