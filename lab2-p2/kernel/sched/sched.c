#include "lock.h"
#include "time.h"
#include "stdio.h"
#include "sched.h"
#include "queue.h"
#include "screen.h"
#include "string.h"

pcb_t pcb[NUM_MAX_TASK];

// current running task ptr
pcb_t *current_running;
// global process id
pid_t process_id = 1;
// kernel stack
static uint64_t kernel_stack[NUM_KERNEL_STACK];
static int kernel_stack_count;
// user stack
static uint64_t user_stack[NUM_KERNEL_STACK];
static int user_stack_count;

// ready queue: to run
queue_t ready_queue;
// block queue: to wait
queue_t block_queue;

// init kernel stack / user stack
void init_stack()
{
    bzero((void *)ADDR_KNSTACK_BASE, NUM_MAX_TASK * SIZE_KERNEL_STACK);
    bzero((void *)ADDR_USSTACK_BASE, NUM_MAX_TASK * SIZE_USER_STACK);
}

// calc location of new kernel stack's top
static uint64_t new_kernel_stack(int index)
{
    uint64_t kn_sp;
    kn_sp = ADDR_KNSTACK_BASE + index * SIZE_KERNEL_STACK;
    return kn_sp;
}
// calc location of new user stack's top
static uint64_t new_user_stack(int index)
{
    uint64_t us_sp;
    us_sp = ADDR_USSTACK_BASE + index * SIZE_USER_STACK;
    return us_sp;
}

//
static void free_kernel_stack(uint64_t stack_addr)
{
}
//
static void free_user_stack(uint64_t stack_addr)
{
}

// alloc_pcb:
//   find a free pcb by the array
//   return [the alloced pcb_t's number] or [fail_info]
int alloc_pcb()
{
    int i;
    for (i = 1; i < NUM_MAX_TASK; i++)
    {
        if (pcb[i].status == TASK_EXITED)
        {
            bzero(&pcb[i], sizeof(pcb_t));
            pcb[i].kernel_stack_top = new_kernel_stack(i);
            pcb[i].user_stack_top = new_user_stack(i);
            pcb[i].status = TASK_READY;
            queue_init(&(pcb[i].lock_queue));
            return i;
        }
    }
    return -1;
}
// set_pcb:
//   input : info, *pcb_t
//   output: modified *pcb_t
void set_pcb(pid_t pid, pcb_t *pcb, task_info_t *task_info)
{
    // basic info
    pcb->pid = pid;
    pcb->type = task_info->type;
    // ra
    pcb->user_context.regs[31] = task_info->entry_point;
    // sp
    pcb->user_context.regs[29] = pcb->user_stack_top;
    pcb->kernel_context.regs[29] = pcb->kernel_stack_top;
    // no name
}

static void check_sleeping()
{
}

// scheduler:
//   switch from current 'current_running' to the next 'current_running'
void scheduler(void)
{
    current_running->status = TASK_READY; // c_r(old): (running)->READY
    // which to switch?
    if (!current_running->next)
    // no next: head(ready_queue)
    {
        current_running = (pcb_t *)(ready_queue.head);
    }
    else
    // with next: next
    {
        current_running = (pcb_t *)(current_running->next);
    }
    current_running->status = TASK_RUNNING; // c_r(new): ?->RUNNING
}

void do_sleep(uint32_t sleep_time)
{
}

void do_exit(void)
{
}

// do_block:
//   block current_running to '*queue'
void do_block(queue_t *queue)
{
    pcb_t *be_block;
    be_block = current_running;
    queue_remove(&ready_queue, (void *)be_block);
    queue_push(queue, (void *)be_block);
    do_scheduler();
    // after this we turn to the head of ready_queue
}

// do_unblock_one:
//   unblock head of '*queue' (single)
void do_unblock_one(queue_t *queue)
{
    void *be_unblock;
    if (queue_is_empty(queue))
        printk("queue is empty!");
    be_unblock = queue_dequeue(queue);
    queue_push(&ready_queue, be_unblock);
}

// do_unblock_all:
//   unblock all items in '*queue'
void do_unblock_all(queue_t *queue)
{
    while (!queue_is_empty(queue))
    {
        do_unblock_one(queue);
    }
}

int do_spawn(task_info_t *task)
{
}

int do_kill(pid_t pid)
{
}

int do_waitpid(pid_t pid)
{
}

// process show
void do_process_show()
{
}

pid_t do_getpid()
{
}