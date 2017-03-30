#include <asm/regdef.h>
#include <asm/cp0regdef.h>
#include <asm/asm.h>

/* Overview:
 * 	`msyscall` push all the arguments into the stack, running
 * `syscall` instruction.
 * 
 * Pre-Condition:
 * 	The first, second, third and fourth arguments are passed
 * by registers(a0~a3). The remains are stored on the stack.
 *
 * Post-Condition:
 *	All arguments should be stored on the stack. Syscall number
 * should be passed by register v0.
 *
 * Hint:
 *	Interestingly, MIPS 32 ABI(application binary interface) defined that
 * allocating space, which shoud be large enough to contain all the arguments, 
 * on the stack is always required.
 *	So, we needn't allocate space on the stack again. In another word,
 * we shouldn't change the value of $sp. All we need to do is store 
 * registers(a0~a3) on the stack. 
 *  Remember passing syscall number by register v0 :)
 */
LEAF(msyscall)

sw	a0,0(sp)
sw	a1,4(sp)
sw	a2,8(sp)
sw	a3,12(sp)
move	v0, a0

syscall

jr	ra

END(msyscall)

