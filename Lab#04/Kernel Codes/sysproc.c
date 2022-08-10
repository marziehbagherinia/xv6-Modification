#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "condvar.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_reverse_number(void) {
    int inputNumber;
    int sum=0;
    asm ("movl %%edi, %0;"
                    : "=r"(inputNumber)
                    :
                    : "%edi");

    for(;inputNumber != 0;inputNumber /= 10){
         sum = sum*10 + inputNumber % 10;
    }
    return(sum);
}

int
sys_get_parent_id(void)
{
  return get_parent_id();
}

int
sys_get_children(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return get_children(pid);
}

int
sys_get_family(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return get_family(pid);
}

int
sys_trace_syscalls(void)
{
  int status;
  if(argint(0, &status) < 0)
    return -1;
  return trace_syscalls(status);
}


void
sys_print_syscalls_handler(void)
{
  return print_syscalls_handler();
}


void
sys_print_procs(void)
{

  return print_procs();
}

void
sys_set_level(void)
{
  int pid, level;
  argint(0, &pid);
  argint(1, &level);
  set_level(pid, level);
}

void
sys_set_tickets(void)
{
  int pid, tickets;
  argint(0, &pid);
  argint(1, &tickets);
  set_tickets(pid, tickets);
}

void
sys_set_bjf_params_proc(void)
{
  int pid, priority_ratio, arrival_time_ratio, executes_cycle_ratio;
  argint(0, &pid);
  argint(1, &priority_ratio);
  argint(2, &arrival_time_ratio);
  argint(3, &executes_cycle_ratio);
  set_bjf_params_proc(pid, priority_ratio, arrival_time_ratio, executes_cycle_ratio);
}

void
sys_set_bjf_params_system(void)
{
  int priority_ratio, arrival_time_ratio, executes_cycle_ratio;
  argint(0, &priority_ratio);
  argint(1, &arrival_time_ratio);
  argint(2, &executes_cycle_ratio);
  set_bjf_params_system(priority_ratio, arrival_time_ratio, executes_cycle_ratio);
}

void
sys_cv_wait(void)
{
  struct condvar cv;
  char* s;

  argptr(0, &s, sizeof(struct condvar));
  cv = *(struct condvar*)s;
  //cprintf("is it: %d \n", cv.pid);

  cv_wait(&cv);
}

void
sys_cv_signal(void)
{
  struct condvar cv;
  char* s;

  argptr(0, &s, sizeof(struct condvar));
  cv = *(struct condvar*)s;
  //cprintf("is it: %d \n", cv.pid);

  cv_signal(&cv);
}

void
sys_read_p(void)
{
  int id;
  argint(0, &id);
  read_p(id);
}

void
sys_write_p(void)
{
  int id;
  argint(0, &id);
  write_p(id);
}

void sys_init_sl()
{
  init_sl();
}
void sys_init_cv()
{
  init_cv();
}



int
sys_semaphore_initialize(void)
{
  int i, v, m;
  if(argint(0, &i) < 0)
    return -1;
  if(argint(1, &v) < 0)
    return -1;
  if(argint(2, &m) < 0)
    return -1; 
  semaphore_initialize(i, v, m);
  return 1;
}

int
sys_semaphore_acquire(void)
{
  int i;
  if(argint(0, &i) < 0)
    return -1; 
  semaphore_acquire(i);
  return 1;
}

int
sys_semaphore_release(void)
{
  int i;
  if(argint(0, &i) < 0)
    return -1;
  semaphore_release(i);
  return 1;
}

int
sys_write_buffer(void)
{
  int value;
  if(argint(0, &value) < 0)
    return -1;
  write_buffer(value);
  return 1;
}

int
sys_read_buffer(void)
{
  read_buffer();
  return 1;
}