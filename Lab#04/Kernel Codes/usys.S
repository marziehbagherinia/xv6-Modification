#include "syscall.h"
#include "traps.h"

#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret

SYSCALL(fork)
SYSCALL(exit)
SYSCALL(wait)
SYSCALL(pipe)
SYSCALL(read)
SYSCALL(write)
SYSCALL(close)
SYSCALL(kill)
SYSCALL(exec)
SYSCALL(open)
SYSCALL(mknod)
SYSCALL(unlink)
SYSCALL(fstat)
SYSCALL(link)
SYSCALL(mkdir)
SYSCALL(chdir)
SYSCALL(dup)
SYSCALL(getpid)
SYSCALL(sbrk)
SYSCALL(sleep)
SYSCALL(uptime)
SYSCALL(get_parent_id)
SYSCALL(get_children)
SYSCALL(get_family)
SYSCALL(reverse_number)
SYSCALL(trace_syscalls)
SYSCALL(print_syscalls_handler)
SYSCALL(print_procs)
SYSCALL(set_level)
SYSCALL(set_tickets)
SYSCALL(set_bjf_params_proc)
SYSCALL(set_bjf_params_system)
SYSCALL(cv_wait)
SYSCALL(cv_signal)
SYSCALL(read_p)
SYSCALL(write_p)
SYSCALL(init_sl)
SYSCALL(init_cv)
SYSCALL(semaphore_initialize)
SYSCALL(semaphore_acquire)
SYSCALL(semaphore_release)
SYSCALL(write_buffer)
SYSCALL(read_buffer)