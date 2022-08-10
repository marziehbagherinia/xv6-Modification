#include "types.h"
#include "stat.h"
#include "user.h"
#include "x86.h"
#include "condvar.h"

void 
init_lock(struct spinlock1 *lk)
{
	lk->locked = 0;
}

void
lock(struct spinlock1 *lk)
{
	while(xchg(&lk->locked, 1) != 0)
	;
	__sync_synchronize();
	//printf(1, "spinlock locked - lk->locked = %d\n", lk->locked);
}

void
unlock(struct spinlock1 *lk)
{
	__sync_synchronize();
	asm volatile("movl $0, %0" : "+m" (lk->locked) : );
	//printf(1, "spinlock locked - lk->unlocked = %d\n", lk->locked);
}

void
init_condvar(struct condvar *cv)
{
		init_lock(&cv->lk); 
		cv->locked = 0;
		cv->pid = getpid();
}

/*
int
main(int argc, char *argv[])
{
		struct spinlock1 lk;  
		init_lock(&lk);
		lock(&lk);
		unlock(&lk);
		struct condvar cv;
		init_condvar(&cv);

		if(fork() == 0)
		{
			printf(2, "Child! \n");	
			cv_signal(&cv);
		}
		else 
		{
			printf(1, "Parent! \n");
			lock(&cv.lk);
			cv_wait(&cv);
			unlock(&cv.lk);  		
		}
		lock(&cv.lk);
		cv_wait(&cv);
		printf(2, "hello 2\n");
		
	exit();
}
*/

int main()
{
	struct condvar cv;
	init_condvar(&cv);

	int pid = fork();
	if (pid < 0)
	{
		printf(1, "Error forking first child. \n");
	}
	else if (pid == 0)
	{
		printf(1, "child 1 Executing. \n");
		sleep(2);
		cv_signal(&cv);
	}
	else
	{
		pid = fork();
		if (pid < 0)
		{
			printf(1, "Error forking second child. \n");
		}
		else if (pid == 0)
		{
			lock(&cv.lk);
			cv_wait(&cv);
			unlock(&cv.lk);
			printf(1, "child 2 Executing. \n");
						
		}
		else
		{
			for (int i = 0; i < 2; i++)
			{
				wait();
			}
		}
	}
	exit();        
}