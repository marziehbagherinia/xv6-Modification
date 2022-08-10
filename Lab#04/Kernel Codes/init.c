// init: The initial user-level program

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char *argv[] = { "sh", 0 };

int
main(void)
{
  int pid, wpid;
  int print_handler_pid;

  if(open("console", O_RDWR) < 0){
    mknod("console", 1, 1);
    open("console", O_RDWR);
  }
  dup(0);  // stdout
  dup(0);  // stderr

  for(;;){
    pid = fork();
    if(pid < 0){
      printf(1, "init: fork failed\n");
      exit();
    }
    if(pid > 0){
      print_handler_pid = fork();
      if(print_handler_pid < 0){
        printf(1, "init: fork print handler failed.\n");
        exit();
      }
      if(print_handler_pid == 0)
        print_syscalls_handler();
    }
    if(pid == 0){
      exec("sh", argv);
      printf(1, "init: exec sh failed\n");
      exit();
    }

    printf(1, "init: starting sh\n");
	  printf(1, "Group #25: \n");
	  printf(1, "1- Kiavash Jamshidi\n");
	  printf(1, "2- Dorin Mosalman Yazdi\n");
	  printf(1, "3- Marzieh Bagherinia\n");

    while((wpid=wait()) >= 0 && wpid != pid)
    {
      //printf(1, "zombie!\n");
    }
  }
}
