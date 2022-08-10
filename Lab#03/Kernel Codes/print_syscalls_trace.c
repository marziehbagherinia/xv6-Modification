#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
    int result = trace_syscalls(1);
    printf(2, "trace syscalls result: %d", result);
    exit();
}
