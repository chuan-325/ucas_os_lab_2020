/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * *
 * * * * * * Copyright (C) 2018 Institute of Computing Technology, CAS Author :
 * Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * *
 * * * * * * The kernel's entry, where most of the initialization work is done.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * *
 * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
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
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * *
 * * * * * */

#include "common.h"
#include "fs.h"
#include "irq.h"
#include "mac.h"
#include "mm.h"
#include "sched.h"
#include "screen.h"
#include "smp.h"
#include "stdio.h"
#include "string.h"
#include "syscall.h"
#include "test.h"
#include "time.h"

#define TASK_INIT (00)
static void init_memory() {}

static void init_pcb() {
  process_id = 0; // kernel process

  // Init
  int i;
  for (i = 1; i < NUM_MAX_TASK; i++)
    pcb[i].status = TASK_EXITED; // all available

  queue_init(&ready_queue);
  queue_init(&block_queue);
  queue_init(&sleep_queue);

  struct task_info task0 = {"task0", (uint64_t)&test_shell, USER_PROCESS, 5};

  // pcb0
  pcb[0].kernel_stack_top = ADDR_KNSTACK_BASE; // sp
  pcb[0].user_stack_top = ADDR_USSTACK_BASE;
  pcb[0].block_me = NULL;
  pcb[0].prev = NULL;
  pcb[0].next = &pcb[1];
  queue_init(&(pcb[0].lock_queue));
  queue_init(&(pcb[0].wait_queue));
  set_pcb(0, &pcb[0], &task0, 0);
  queue_push(&ready_queue, &pcb[0]);

  pcb[0].cursor_x = SHELL_LEFT_LOC;
  pcb[0].cursor_y = SHELL_BOUNDARY;

  // pcb1
  pcb[1].kernel_stack_top = ADDR_KNSTACK_BASE + SIZE_KERNEL_STACK; // sp
  pcb[1].user_stack_top = ADDR_USSTACK_BASE + SIZE_USER_STACK;
  pcb[1].block_me = NULL;
  pcb[1].prev = NULL;
  pcb[1].next = &pcb[0];
  pcb[1].kernel_context.regs[29] = pcb[1].kernel_stack_top;
  pcb[1].user_context.regs[29] = pcb[1].user_stack_top;

  current_running = &pcb[1];
}

static void init_binsem() {
  int i;
  for (i = 0; i < BINSEM_NUM; i++)
    binsem[i].taken = 0;
}

static void init_exception_handler() {
  int i;
  for (i = 0; i < 32; i++)
    exception_handler[i] = (uint64_t)handle_other;

  exception_handler[INT] = (uint64_t)handle_int;
  exception_handler[SYS] = (uint64_t)handle_syscall;
  exception_handler[TLBL] = (uint64_t)handle_tlb;
  exception_handler[TLBS] = (uint64_t)handle_tlb;
}

static void init_exception() {
  /*
   * 1. Copy the level 2 exception handling code to 0xffffffff80000180
   * 2. Set EXC table
   * 3. reset CP0 regs
   */
  // 1
  uint8_t *exc_h;
  exc_h = (uint8_t *)0xffffffff80000180;
  uint32_t exc_h_size = exception_handler_end - exception_handler_begin;
  memcpy(exc_h, (uint8_t *)exception_handler_begin, exc_h_size);
  // 2
  init_exception_handler();
  // 3: reset
  reset_timer();                   // cnt
  set_cp0_compare(TIMER_INTERVAL); // cmp
  set_cp0_status(0x10008000);      // status: EXL=0, IE=0
}

// [2]
// extern int read_shell_buff(char *buff);

static void init_syscall(void) {
  syscall[SYSCALL_SPAWN] = (uint64_t(*)())(&do_spawn);
  syscall[SYSCALL_EXIT] = (uint64_t(*)())(&do_exit);
  syscall[SYSCALL_SLEEP] = (uint64_t(*)())(&do_sleep);
  syscall[SYSCALL_KILL] = (uint64_t(*)())(&do_kill);
  syscall[SYSCALL_WAITPID] = (uint64_t(*)())(&do_waitpid);
  syscall[SYSCALL_PS] = (uint64_t(*)())(&do_process_show);
  syscall[SYSCALL_GETPID] = (uint64_t(*)())(&do_getpid);
  syscall[SYSCALL_GET_TIMER] = (uint64_t(*)())(&get_timer);
  syscall[SYSCALL_SCHEDULER] = (uint64_t(*)())(&do_scheduler);

  syscall[SYSCALL_WRITE] = (uint64_t(*)())(&screen_write);
  // syscall[SYSCALL_READ]
  syscall[SYSCALL_CURSOR] = (uint64_t(*)())(&screen_move_cursor);
  syscall[SYSCALL_REFLUSH] = (uint64_t(*)())(&screen_reflush);
  syscall[SYSCALL_SERIAL_READ] = (uint64_t(*)())(&serial_read);
  syscall[SYSCALL_SCREEN_CLEAR] = (uint64_t(*)())(&screen_clear);
  // syscall[SYSCALL_SERIAL_READ]

  syscall[SYSCALL_MUTEX_LOCK_INIT] = (uint64_t(*)())(&do_mutex_lock_init);
  syscall[SYSCALL_MUTEX_LOCK_ACQUIRE] = (uint64_t(*)())(&do_mutex_lock_acquire);
  syscall[SYSCALL_MUTEX_LOCK_RELEASE] = (uint64_t(*)())(&do_mutex_lock_release);

  syscall[SYSCALL_BINSEM_GET] = (uint64_t(*)())(&do_binsemget);
  syscall[SYSCALL_BINSEM_OP] = (uint64_t(*)())(&do_binsemop);
}

/* [0] The beginning of everything */
void __attribute__((section(".entry_function"))) _start(void) {

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

  /* init binsem */
  init_binsem();
  printk("> [INIT] Binsem initialization succeeded.\n");

  /* init tlb entry */
  init_tlb_entry();
  printk("> [INIT] TLB entries initialization succeeded.\n");

  /* init screen */
  init_screen();
  // printk("> [INIT] SCREEN initialization succeeded.\n");

  /* init filesystem */
  //?read_super_block();

  /* wake up core1*/
  //?loongson3_boot_secondary();

  /* set cp0_status register to allow interrupt */
  // enable exception and interrupt
  // ERL = 0, EXL = 0, IE = 1
  uint32_t status_ival = get_cp0_status();
  status_ival |= 0x1;
  reset_timer();    // reset count
  time_elapsed = 0; // global time reset
  set_cp0_status(status_ival);

  while (1) {
    // do_scheduler();
  };
  return;
}
