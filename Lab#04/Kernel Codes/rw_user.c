#include "types.h"
#include "stat.h"
#include "user.h"
#include "x86.h"
#include "condvar.h"

int main()
{
	init_sl();
	init_cv();

	int pid = fork();
	if (pid < 0)
	{
		printf(1, "Error forking first child. \n");
	}
	else if (pid == 0)
	{
		read_p(0);
		write_p(1);
	}
	else
	{
		read_p(2);
		write_p(3);
	}	
	exit();        
}