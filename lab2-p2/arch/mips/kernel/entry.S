
#include "asm.h"

.equ    KERNEL, 0         # addr of reg_context
.equ    USER,   320
.equ    NESTED_COUNT, 640
.equ    TIMER_INTERVAL, 150000 # ref to macro definition in "irq.h"


LEAF(asm_start)
    mtc0    zero, CP0_STATUS
    mtc0    zero, CP0_WATCHLO
    mtc0    zero, CP0_WATCHHI

    mfc0    t0, CP0_CONFIG
    and     t0, ~0x7
    ori     t0, 0x2
    mtc0    t0, CP0_CONFIG

    jr  ra
END(asm_start)

.macro SAVE_CONTEXT offset
    .set    noat
    ld      k0, current_running
    daddi   k0, k0, \offset # select on USER/KERNEL context
    sd      $0,0(k0)
    sd      $2,16(k0)
    sd      $3,24(k0)
    sd      $4,32(k0)
    sd      $5,40(k0)
    sd      $6,48(k0)
    sd      $7,56(k0)
    sd      $8,64(k0)
    sd      $9,72(k0)
    sd      $10,80(k0)
    sd      $11,88(k0)
    sd      $12,96(k0)
    sd      $13,104(k0)
    sd      $14,112(k0)
    sd      $15,120(k0)
    sd      $16,128(k0)
    sd      $17,136(k0)
    sd      $18,144(k0)
    sd      $19,152(k0)
    sd      $20,160(k0)
    sd      $21,168(k0)
    sd      $22,176(k0)
    sd      $23,184(k0)
    sd      $24,192(k0)
    sd      $25,200(k0)
    sd      $28,224(k0)
    sd      $29,232(k0)
    sd      $30,240(k0)
    sd      $31,248(k0)
    mfc0    k1, CP0_STATUS
    sw      k1, 256(k0)
    mfc0    k1, CP0_CAUSE
    sw      k1, 264(k0)
    dmfc0    k1, CP0_EPC
    sd      k1, 288(k0)
    .set    at
.endm

.macro RESTORE_CONTEXT offset
    .set    noat
    ld      k0, current_running
    daddi   k0, k0, \offset
    ld      $0,0(k0)
    ld      $2,16(k0)
    ld      $3,24(k0)
    ld      $4,32(k0)
    ld      $5,40(k0)
    ld      $6,48(k0)
    ld      $7,56(k0)
    ld      $8,64(k0)
    ld      $9,72(k0)
    ld      $10,80(k0)
    ld      $11,88(k0)
    ld      $12,96(k0)
    ld      $13,104(k0)
    ld      $14,112(k0)
    ld      $15,120(k0)
    ld      $16,128(k0)
    ld      $17,136(k0)
    ld      $18,144(k0)
    ld      $19,152(k0)
    ld      $20,160(k0)
    ld      $21,168(k0)
    ld      $22,176(k0)
    ld      $23,184(k0)
    ld      $24,192(k0)
    ld      $25,200(k0)
    ld      $28,224(k0)
    ld      $29,232(k0)
    ld      $30,240(k0)
    ld      $31,248(k0)
    lw      k1, 256(k0)
    mtc0    k1, CP0_STATUS
    lw      k1, 264(k0)
    mtc0    k1, CP0_CAUSE
    ld      k1, 288(k0)
    dmtc0    k1, CP0_EPC
   .set    at
.endm

NESTED(do_scheduler, 0, ra)
    SAVE_CONTEXT(KERNEL)         # save context
    jal     scheduler            # modify current_running
    RESTORE_CONTEXT(KERNEL)      # restore context
    jr      ra                   # jump to entry
END(do_scheduler)

LEAF(reset_timer)
    mtc0    zero, CP0_COUNT
    jr      ra
END(reset_timer)

LEAF(set_cp0_status)
    mtc0    a0, CP0_STATUS
    jr      ra
END(set_cp0_status)

LEAF(set_cp0_cause)
    mtc0    a0, CP0_CAUSE
    jr      ra
END(set_cp0_cause)

LEAF(set_cp0_compare)
    mtc0    a0, CP0_COMPARE
    jr      ra
END(set_cp0_compare)

LEAF(get_cp0_status)
    mfc0    v0, CP0_STATUS
    jr      ra
END(get_cp0_status)

LEAF(get_cp0_cause)
    mfc0    v0, CP0_CAUSE
    jr      ra
END(get_cp0_cause)

LEAF(get_cp0_count)
    mfc0    v0, CP0_COUNT
    jr      ra
END(get_cp0_count)

LEAF(get_cp0_compare)
    mfc0    v0, CP0_COMPARE
    jr      ra
END(get_cp0_compare)


.global exception_handler_begin
.global exception_handler_end
.global TLBexception_handler_begin
.global TLBexception_handler_end

NESTED(TLBexception_handler_entry, 0, sp)
TLBexception_handler_begin:


