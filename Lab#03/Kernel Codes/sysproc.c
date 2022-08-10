#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

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
  // if(argint(0, &pid) < 0)
    // return -1;
  // if(argint(1, &level) < 0)
    // return -1;
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