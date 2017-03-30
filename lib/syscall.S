#include <asm/regdef.h>
#include <asm/cp0regdef.h>
#include <asm/asm.h>
#include <stackframe.h>
#include <unistd.h>


NESTED(handle_sys,TF_SIZE, sp)

    //save cpu's registers
    SAVE_ALL

    //close interrput
    CLI
    nop
    .set at


    //Hint:refer to ./user/syscall_wrap.S , what is the value of v0 ?
    lw v0, TF_REG2(sp)
    subu    v0, v0, __SYSCALL_BASE
    sltiu   t0, v0, __NR_SYSCALLS+1

    //set epc as epc plus 4
    //Hint:Where is the next being exectued cpu instruction when return from exception?
    lw      t1,TF_EPC(sp)
    addiu   t1, 4
    sw	    t1, TF_EPC(sp)

    //if legal , goto the right function's address
    //Hint:Why sll?
    beqz	t0, illegal_syscall
    nop
    sll	    t0, v0,2
    la	    t1, sys_call_table
    addu	t1, t0
    lw	    t2, (t1)
    beqz	t2, illegal_syscall
    nop
    lw	    t0,TF_REG29(sp)
    
    //get the function parameters 
    lw	t1, (t0)
    lw	t3, 4(t0)
    lw	t4, 8(t0)
    lw	t5, 12(t0)
    lw	t6, 16(t0)
    lw	t7, 20(t0)

    subu	sp, 20

    //push the parameters into stack
    //Hint:think about syscall_wrap.S , can we remove some lines?
    sw	t1, 0(sp)
    sw	t3, 4(sp)
    sw	t4, 8(sp)
    sw	t5, 12(sp)
    sw	t6, 16(sp)
    sw	t7, 20(sp)

    //go to exectue the syscall
    jalr	t2
    nop
    
    //syscall finished , restore sp and v0
    addu	sp, 20
    sw	v0, TF_REG2(sp)

    //return to user mode
    //ret_from_exception is a function which is defined in ./lib/genex.S
    j	ret_from_exception
    nop

    //error and loop
illegal_syscall:

    j illegal_syscall 
    nop

END(handle_sys)


    
    .extern sys_putchar
    .extern sys_getenvid
    .extern sys_yield
    .extern sys_env_destroy
    .extern sys_set_pgfault_handler
    .extern sys_mem_alloc
    .extern sys_mem_map
    .extern sys_mem_unmap
    .extern sys_env_alloc
    .extern sys_set_env_status
    .extern sys_set_trapframe
    .extern sys_panic
    .extern sys_ipc_can_send
    .extern sys_ipc_recv
    .extern sys_cgetc

    //Overview:
    //syscalltable stores all the syscall function's entrypoints

.macro syscalltable
   
    .word sys_putchar
    .word sys_getenvid
    .word sys_yield
    .word sys_env_destroy
    .word sys_set_pgfault_handler
    .word sys_mem_alloc
    .word sys_mem_map
    .word sys_mem_unmap
    .word sys_env_alloc
    .word sys_set_env_status
    .word sys_set_trapframe
    .word sys_panic
    .word sys_ipc_can_send
    .word sys_ipc_recv
    .word sys_cgetc
    
.endm
EXPORT(sys_call_table)

    syscalltable
.size sys_call_table, . - sys_call_table 