TLBexception_handler_end:
END(TLBexception_handler_entry)

NESTED(exception_handler_entry, 0, sp)
exception_handler_begin:
    mfc0    k0, CP0_STATUS        # DISABLE INTR
    li      k1, 0xfffffffe
    and     k0, k0, k1
    mtc0    k0, CP0_STATUS
    ld      k0, current_running   # STATE: user->kernel
    daddi   k0, k0, NESTED_COUNT
    sw      zero, (k0)
    SAVE_CONTEXT(USER)
    mfc0    t0, CP0_STATUS
    mfc0    t1, CP0_CAUSE         # get excode
    ld      k0, current_running
    daddi   k0, k0, KERNEL        # select on USER/KERNEL context
    sd      a0, 32(k0) # a0~a3 parameters
    sd      a1, 40(k0)
    sd      a2, 48(k0)
    sd      a3, 56(k0)
    sw      t0, 256(k0) #status
    sw      t1, 264(k0) #cause
    RESTORE_CONTEXT(KERNEL)
    mfc0    k1, CP0_CAUSE
    andi    k1, k1, CAUSE_EXCCODE
    dsll    k1, k1, 0x1
    la      k0, exception_handler # load array's address
    dadd    k0, k0, k1            # get specific handler's address in array
    ld      k1, (k0)              # load handler func's entry
    jr      k1
exception_handler_end:
END(exception_handler_entry)

NESTED(handle_int, 0, sp)
    daddi   sp, sp, -16          # para(stack push)
    mfc0    a0, CP0_STATUS
    mfc0    a1, CP0_CAUSE
    jal     interrupt_helper
    daddi   sp, sp, 16           # para(stack pop)
    SAVE_CONTEXT(KERNEL)
    RESTORE_CONTEXT(USER)        # STATE: kernal->user
    li      k0, 0x1
    ld      k1, current_running
    daddi   k1, k1, NESTED_COUNT
    sw      k0,(k1)
    mfc0    k0, CP0_STATUS       # ENABLE INTR
    ori     k0, k0, 0x1
    mtc0    k0, CP0_STATUS
    eret                         # end
END(handle_int)

NESTED(handle_syscall, 0, sp)
    daddi   sp, sp, -32          # para(stack push)
    jal     system_call_helper
    daddi   sp, sp, 32           # para(stack pop)
    SAVE_CONTEXT(KERNEL)
    ld      k0, current_running
    daddi   k0, k0, USER         # select on USER/KERNEL context
    sd      v0, 16(k0)           # protect syscall's ret_val
    RESTORE_CONTEXT(USER)        # STATE: kernel->user
    li      k0, 0x1
    ld      k1, current_running
    daddi   k1, k1, NESTED_COUNT
    sw      k0,(k1)
    mfc0    k0, CP0_EPC          # prepare epc
    daddi   k0, k0, 0x4
    mtc0    k0, CP0_EPC
    mfc0    k0, CP0_STATUS       # ENABLE INTR
    ori     k0, k0, 0x1
    mtc0    k0, CP0_STATUS
    eret                         # end
END(handle_syscall)

NESTED(handle_tlb, 0, sp)

END(handle_tlb)

NESTED(handle_other, 0, sp)


END(handle_other)

LEAF(exception_handler_exit)
    SAVE_CONTEXT(KERNEL)
    RESTORE_CONTEXT(USER)        # STATE: kernal->user
    li      k0, 0x1
    ld      k1, current_running
    daddi   k1, k1, NESTED_COUNT
    sw      k0,(k1)
    mfc0    k0, CP0_STATUS       # ENABLE INTR
    ori     k0, k0, 0x1
    mtc0    k0, CP0_STATUS
    mtc0    zero, CP0_COUNT
    li      k0, TIMER_INTERVAL
    mtc0    k0, CP0_COMPARE
    eret                         # end
END(exception_handler_exit)

LEAF(set_cp0_entryhi)

END(set_cp0_entryhi)


LEAF(get_cp0_index)

END(get_cp0_index)

LEAF(set_cp0_index)

END(set_cp0_index)

LEAF(get_cp0_badvaddr)

END(get_cp0_badvaddr)

LEAF(get_cp0_entrylo0)

END(get_cp0_entrylo0)

LEAF(set_cp0_entrylo0)

END(set_cp0_entrylo0)

LEAF(get_cp0_entrylo1)

END(get_cp0_entrylo1)

LEAF(set_cp0_entrylo1)

END(set_cp0_entrylo1)

LEAF(set_cp0_pagemask)

END(set_cp0_pagemask)

LEAF(tlbwr_operation)

END(tlbwr_operation)

LEAF(tlbwi_operation)

END(tlbwi_operation)

LEAF(tlbp_operation)

END(tlbp_operation)

LEAF(get_cp0_config)

END(get_cp0_config)

LEAF(get_cpu_id)

END(get_cpu_id)

