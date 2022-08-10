#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "condvar.h"

#define NSEMS 5
#define BUFF_SIZE 5

int pcbuf_start = 0;
int pcbuf_end = 0;
int pc_buffer[BUFF_SIZE] = {0, 0, 0, 0, 0};

int global_status;

struct condvar canread;
struct condvar canwrite;
struct spinlock1 condlock;

int rcnt;  // no. of readers 
int wcnt;  // no. of writers  
int waitr; // no. of readers waiting 
int waitw; // no. of writers waiting 
int shared_var = 0;

void RR_scheduling(void);
void lottery_scheduling(void);
void BJF_scheduling(void);
int random_seed = 3;

char* states_string[6] = {"unused", "embryo", "sleeping", "runnable", "running", "zombie"};

int
rand()
{
  random_seed = random_seed * 989457 + 9546817623;
  return random_seed;
}

int power(int ten, int n)
{
  int result = ten;
  
  for(int i = 0; i < n - 1 ; i++)
  {
    result = result * ten;
  }
  return result;
}

int countDigit(int n)
{
  if (n == 0)
    return 0;
  return 1 + countDigit(n / 10);
}

float rand_zero_to_one()
{
  int number = rand();
  int numofdigits = countDigit(number);
  int tavan = power(10, numofdigits);
  float res = (float)number / tavan;

  return res; 
}

float itof(int int_num)
{ 
  int numofdigits = countDigit(int_num);
  int tavan = power(10, numofdigits);
  float res = (float)int_num / tavan;
  return res; 
}

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
  int syscalls_count[NPROC][NSYSCALLS];
  int syscount_status;
  int RR_process;
  struct semaphore semaphores[NSEMS];
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);
static void wakeup2(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  // p->tracing_stat = NOTTRACING; //dorin set initial tracing stat to not tracing
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  p->level = 2;   //setting level default to 2
  p->tickets = 10;  //tickets default to 10
  p->waiting_cycles = 0;
  acquire(&tickslock);
  p->arrival_time = ticks;
  release(&tickslock);
  p->executes_cycle = 0;
  p->arrival_time_ratio = rand_zero_to_one();
  p->executes_cycle_ratio = rand_zero_to_one();
  p->priority_ratio = rand_zero_to_one();

  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");


  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  p->state = RUNNABLE;

  release(&ptable.lock);

  // fork();

}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();


  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  acquire(&ptable.lock);

  np->state = RUNNABLE;

  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.



void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;
  
  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      c->proc = p;
      switchuvm(p);
      p->state = RUNNING;

      swtch(&(c->scheduler), p->context);
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;
    }
    release(&ptable.lock);

  }
}

// void
// scheduler(void)
// {
//   struct proc *p;
//   struct cpu *c = mycpu();
//   c->proc = 0;
  
//   for(;;){
//     // Enable interrupts on this processor.
//     sti();

//     int level1 = 0, level2 = 0, level3 = 0;


//     // Loop over process table looking for process to run.
//     acquire(&ptable.lock);

//     for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
//       if(p->state != RUNNABLE)continue;
//       else{
//         if(p->level == 1) level1 = 1;
//         if(p->level == 2) level2 = 1;
//         if(p->level == 3) level3 = 1;

//       }
//     }

//     if(level1)
//       RR_scheduling();
//     else if(level2){
//       // cprintf("__LOTTERY SCHEDULING SELECTED\n");
//       lottery_scheduling();
//     }
//       // print()
//     else if(level3){    
//       BJF_scheduling(); 
//     }
//     //   // Switch to chosen process.  It is the process's job
//     //   // to release ptable.lock and then reacquire it
//     //   // before jumping back to us.
//     //   c->proc = p;
//     //   switchuvm(p);
//     //   p->state = RUNNING;

//     //   swtch(&(c->scheduler), p->context);
//     //   switchkvm();

//     //   // Process is done running for now.
//     //   // It should have changed its p->state before coming back.
//     //   c->proc = 0;
//     // }
//     release(&ptable.lock);

//   }
// }

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

