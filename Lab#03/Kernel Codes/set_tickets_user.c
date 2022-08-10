#include "types.h"
#include "stat.h"
#include "user.h"

int 
main(int argc, char *argv[])
{
    int pid = atoi(argv[1]);
    int tickets = atoi(argv[2]);
    set_tickets(pid, tickets);
    exit();
}