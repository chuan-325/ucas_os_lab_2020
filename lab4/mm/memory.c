#include "mm.h"

void init_tlb_entry(void) {
  uint64_t i;
  for (i = 0; i < 64; i++) {

    pte_t *pte_even, *pte_odd;
    pte_even = (pte_t *)(PTE_BASE_ADDR + (i * 2) * sizeof(pte_t));
    pte_odd = (pte_t *)(PTE_BASE_ADDR + (i * 2 + 1) * sizeof(pte_t));

    set_cp0_entryhi(pte_even->entryhi);
    set_cp0_entrylo0(pte_even->entrylo);
    set_cp0_entrylo1(pte_odd->entrylo);
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

    pte_t *pte_even, *pte_odd;
    pte_even = (pte_t *)(PTE_BASE_ADDR + (i * 2) * sizeof(pte_t));
    pte_odd = (pte_t *)(PTE_BASE_ADDR + (i * 2 + 1) * sizeof(pte_t));

    pte_even->flag_eo = 0;
    pte_odd->flag_eo = 1;

    pte_even->valid = pte_odd->valid = 1;
    pte_even->entryhi = pte_odd->entryhi = enhi;

    pte_even->entrylo = enl0;
    pte_odd->entrylo = enl1;
  }
}

void do_TLB_Refill() {}

void do_page_fault() {}

void physical_frame_initial(void) {}