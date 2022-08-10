// Per-CPU state
struct cpu {
  uchar apicid;                // Local APIC ID
  struct context *scheduler;   // swtch() here to enter scheduler
  struct taskstate ts;         // Used by x86 to find stack for interrupt
  struct segdesc gdt[NSEGS];   // x86 global descriptor table
  volatile uint started;       // Has the CPU started?
  int ncli;                    // Depth of pushcli nesting.
  int intena;                  // Were interrupts enabled before pushcli?
  struct proc *proc;           // The process running on this cpu or null
};

extern struct cpu cpus[NCPU];
extern int ncpu;

//PAGEBREAK: 17
// Saved registers for kernel context switches.
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is that the caller has saved them.
// Contexts are stored at the bottom of the stack they
// describe; the stack pointer is the address of the context.
// The layout of the context matches the layout of the stack in swtch.S
// at the "Switch stacks" comment. Switch doesn't save eip explicitly,
// but it is on the stack and allocproc() manipulates it.
struct context {
  uint edi;
  uint esi;
  uint ebx;
  uint ebp;
  uint eip;
};

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };


struct called_syscalls{
  int called_syscalls[25];
};

#define TRACING 1
#define NOTTRACING 0
#define NUM_SYSCALLS 25


// Per-process state
struct proc {
  uint sz;                     // Size of process memory (bytes)
  pde_t* pgdir;                // Page table
  char *kstack;                // Bottom of kernel stack for this process
  enum procstate state;        // Process state
  int pid;                     // Process ID
  struct proc *parent;         // Parent process
  struct trapframe *tf;        // Trap frame for current syscall
  struct context *context;     // swtch() here to run process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)
  // char* syscalls_name[NPROC];
  int syscalls_count [NSYSCALLS];
  int sys_count_stat;
  int level;                   // 1 for RR, 2 for lottery, 3 for BJF
  int tickets;                 // for lottery scheduling
  int waiting_cycles;          // cycles waiting for turn - for aging
  int arrival_time;
  float arrival_time_ratio;
  float executes_cycle;
  float executes_cycle_ratio;
  float priority_ratio;
};

// Process memory is laid out contiguously, low addresses first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap


#define SYS_fork    1
#define SYS_exit    2
#define SYS_wait    3
#define SYS_pipe    4
#define SYS_read    5
#define SYS_kill    6
#define SYS_exec    7
#define SYS_fstat   8
#define SYS_chdir   9
#define SYS_dup    10
#define SYS_getpid 11
#define SYS_sbrk   12
#define SYS_sleep  13
#define SYS_uptime 14
#define SYS_open   15
#define SYS_write  16
#define SYS_mknod  17
#define SYS_unlink 18
#define SYS_link   19
#define SYS_mkdir  20
#define SYS_close  21
#define SYS_get_parent_id 22
#define SYS_get_children 23
#define SYS_get_family 24
#define SYS_trace_syscalls 25
#define SYS_print_syscalls_handler 26
#define SYS_reverse_number 27
#define SYS_print_procs 28
#define SYS_set_level 29
#define SYS_set_tickets 30
#define SYS_set_bjf_params_proc 31
#define SYS_set_bjf_params_system 32
#define SYS_cv_wait   33
#define SYS_cv_signal 34
#define SYS_read_p    35
#define SYS_write_p   36
#define SYS_init_sl   37
#define SYS_init_cv   38
#define SYS_semaphore_initialize 39
#define SYS_semaphore_acquire 40
#define SYS_semaphore_release 41
#define SYS_write_buffer 42
#define SYS_read_buffer 43

#define PRINT_HANDLER "print handler"


#define NWAIT 5
struct semaphore
{
    uint locked;
    int max_available;
    int current_occupied;
    struct proc* waiting_list[NWAIT];
    int waiting_start;
    int waiting_end;
};
