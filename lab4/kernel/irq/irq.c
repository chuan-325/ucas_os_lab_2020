#include "irq.h"
#include "sched.h"
#include "string.h"
#include "time.h"

/* exception handler */
uint64_t exception_handler[32];

/* used to init PCB */
uint32_t initial_cp0_status;

// extern void do_shell();

static void irq_timer() {
  screen_reflush();

  time_elapsed += SEC_SLICE; // increase global time counter

  do_scheduler(); // sched.c to do scheduler
  reset_timer();  // reset count
  set_cp0_compare(TIMER_INTERVAL);
}

void interrupt_helper(uint32_t status, uint32_t cause) {
  int im = (status & 0xff00) >> 8;
  int ip = (cause & 0xff00) >> 8;
  int num = im & ip;
  if (num == 0x80) // clk intr
    irq_timer();
}

void other_exception_handler() {}