static void
wakeup2(void *chan)
{
  acquire(&ptable.lock);
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
  {
    if(p->state == SLEEPING && *(int*)p->chan == *(int*)chan)
    {
      p->state = RUNNABLE;
    }
  }

  release(&ptable.lock);
}

void
sleep1(void *chan)
{
  struct proc *p = myproc();

  if(p == 0)
    panic("sleep");

  acquire(&ptable.lock);
  p->chan = chan;
  p->state = SLEEPING;

  sched();
  p->chan = 0;
  release(&ptable.lock);
}

void cv_wait(struct condvar *cv)
{
  //cprintf("Wait pid is:  %d\n", cv->pid);
  sleep1(&cv->pid);
}

void cv_signal(struct condvar *cv)
{
  //cprintf("Signal pid is:  %d\n", cv->pid);
  wakeup2(&cv->pid);
}

void init_sl()
{
  init_lock(&condlock);
}

void
init_condvar(struct condvar *cv)
{
    init_lock(&cv->lk); 
    cv->locked = 0;
    cv->pid = myproc()->pid;
}

void init_cv()
{
  init_condvar(&canread);
  init_condvar(&canwrite);
}

void beginread(int i) 
{ 
  lock(&condlock);
  
  // if there are active or waiting writers 
  if (wcnt == 1 || waitw > 0) 
  { 
      // incrementing waiting readers 
      waitr++;
      cv_wait(&canread);
      waitr--; 
  }
  
  // else reader reads the resource 
  rcnt++;
  cprintf("Reader %d is reading! Shared variable is: %d \n", i, shared_var);
  cv_signal(&canread);
  unlock(&condlock);

  // if there are no readers left then writer enters monitor 
  lock(&condlock);
  if (--rcnt == 0) 
      cv_signal(&canwrite);
  unlock(&condlock);  
} 

void read_p(int id)
{
  beginread(id);
}

void beginwrite(int i) 
{ 
  lock(&condlock);
  
  // a writer can enter when there are no active or waiting readers or other writer
  if (wcnt == 1 || rcnt > 0) 
  { 
      ++waitw; 
      cv_wait(&canwrite); 
      --waitw; 
  } 
  
  wcnt = 1;
  shared_var++;
  cprintf("Writer %d is writing! New Shared variable is: %d \n", i, shared_var);
  unlock(&condlock);  

  lock(&condlock); 
  wcnt = 0;
  
  if (waitr > 0) 
    cv_signal(&canread); 
  else
    cv_signal(&canwrite); 
  
  unlock(&condlock);
} 

void write_p(int id)
{
  beginwrite(id);
}
// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}

int
get_parent_id()
{
  struct proc *p = myproc();
  return p->parent->pid;
}


  //todo
int
get_process_children(int pid)
{
  struct proc *p;
  int children =0;
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    if (p->parent->pid == pid) {
      children = children*10 +p->pid;
    }
  }
  
  release(&ptable.lock);
  return children;
}

int num_of_degits(int num)
{
  int digits = 0;
  while(num > 0)
  {
    num = num/10;
    digits++;
  }
  return digits;
}

int pow(int ten, int n)
{
  for(int i=0;i<n-1;i++)
  {
    ten = ten * ten;
  }
  return ten;
}

int
get_children(int pid)
{

  int all_children = get_process_children(pid);
  int num_of_all_children = num_of_degits(all_children);
  int final = all_children;
  
  if(num_of_all_children == 1 && all_children == 0)
  {
      return 0;
  }

  return final;
}

int
get_family(int pid)
{
  int cur_pid;
  int num_of_cur_children;
  int cur_children;
  int all_children = get_process_children(pid);
  int num_of_all_children = num_of_degits(all_children);
  int final = all_children;
  int i=0;

  if(num_of_all_children == 1 && all_children == 0)
  {
      return 0;
  }
  while(i <= num_of_all_children)
  { 
    if(all_children < 10){
      cur_pid = all_children;
    }
    else
    {
      cur_pid = all_children / pow(10,(num_of_all_children-1));
    }
    
    cur_children = get_process_children(cur_pid);
    num_of_cur_children = num_of_degits(cur_children);
  
    if(cur_children != 0)
    {
      final = final* pow(10,num_of_cur_children) + cur_children;
      all_children = all_children* pow(10,num_of_cur_children) + cur_children;
       num_of_all_children = num_of_all_children + num_of_cur_children;
      i = i+1;
      
    }
    else
    {
      i = i+1;
      num_of_cur_children = 0;
    }
    all_children = all_children % pow(10,num_of_all_children-1);
    num_of_all_children = num_of_all_children  - 1;
  }
  return final;
}

