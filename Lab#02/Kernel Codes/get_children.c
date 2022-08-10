#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
	
	int pid = getpid();
	int res;
	if(fork() == 0)
	{
		if(fork()!=0)
		{
			wait();
		}
		else
    		{
     			sleep(100);
     			printf(2, "child[1] --> pid = %d and parent's pid = %d\n", getpid(), get_parent_id());
    		}
   
  	}
  	else 
  	{
    		if(fork()!=0)
    		{ 
      			wait();
 	        }
    		else
    		{
      			printf(2, "child[2] --> pid = %d and parent's pid = %d\n", getpid(), get_parent_id());
      			sleep(200);
    		}
    		wait();
  	}
  	
  	if(getpid() == pid + 3)
  	{
      		res = get_children(pid - 1);
    		printf(2, "Parent with %d pid's children are: %d \n", pid - 1, res);
    		
    		res = get_children(pid);
    		printf(2, "Parent with %d pid's children are: %d \n", pid, res);
    		
    		res = get_children(pid + 1);
    		printf(2, "Parent with %d pid's children are: %d \n", pid + 1, res);  
    		
    		res = get_children(pid + 2);
    		printf(2, "Parent with %d pid's children are: %d \n", pid + 2, res);
  
      		res = get_children(pid + 3);
    		printf(2, "Parent with %d pid's children are: %d \n", pid + 3, res);   
  	}
  
  	exit();
}
