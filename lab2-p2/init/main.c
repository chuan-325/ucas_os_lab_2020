/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *         The kernel's entry, where most of the initialization work is done.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#include "fs.h"
#include "irq.h"
#include "test.h"
#include "stdio.h"
#include "sched.h"
#include "screen.h"
#include "common.h"
#include "syscall.h"
#include "smp.h"
#include "mm.h"
#include "mac.h"

#define TASK_INIT (00)
static void init_memory()
{
}

// pcb initialization
static void init_pcb()
{
    process_id = 0; // kernel process

    // Init
    int i;
    for (i = 1; i < NUM_MAX_TASK; i++)
    {
        pcb[i].status = TASK_EXITED; // all available
    }
    // Clear Queues
    queue_init(&ready_queue);
    queue_init(&block_queue);
    // Set pcb[0]
    pcb[0].kernel_stack_top = ADDR_KNSTACK_BASE;
    pcb[0].prev = NULL;
    pcb[0].next = &pcb[1]; // when init, the next must be 1
    pcb[0].pid = 0;
    current_running = &pcb[0];
    // Load Tasks
    for (i = 0; i < 3; i++)
    {
        int index = alloc_pcb();
        set_pcb(++process_id, &pcb[index], sched1_tasks[i]); // sched1
        queue_push(&ready_queue, &pcb[index]);
    }
    for (; i < 5; i++)
    {
        int index = alloc_pcb();
        set_pcb(++process_id, &pcb[index], lock_tasks[i - 3]); // lock1
        queue_push(&ready_queue, &pcb[index]);
    }
}

static void init_exception_handler()
{
}

static void init_exception()
{

    /* fill nop */

    /* fill nop */

    /* set COUNT & set COMPARE */

    /* open interrupt */
}

// [2]
// extern int read_shell_buff(char *buff);

static void init_syscall(void)
{
}

/* [0] The beginning of everything */
void __attribute__((section(".entry_function"))) _start(void)
{

    asm_start();

    /* init stack space */
    init_stack();
    printk("> [INIT] Stack initialization succeeded.\n");

    /* init interrupt */
    init_exception();
    printk("> [INIT] Interrupt processing initialization succeeded.\n");

    init_memory();
    printk("> [INIT] Virtual memory initialization succeeded.\n");

    /* init system call table */
    init_syscall();
    printk("> [INIT] System call initialized successfully.\n");

    /* init Process Control Block */
    init_pcb();
    printk("> [INIT] PCB initialization succeeded.\n");

    /* init screen */
    init_screen();
    printk("> [INIT] SCREEN initialization succeeded.\n");

    /* init filesystem */
    //?read_super_block();

    /* wake up core1*/
    //?loongson3_boot_secondary();

    /* set cp0_status register to allow interrupt */
    // enable exception and interrupt
    // ERL = 0, EXL = 0, IE = 1

    while (1)
    {
        do_scheduler(); // prj2-1
    };
    return;
}