static char* sys_names[] = {
[SYS_fork]    "sys_fork",
[SYS_exit]    "sys_exit",
[SYS_wait]    "sys_wait",
[SYS_pipe]    "sys_pipe",
[SYS_read]    "sys_read",
[SYS_kill]    "sys_kill",
[SYS_exec]    "sys_exec",
[SYS_fstat]   "sys_fstat",
[SYS_chdir]   "sys_chdir",
[SYS_dup]     "sys_dup",
[SYS_getpid]  "sys_getpid",
[SYS_sbrk]    "sys_sbrk",
[SYS_sleep]   "sys_sleep",
[SYS_uptime]  "sys_uptime",
[SYS_open]    "sys_open",
[SYS_write]   "sys_write",
[SYS_mknod]   "sys_mknod",
[SYS_unlink]  "sys_unlink",
[SYS_link]    "sys_link",
[SYS_mkdir]   "sys_mkdir",
[SYS_close]   "sys_close",
[SYS_get_parent_id]  "sys_get parent_id",
[SYS_get_children]   "sys_get_childeren", 
[SYS_get_family]     "sys_get_family",
[SYS_trace_syscalls] "sys_trace_syscalls",
[SYS_print_syscalls_handler] "sys_print_syscalls_handler",
[SYS_reverse_number] "sys_reverse_number",
[SYS_print_procs]    "sys_print_procs",
[SYS_set_level]      "sys_set_level",
[SYS_set_tickets]     "sys_set_tickets",
[SYS_set_bjf_params_proc]   "sys_set_bjf_params_proc",
[SYS_set_bjf_params_system] "sys_set_bjf_params_system",
// [SYS_init_lock] "sys_init_lock",
// [SYS_lock] "sys_lock",
// [SYS_unlock] "sys_unlock"
};

int
trace_syscalls(int status)
{
  for(int i = 0; i < NPROC; i++){
    struct proc* p = &ptable.proc[i];
    p->sys_count_stat = status;
  }
  global_status = status;
  if(status)
  {
    cprintf("**trace system calls start\n");
    ptable.syscount_status = status;  
    acquire(&ptable.lock);
    for(int i = 0; i < NPROC; i++){
      struct proc* p = &ptable.proc[i];
      for(int j = 0; j < NSYSCALLS; j++){
        p->syscalls_count[j] = 0;
      }
    }
    release(&ptable.lock);
  }
  else
  {
    cprintf("**trace system calls end\n");
  }
  for(int i = 0; i < NPROC; i++){
    struct proc* p = &ptable.proc[i];
    if(strncmp(p->name, PRINT_HANDLER, sizeof(p->name)) == 0){
      return p->sys_count_stat;
    }
  }
  return 4;
}


void 
print_syscalls(){
  for(int i = 0; i < NPROC; i++){
    struct proc* p = &ptable.proc[i];
    if(p->pid != 0){
      cprintf("proc name: %s, proc id: %d\n", p->name, p->pid);

      for(int j = 0; j < NSYSCALLS; j++){
        if(p->syscalls_count[j] != 0)
          cprintf("    -%s: %d\n", sys_names[j+1], p->syscalls_count[j]);
      }
    }
  }

}



void
print_syscalls_handler()
{
  uint startticks;
  uint currticks;
  int count_stat = 0;
  struct proc* currproc = myproc();
  currproc->sys_count_stat = 0;
  safestrcpy(currproc->name, PRINT_HANDLER, sizeof(PRINT_HANDLER));

  for(;;)
  {
    for(int i = 0; i < NPROC; i++){
      struct proc* p = &ptable.proc[i];
      if(strncmp(p->name, PRINT_HANDLER, sizeof(p->name)) == 0){
        count_stat = p->sys_count_stat;
        if(count_stat)
        break;
      }
    }
    if(global_status == 1)
    {
      cprintf("start printing syscall trace");
      acquire(&tickslock);
      startticks = ticks;
      release(&tickslock); 
      for(;;)
      {

        if(global_status == 0){
          break;
        }
        currticks = ticks;
        while(currticks - startticks < 1000)
        {
          acquire(&tickslock);
          currticks = ticks;
          release(&tickslock); 
        }
        if(global_status == 1){
          print_syscalls();
          startticks = currticks;
        }
      }
    }
  }

}


