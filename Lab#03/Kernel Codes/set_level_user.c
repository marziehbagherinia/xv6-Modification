#include "types.h"
#include "stat.h"
#include "user.h"

int 
main(int argc, char *argv[])
{
	int pid = atoi(argv[1]);
	int level = atoi(argv[2]);
	set_level(pid, level);
	exit();
}