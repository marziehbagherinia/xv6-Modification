#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "syscall.h"
#include "condvar.h"

// User code makes a system call with INT T_SYSCALL.
// System call number in %eax.
// Arguments on the stack, from the user call to the C
// library system call function. The saved user %esp points
// to a saved program counter, and then the first argument.
// Fetch the int at addr from the current process.

int
fetchint(uint addr, int *ip)
{
  struct proc *curproc = myproc();

  if(addr >= curproc->sz || addr+4 > curproc->sz)
    return -1;
  *ip = *(int*)(addr);
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Doesn't actually copy the string - just sets *pp to point at it.
// Returns length of string, not including nul.
int
fetchstr(uint addr, char **pp)
{
  char *s, *ep;
  struct proc *curproc = myproc();

  if(addr >= curproc->sz)
    return -1;
  *pp = (char*)addr;
  ep = (char*)curproc->sz;
  for(s = *pp; s < ep; s++){
    if(*s == 0)
      return s - *pp;
  }
  return -1;
}

// Fetch the nth 32-bit system call argument.
int
argint(int n, int *ip)
{
  return fetchint((myproc()->tf->esp) + 4 + 4*n, ip);
}

// Fetch the nth word-sized system call argument as a pointer
// to a block of memory of size bytes.  Check that the pointer
// lies within the process address space.
int
argptr(int n, char **pp, int size)
{
  int i;
  struct proc *curproc = myproc();
 
  if(argint(n, &i) < 0)
    return -1;
  if(size < 0 || (uint)i >= curproc->sz || (uint)i+size > curproc->sz)
    return -1;
  *pp = (char*)i;
  return 0;
}

// Fetch the nth word-sized system call argument as a string pointer.
// Check that the pointer is valid and the string is nul-terminated.
// (There is no shared writable memory, so the string can't change
// between this check and being used by the kernel.)
int
argstr(int n, char **pp)
{
  int addr;
  if(argint(n, &addr) < 0)
    return -1;
  return fetchstr(addr, pp);
}

extern int sys_chdir(void);
extern int sys_close(void);
extern int sys_dup(void);
extern int sys_exec(void);
extern int sys_exit(void);
extern int sys_fork(void);
extern int sys_fstat(void);
extern int sys_getpid(void);
extern int sys_kill(void);
extern int sys_link(void);
extern int sys_mkdir(void);
extern int sys_mknod(void);
extern int sys_open(void);
extern int sys_pipe(void);
extern int sys_read(void);
extern int sys_sbrk(void);
extern int sys_sleep(void);
extern int sys_unlink(void);
extern int sys_wait(void);
extern int sys_write(void);
extern int sys_uptime(void);
extern int sys_get_parent_id(void);
extern int sys_get_children(void);
extern int sys_get_family(void);
extern int sys_reverse_number(void);
extern int sys_trace_syscalls(void);
extern int sys_print_syscalls_handler(void);
extern int sys_print_procs(void);
extern int sys_set_level(void);
extern int sys_set_tickets(void);
extern int sys_set_bjf_params_proc(void);
extern int sys_set_bjf_params_system(void);
extern int sys_init_lock(void);
extern int sys_lock(void);
extern int sys_unlock(void);
extern int sys_cv_wait(void);
extern int sys_cv_signal(void);
extern int sys_read_p(void);
extern int sys_write_p(void);
extern int sys_init_sl();
extern int sys_init_cv();
extern int sys_semaphore_initialize(void);
extern int sys_semaphore_acquire(void);
extern int sys_semaphore_release(void);
extern int sys_write_buffer(void);
extern int sys_read_buffer(void);

static int (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
[SYS_exit]    sys_exit,
[SYS_wait]    sys_wait,
[SYS_pipe]    sys_pipe,
[SYS_read]    sys_read,
[SYS_kill]    sys_kill,
[SYS_exec]    sys_exec,
[SYS_fstat]   sys_fstat,
[SYS_chdir]   sys_chdir,
[SYS_dup]     sys_dup,
[SYS_getpid]  sys_getpid,
[SYS_sbrk]    sys_sbrk,
[SYS_sleep]   sys_sleep,
[SYS_uptime]  sys_uptime,
[SYS_open]    sys_open,
[SYS_write]   sys_write,
[SYS_mknod]   sys_mknod,
[SYS_unlink]  sys_unlink,
[SYS_link]    sys_link,
[SYS_mkdir]   sys_mkdir,
[SYS_close]   sys_close,
[SYS_get_parent_id]   sys_get_parent_id,
[SYS_get_children]    sys_get_children,
[SYS_get_family]      sys_get_family,
[SYS_reverse_number]  sys_reverse_number,
[SYS_trace_syscalls]  sys_trace_syscalls,
[SYS_print_syscalls_handler] sys_print_syscalls_handler,
[SYS_print_procs]            sys_print_procs,
[SYS_set_level]              sys_set_level,
[SYS_set_tickets]            sys_set_tickets,
[SYS_set_bjf_params_proc]    sys_set_bjf_params_proc,
[SYS_set_bjf_params_system]  sys_set_bjf_params_system,
[SYS_cv_wait]                sys_cv_wait,
[SYS_cv_signal]              sys_cv_signal,
[SYS_read_p]                 sys_read_p,
[SYS_write_p]                sys_write_p,
[SYS_init_sl]                sys_init_sl,
[SYS_init_cv]                sys_init_cv,
[SYS_semaphore_initialize] sys_semaphore_initialize,
[SYS_semaphore_acquire] sys_semaphore_acquire,
[SYS_semaphore_release] sys_semaphore_release,
[SYS_write_buffer]      sys_write_buffer,
[SYS_read_buffer]       sys_read_buffer
};



void
syscall(void)
{
  int num;
  struct proc *curproc = myproc();

  num = curproc->tf->eax;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    curproc->tf->eax = syscalls[num]();
    curproc->syscalls_count[num - 1] ++;  //increase sys count for proc dorin
  } else {
    cprintf("%d %s: unknown sys call %d\n",
            curproc->pid, curproc->name, num);
    curproc->tf->eax = -1;
  }
}
