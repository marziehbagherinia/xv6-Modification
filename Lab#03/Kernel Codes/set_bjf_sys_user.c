#include "types.h"
#include "stat.h"
#include "user.h"

int 
main(int argc, char *argv[])
{
	printf(1, "Done! \n");
    int priority_ratio = atoi(argv[1]);
    int arrival_time_ratio = atoi(argv[2]);
    int executes_cycle_ratio = atoi(argv[3]);
    set_bjf_params_system(priority_ratio, arrival_time_ratio, executes_cycle_ratio);
    exit();
}