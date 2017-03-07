#include <asm/regdef.h>
#include <asm/cp0regdef.h>
#include <asm/asm.h>


			.section .data.stk
KERNEL_STACK:
			.space 0x8000


			.text
LEAF(_start)           /*鍦╝sm.h涓畾涔夌殑鍙跺瓙鍑芥暟锛屽彾瀛愬嚱鏁颁笉璋冪敤鍏跺畠鍑芥暟*/              
	
	.set	mips2      /*.set鐢ㄤ簬鎸囧姹囩紪鍣ㄥ浣曞伐浣滐紝鎺у埗鎸囦护鎵ц椤哄簭*/
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
  set up stack 
  you can reference the memory layout in the include/mmu.h
*/

			
loop:
	j	loop
	nop
END(_start)            /*在asm.h中定义的函数*/