void
print_procs(void){
  struct proc *p;
  cprintf("Name \t PID \t State \t Level \t Tickets \t Waited\n");
  for(int i = 0; i < NPROC; i++){
    p = &ptable.proc[i];
    if(p->state == UNUSED)
      continue;
    cprintf("%s \t %d \t %s \t %d \t %d \t %d \n", 
      p->name,
      p->pid,
      states_string[p->state], 
      p->level,
      p->tickets,
      p->waiting_cycles);
  }
}



void
set_tickets(int pid, int tickets){
  struct proc *p;
  for(int i = 0; i < NPROC; i++){
    p = &ptable.proc[i];
    if(p->pid == pid){
      p->tickets = tickets;
      break;
    }
  }
}


void
set_level(int pid, int level){
  struct proc *p;
  for(int i = 0; i < NPROC; i++){
    p = &ptable.proc[i];
    if(p->pid == pid){
      p->level = level;
      p->waiting_cycles = 0;  //reseting waited cycles
      break;
    }

  }
}

void
set_bjf_params_proc(int pid, int pr, int atr, int ecr){
  float priority_ratio_ = itof(pr);
  float arrival_time_ratio_ = itof(atr);
  float executes_cycle_ratio_ = itof(ecr);
  struct proc *p;
  for(int i = 0; i < NPROC; i++){
    p = &ptable.proc[i];
    if(p->pid == pid){
      p->priority_ratio = priority_ratio_;
      p->arrival_time_ratio = arrival_time_ratio_;
      p->executes_cycle_ratio = executes_cycle_ratio_;
      break;
    }
  }
}

void
set_bjf_params_system(int pr, int atr, int ecr){

  float priority_ratio_ = itof(pr);
  float arrival_time_ratio_ = itof(atr);
  float executes_cycle_ratio_ = itof(ecr);  
  struct proc *p;
  for(int i = 0; i < NPROC; i++){
    p = &ptable.proc[i];
    p->priority_ratio = priority_ratio_;
    p->arrival_time_ratio = arrival_time_ratio_;
    p->executes_cycle_ratio = executes_cycle_ratio_;
  }
}


void
age_procs(int running_proc){
  struct proc *p;
  for(int i =0; i < NPROC; i++){
    p = &ptable.proc[i];
    if(p->state == UNUSED)
      continue;
    if(p->pid != running_proc){
      p->waiting_cycles += 1;
      if(p->waiting_cycles >= 10000)
        set_level(p->pid, 1);

    }
  }
}


void
context_switch(struct proc* p, struct cpu* c){
  if(p->state != RUNNABLE) return;
  p->executes_cycle += 0.1;
  c->proc = p;
  switchuvm(p);
  p->state = RUNNING;
  swtch(&(c->scheduler), p->context);
  switchkvm();
  c->proc = 0;
  age_procs(p->pid);
}

void
RR_scheduling(void){
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;

  if((ptable.RR_process == 0) || (ptable.RR_process == NPROC))
    ptable.RR_process = 0;
  
  for(; ptable.RR_process < NPROC; ptable.RR_process++){
    p = &ptable.proc[ptable.RR_process];
    if(p->state == RUNNABLE && p->level == 1){
      context_switch(p, c);
      ptable.RR_process++;
      return;
    }
  }
}

