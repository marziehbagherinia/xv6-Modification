#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
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
  	
  	if(getpid() == 6)
  	{
    		res = get_family(2);
    		printf(2, "Parent with %d pid's children are: %d \n", 2, res);
    		
    		res = get_family(3);
    		printf(2, "Parent with %d pid's children are: %d \n", 3, res);
    		
    		res = get_family(4);
    		printf(2, "Parent with %d pid's children are: %d \n", 4, res);  
    		
    		res = get_family(5);
    		printf(2, "Parent with %d pid's children are: %d \n", 5, res);
  
      		res = get_family(6);
    		printf(2, "Parent with %d pid's children are: %d \n", 6, res);   		 		  		
  	}
  
  	exit();
}
