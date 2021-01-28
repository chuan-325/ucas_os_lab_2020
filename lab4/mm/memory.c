#include "mm.h"

pte_t pt[PT_SIZE] = {0}; // 1-level

void init_tlb_entry(void) {
  uint64_t i;
  for (i = 0; i < 64; i++) {
    uint64_t enhi = (i << 13); // |(asid&0xff)
    uint64_t dpfn = (i << 1) + 0x8000;
    uint64_t enl0 =
        (dpfn << 6) | (2 << 3) | (1 << 2) | (1 << 1) | (1); // d-pfn/c/d/v/g
    uint64_t enl1 = enl0 | (1 << 6);                        // d-pfn/c/d/v/g
    set_cp0_entryhi(enhi);
    set_cp0_entrylo0(enl0);
    set_cp0_entrylo1(enl1);
    set_cp0_pagemask(0);
    set_cp0_index(i);
    tlbwi_operation();
  }
}
void init_page_table() {
  uint64_t i;
  for (i = 0; i < 64; i++) {
    uint64_t enhi = (i << 13); // |(asid&0xff)
    uint64_t dpfn = (i << 1) + 0x8000;
    uint64_t enl0 =
        (dpfn << 6) | (2 << 3) | (1 << 2) | (1 << 1) | (1); // d-pfn/c/d/v/g
    uint64_t enl1 = enl0 | (1 << 6);                        // d-pfn/c/d/v/g

    pt[i * 2].flag_eo = 0;
    pt[i * 2 + 1].flag_eo = 1;
    pt[i * 2].valid = pt[i * 2 + 1].valid = 1;
    pt[i * 2].entryhi = pt[i * 2 + 1].entryhi = enhi;
    pt[i * 2].entrylo = enl0;
    pt[i * 2 + 1].entrylo = enl1;
  }
}

void do_TLB_Refill() {}

void do_page_fault() {}

void physical_frame_initial(void) {}