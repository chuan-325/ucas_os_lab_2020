#ifndef INCLUDE_MM_H_
#define INCLUDE_MM_H_
#include "sched.h"
#include "type.h"

#define PT_SIZE 0x100

typedef struct pte {
  uint32_t flag_eo, // even or odd
      valid;
  uint64_t entryhi, entrylo;
} pte_t; // 4*2+8*2=24B

extern pte_t *pt[PT_SIZE];

void init_page_table();
void do_TLB_Refill();

void do_page_fault();
void init_TLB(void);
void physical_frame_initial(void);

#endif
