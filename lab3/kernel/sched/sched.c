#include "lock.h"
#include "time.h"
#include "stdio.h"
#include "sched.h"
#include "queue.h"
#include "screen.h"
#include "string.h"
#include "irq.h"

pcb_t pcb[NUM_MAX_TASK];

// current running task ptr
pcb_t *current_running;
// global process id
pid_t process_id = 1;
// global.prior
uint32_t global_prior = PRIOR_MAX;
// kernel stack
static uint64_t kernel_stack[NUM_KERNEL_STACK];
static int kernel_stack_count;
// user stack
static uint64_t user_stack[NUM_KERNEL_STACK];
static int user_stack_count;

queue_t ready_queue;
queue_t block_queue;
queue_t sleep_queue;

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
	for (i = 1; i < NUM_MAX_TASK; i++) {
		if (pcb[i].status == TASK_EXITED) {
			bzero(&pcb[i], sizeof(pcb_t));
			pcb[i].kernel_stack_top = new_kernel_stack(i);
			pcb[i].user_stack_top = new_user_stack(i);
			pcb[i].status = TASK_READY;
			pcb[i].block_me = NULL;
			queue_init(&(pcb[i].lock_queue));
			queue_init(&(pcb[i].wait_queue));
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
	pcb->prior = task_info->prior;
	pcb->state = (task_info->type == USER_PROCESS ||
		      task_info->type == USER_THREAD) ?
			     STATE_USER :
			     STATE_KERNEL;

	int i;
	for (i = 0; i < 32; i++)
		pcb->name[i] = task_info->name[i];

	if (pid < NUM_MAX_TASK)
		pcb->next = (pcb_t *)(&pcb + sizeof(pcb_t));

	// init contexts
	// ra
	pcb->kernel_context.regs[31] = (uint64_t)exception_handler_exit;
	pcb->user_context.cp0_epc = task_info->entry_point;
	// sp
	pcb->user_context.regs[29] = pcb->user_stack_top;
	pcb->kernel_context.regs[29] = pcb->kernel_stack_top;
	// CPRs
	pcb->user_context.cp0_status = 0x10008002;
	pcb->kernel_context.cp0_status = 0x10008002;
}

static void check_sleeping()
{
	pcb_t *be_chk;
	be_chk = (pcb_t *)sleep_queue.head;
	while (be_chk != NULL) // && be_chk->next != NULL)
	{
		if (be_chk->wake_up_clk <= time_elapsed) {
			pcb_t *be_waked;
			be_waked = be_chk;
			be_waked->status = TASK_READY;
			queue_remove(&sleep_queue, be_waked);
			queue_push(&ready_queue, be_waked);
		}
		be_chk = be_chk->next;
	}
}

// scheduler:
//   switch from current 'current_running' to the next 'current_running'
void scheduler(void)
{
	check_sleeping();

	if (current_running->status == TASK_RUNNING)
		current_running->status =
			TASK_READY; // c_r(old): (running)->READY

	current_running->cursor_x = screen_cursor_x;
	current_running->cursor_y = screen_cursor_y;

	pcb_t *next_running;
	next_running = current_running->next; // init nr

	uint32_t nr_valid = 0;

	while (1) {
		while (next_running != NULL) { // check validity of next running
			if (next_running->prior >= global_prior) {
				nr_valid = 1;
				break;
			}
			next_running = next_running->next; // see whole queue
		}
		if (nr_valid)
			break;

		if (global_prior == 0) // reset global prior
			global_prior = PRIOR_MAX;
		else // update suitable global prior
			global_prior--;

		next_running =
			(pcb_t *)(ready_queue.head); // prepare for next query
	}

	current_running = next_running;

	screen_cursor_x = current_running->cursor_x;
	screen_cursor_y = current_running->cursor_y;

	current_running->status = TASK_RUNNING; // c_r(new): ?->RUNNING
}

// do_block:
//   block current_running to '*queue'
void do_block(queue_t *queue)
{
	pcb_t *be_block;
	be_block = current_running;

	queue_remove(&ready_queue, (void *)be_block);
	queue_push(queue, (void *)be_block);
	be_block->status = TASK_BLOCKED;
	be_block->block_me = queue;
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
	((pcb_t *)be_unblock)->block_me = NULL; //lab3
}

// do_unblock_all:
//   unblock all items in '*queue'
void do_unblock_all(queue_t *queue)
{
	while (!queue_is_empty(queue))
		do_unblock_one(queue);
}

int do_spawn(task_info_t *task)
{
	//TODO ok
	int i;
	i = alloc_pcb();
	if (i == -1)
		return -1;
	set_pcb(++process_id, &pcb[i], task);
	queue_push(&ready_queue, &pcb[i]);
	return 0;
}

void do_sleep(uint32_t sleep_time)
{
	current_running->wake_up_clk = sleep_time * SEC_SLICE + time_elapsed;
	do_block(&sleep_queue);
}

void do_exit(void)
{
	//TODO ok
	pcb_t *to_exit;
	to_exit = current_running;
	queue_remove(&ready_queue, current_running);

	// lock blocked
	while (!queue_is_empty(&(to_exit->lock_queue))) {
		mutex_lock_t *t;
		t = to_exit->lock_queue.head;
		do_unblock_all(&(t->blocked));
		queue_dequeue(&(to_exit->lock_queue));
	}
	//wait
	do_unblock_all(&(to_exit->wait_queue));

	to_exit->status = TASK_EXITED;
}

int do_kill(pid_t pid)
{
	//TODO ok
	int i, flag;
	flag = 0;
	for (i = 0; i < NUM_MAX_TASK; i++) {
		if (pcb[i].pid == pid && pcb[i].status != TASK_EXITED) {
			flag = 1;
			break;
		}
	}
	if (flag == 0) // not existed
		return -1;

	pcb_t *to_kill;
	to_kill = &pcb[i];
	if (to_kill->status == TASK_BLOCKED)
		queue_remove((queue_t *)(to_kill->block_me), to_kill);
	else
		queue_remove(&ready_queue, current_running);

	// lock blocked
	while (!queue_is_empty(&(to_kill->lock_queue))) {
		mutex_lock_t *t;
		t = to_kill->lock_queue.head;
		do_unblock_all(&(t->blocked));
		queue_dequeue(&(to_kill->lock_queue));
	}
	//wait
	do_unblock_all(&(to_kill->wait_queue));
	to_kill->status = TASK_EXITED;
	to_kill->block_me = NULL;

	return 0;
}

int do_waitpid(pid_t pid)
{
	//TODO ok
	int i, find;
	find = 0;
	for (i = 0; i < NUM_MAX_TASK; i++)
		if (pcb[i].pid == pid) {
			find = 1;
			break;
		}

	if (find == 0 && i == NUM_MAX_TASK - 1)
		return -1; // not existed

	if (pcb[i].status != TASK_EXITED)
		do_block(&(pcb[i].wait_queue));
}

// process show
void do_process_show()
{
	// TODO ok
	pcb[0].cursor_x = SHELL_LEFT_LOC;
	screen_move_cursor(pcb[0].cursor_x, ++pcb[0].cursor_y);
	printk("[PROC TABLE]\n");
	int i, num_ps = 0;
	for (i = 0; i < NUM_MAX_TASK; i++) // show running
	{
		if (pcb[i].status == TASK_RUNNING) {
			screen_move_cursor(pcb[0].cursor_x, ++pcb[0].cursor_y);
			printk("[%d] PID = %d STATUS = RUNNING\n", num_ps++,
			       pcb[i].pid);
		}
	}
}

pid_t do_getpid()
{
	// TODO ok
	return current_running->pid;
}

// which to switch? sequence sched
/*
    if (!current_running->next) // no next: head(ready_queue)
        current_running = (pcb_t *)(ready_queue.head);
    else // with next: next
        current_running = (pcb_t *)(current_running->next);
    */