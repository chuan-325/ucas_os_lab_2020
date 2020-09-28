.text
.global main

main:
    # 1) call BIOS print string "It's bootloader!"
	la $a0, msg                 # Ad(msg) -> $a0
    ld $t0, printstr            # "printstr" -> $0
    jal $t0                     # jmp to "printstr"

    # 2) call BIOS read kernel in SD card
                                # a0, a1, a2 are 3 parameters prepared for read_sd_card()
    ld $a0, kernel              # dest: Ad(kernel) -> $a0
    li $a1, 512                 # uint32_t offset -> $a1
    li $a2, 0xffffffffa08001f0  # OS_size loc (optimize)
    ld $a2, ($a2)               # m(loc) -> $a2
    dsll $a2, $a2, 9
    ld $t0, read_sd_card        # "read_sd_card" -> $t0
    jal $t0                     # jmp to "read_sd_card"
                                # jal, link to $31, after the func it will execute the next 'jal'

	# 3) jump to kernel
    ld $t0, kernel              # Ad(kernel) -> $t0 (Note: GPRs is changed after read_sd_card)
    jal $t0                     # jmp to kernel

    # 4) while(1) --> stop here
    loop: j loop

.data

msg: .ascii "It's a bootloader!!!\n"

# 1. PMON read SD card function address
# read_sd_card();
read_sd_card: .dword 0xffffffff8f0d5e10

# 2. PMON print string function address
# printstr(char *string)
printstr: .dword 0xffffffff8f0d5534

# 3. PMON print char function address
# printch(char ch)
printch: .dword 0xffffffff8f0d5570

# 4. kernel address (move kernel to here ~)
kernel : .dword 0xffffffffa0800200

# 5. kernel main address (jmp here to start kernel main!)
kernel_main : .dword 0xffffffffa0800200
