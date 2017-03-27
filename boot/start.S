#include <asm/regdef.h>
#include <asm/cp0regdef.h>
#include <asm/asm.h>


			.section .data.stk
KERNEL_STACK:
			.space 0x8000


			.text
LEAF(_start)           /*在asm.h中定义的叶子函数，叶子函数不调用其它函数*/              
	
	.set	mips2      /*.set用于指导汇编器如何工作，控制指令执行顺序*/
	.set	reorder

	/* Disable interrupts */
	mtc0	zero, CP0_STATUS

        /* Disable watch exception. */
        mtc0    zero, CP0_WATCHLO
        mtc0    zero, CP0_WATCHHI

	/* disable kernel mode cache */
	mfc0	t0, CP0_CONFIG
	and	t0, ~0x7
	ori	t0, 0x2
	mtc0	t0, CP0_CONFIG
	
/*To do:
  set up stack    --->OK 
  you can reference the memory layout in the include/mmu.h
*/
lui sp,0x8040
ori sp,sp,0x0000
jal main
nop
			
loop:
	j	loop
	nop
END(_start)            /*??asm.h?ж????ĺ???*/