void
lottery_scheduling(void){
  struct proc *p;
  struct cpu *c = mycpu();
  int total_tickets = 0;
  for(int i = 0; i < NPROC; i++){
    p = &ptable.proc[i];
    if(p->level == 2)
      total_tickets += p->tickets;
  }

  int random = rand() % (total_tickets) + 1;
  // cprintf("random number selected : %d\n", random);
  int count_tickets = 0;
  for(int i = 0; i < NPROC; i++){
    p = &ptable.proc[i];
    if(p->level == 2){
      count_tickets += p->tickets;
      if(random <= count_tickets){
        context_switch(p, c);
        return;
      }
    }
  }
}

void
BJF_scheduling(void){
  struct proc *p;
  struct cpu *c = mycpu();
  float min_bjf = -1;
  int min_bjf_pid = -1;

  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
  {
    if (p->level == 3 && p->state == RUNNABLE)
    {
      float curr_bjf = ((float)p->arrival_time * p->arrival_time_ratio) + 
                       (p->executes_cycle * p->executes_cycle_ratio) + 
                       ((1 / (float)p->tickets) * p->priority_ratio);
      
      if (curr_bjf < min_bjf)
      {
        min_bjf = curr_bjf;
        min_bjf_pid = p->pid;
      }
    }
  }

  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
  {
    if (p->level == 3 && p->state == RUNNABLE && p->pid == min_bjf_pid)
    {
      context_switch(p, c);
      break;
    }
  }
}




void
print_semaphores()
{
  struct semaphore* sem;
  for(int i = 0; i < NSEMS; i++){
    sem = &ptable.semaphores[i];
    cprintf("semaphore id: %d - max available : %d - current_occupied : %d, waiting start and end: %d, %d\n",
      i, sem->max_available, sem->current_occupied, sem->waiting_start, sem->waiting_end);
  }
}


void
semaphore_initialize(int i, int v, int m)
{
  pushcli();
  struct semaphore *sem = &ptable.semaphores[i];
  sem->max_available = v;
  sem->current_occupied = m;
  sem->waiting_start = 0;
  sem->waiting_end = 0;
  popcli();
  print_semaphores();
}



void
semaphore_acquire(int i)
{
  pushcli();
  struct proc *curproc = myproc();
  struct semaphore *sem = &ptable.semaphores[i];
  sem->current_occupied ++;
  if(sem->current_occupied > sem->max_available){
    sem->waiting_list[sem->waiting_end] = curproc;
    sem->waiting_end = (sem->waiting_end + 1) % NWAIT;
    cprintf("process sleeping\n");
    // acquire(&ptable.lock);
    // curproc->state = SLEEPING;
    // release(&ptable.lock);
    sleep(curproc, &ptable.lock);
    // cprintf("released");
    // sched();

  }
  popcli();
  print_semaphores();
}


void
semaphore_release(int i)
{
  pushcli();
  struct proc *nextproc;
  struct semaphore *sem = &ptable.semaphores[i];
  sem->current_occupied --;
  if(sem->current_occupied >= sem->max_available){
    nextproc  = sem->waiting_list[sem->waiting_start];
    // acquire(&ptable.lock);
    // nextproc->state = RUNNABLE;
    // release(&ptable.lock);
    wakeup(nextproc);
    sem->waiting_start = (sem->waiting_start + 1) % NWAIT;
    // sched();
  }
  // cprintf("semaphore id: %d - max available : %d - current_occupied : %d, waiting start and end: %d, %d\n",
      // i, sem->max_available, sem->current_occupied, sem->waiting_start, sem->waiting_end);
  popcli();
  print_semaphores();
  
}



void
write_buffer(int value)
{
  pc_buffer[pcbuf_end] = value;
  cprintf("write %d to buffer number %d\n", value, pcbuf_end);
  cprintf("buffer: %d - %d - %d - %d - %d\n", pc_buffer[0], pc_buffer[1], pc_buffer[2], pc_buffer[3], pc_buffer[4]);
  pcbuf_end = (pcbuf_end + 1) / BUFF_SIZE;
}


void
read_buffer(void)
{
  cprintf("read %d from buffer\n", pc_buffer[pcbuf_start]);
  pcbuf_start = (pcbuf_start + 1) % BUFF_SIZE;
  cprintf("buffer: %d - %d - %d - %d - %d\n", pc_buffer[0], pc_buffer[1], pc_buffer[2], pc_buffer[3], pc_buffer[4]);

}