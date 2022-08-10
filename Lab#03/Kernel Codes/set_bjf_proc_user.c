#include "types.h"
#include "stat.h"
#include "user.h"

int 
main(int argc, char *argv[])
{
    printf(1, "Done! \n");
    
    int pid = atoi(argv[1]);
    int priority_ratio = atoi(argv[2]);
    int arrival_time_ratio = atoi(argv[3]);
    int executes_cycle_ratio = atoi(argv[4]);
  
    set_bjf_params_proc(pid, priority_ratio, arrival_time_ratio, executes_cycle_ratio);
    exit();
}