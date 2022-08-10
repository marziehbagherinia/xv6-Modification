#include "types.h"
#include "stat.h"
#include "user.h"

int
main()
{
	int p1 = fork();
	
	if (p1 == 0) {
		int a = 1;
		while (1) {
			a++;
			if (a == 1000000) break;
		}
		
		sleep(1000);
		printf(1, "Done! Parent ID is: %d \n", get_parent_id());

	} else 
	{
		wait();
		int a = 1;
		while (1) {
			a++;
			if (a == 5000) {
				break;
			}
		}

		sleep(2000);
		printf(1, "Done! Parent ID is: %d \n", get_parent_id());
		
	}
	
	exit();
}