// implement fork from user space

#include "lib.h"
#include <mmu.h>
#include <env.h>

//because now we are in user status,so we need use user/syscall_lib.c but not lib/syscall_all.c
//be careful.
/* ----------------- help functions ---------------- */

/* Overview:
 * 	Copy `len` bytes from `src` to `dst`.
 *
 * Pre-Condition:
 * 	`src` and `dst` can't be NULL. Also, the `src` area 
 * 	 shouldn't overlap the `dest`, otherwise the behavior of this 
 * 	 function is undefined.
 */
void user_bcopy(const void *src, void *dst, size_t len)
{
	void *max;

	//	writef("~~~~~~~~~~~~~~~~ src:%x dst:%x len:%x\n",(int)src,(int)dst,len);
	max = dst + len;

	// copy machine words while possible
	if (((int)src % 4 == 0) && ((int)dst % 4 == 0)) {
		while (dst + 3 < max) {
			*(int *)dst = *(int *)src;
			dst += 4;
			src += 4;
		}
	}

	// finish remaining 0-3 bytes
	while (dst < max) {
		*(char *)dst = *(char *)src;
		dst += 1;
		src += 1;
	}

	//for(;;);
}

/* Overview:
 * 	Sets the first n bytes of the block of memory 
 * pointed by `v` to zero.
 * 
 * Pre-Condition:
 * 	`v` must be valid.
 *
 * Post-Condition:
 * 	the content of the space(from `v` to `v`+ n) 
 * will be set to zero.
 */
void user_bzero(void *v, u_int n)
{
	char *p;
	int m;

	p = v;
	m = n;

	while (--m >= 0) {
		*p++ = 0;
	}
}
/*--------------------------------------------------------------*/

/* Overview:
 * 	Custom page fault handler - if faulting page is copy-on-write,
 * map in our own private writable copy.
 * 
 * Pre-Condition:
 * 	`va` is the address which leads to a TLBS exception.
 *
 * Post-Condition:
 *  Launch a user_panic if `va` is not a copy-on-write page.
 * Otherwise, this handler should map a private writable copy of 
 * the faulting page at correct address.
 */
static void
pgfault(u_int va)
{
	//first we must make sure that va is align to BY2PG
	int align_va = ROUNDDOWN(va,BY2PG);
	//	writef("fork.c:pgfault():\t va:%x\n",va);
	int curenv_id = syscall_getenvid();
   	if(PADDR(va) & PTE_COW !=0){
		user_bcopy((void *)va,(void *)UXSTACKTOP-BY2PG,BY2PG);
		syscall_mem_map(curenv_id,UXSTACKTOP-BY2PG,curenv_id,va,PTE_V|PTE_R);
		syscall_mem_unmap(curenv_id,UXSTACKTOP-BY2PG);
	}else{
		user_panic("va page is not PTE_COW.\n");
	}	
}

/* Overview:
 * 	Map our virtual page `pn` (address pn*BY2PG) into the target `envid`
 * at the same virtual address. 
 *
 * Post-Condition:
 *  if the page is writable or copy-on-write, the new mapping must be 
 * created copy on write and then our mapping must be marked 
 * copy on write as well. In another word, both of the new mapping and
 * our mapping should be copy-on-write if the page is writable or 
 * copy-on-write.
 * 
 * Hint:
 * 	PTE_LIBRARY indicates that the page is shared between processes.
 * A page with PTE_LIBRARY may have PTE_R at the same time. You
 * should process it correctly.
 */
static void
duppage(u_int envid, u_int pn)
{
	/* Note:
	 *  I am afraid I have some bad news for you. There is a ridiculous, 
	 * annoying and awful bug here. I could find another more adjectives 
	 * to qualify it, but you have to reproduce it to understand 
	 * how disturbing it is.
	 * 	To reproduce this bug, you should follow the steps bellow:
	 * 	1. uncomment the statement "writef("");" bellow.
	 * 	2. make clean && make
	 * 	3. lauch Gxemul and check the result.
	 * 	4. you can add serveral `writef("");` and repeat step2~3.
	 * 	Then, you will find that additional `writef("");` may lead to
	 * a kernel panic. Interestingly, some students, who faced a strange 
	 * kernel panic problem, found that adding a `writef("");` could solve
	 * the problem. 
	 *  Unfortunately, we cannot find the code which leads to this bug,
	 * although we have debugged it for serveral weeks. If you face this
	 * bug, we would like to say "Good luck. God bless."
	 */
	// writef("");
	u_int addr;
	u_int perm;

	//	user_panic("duppage not implemented");
}

/* Overview:
 * 	User-level fork. Create a child and then copy our address space
 * and page fault handler setup to the child.
 *
 * Hint: use vpd, vpt, and duppage.
 * Hint: remember to fix "env" in the child process!
 * Note: `set_pgfault_handler`(user/pgfault.c) is different from 
 *       `syscall_set_pgfault_handler`. 
 */
extern void __asm_pgfault_handler(void);
int
fork(void)
{
	// Your code here.
	u_int newenvid;
	extern struct Env *envs;
	extern struct Env *env;//将其指向当前的进程，如果子进程无法创建，则指向父进程
	u_int i;

	//The parent installs pgfault using set_pgfault_handler
	set_pgfault_handler(pgfault);	
	//alloc a new env
	if((newenvid = syscall_env_alloc())<0){
		panic("Sorry,in fork we found newenv not alloc.\n");
		env = &ens[ENVX(syscall_getenvid())]
		return 0;
	}
	
	env = &envs[ENVX(newenvid)];
	
	/*use vpt vpd，我们只需要将父进程中相关的用户空间的页复制到子进程用户空间即可*/
	

	//搭建异常处理栈，分配一个页，让别的进程不抢占此页
	if(syscall_mem_alloc(newenvid,UXSTACKTOP-BY2PG,PTE_V|PTE_R)<0){
		panic("failed alloc UXSTACK.\n");
		return 0;
	}
	//帮助子进程注册错误处理函数
	if(syscall_set_pgfault_handler(0,newenvid,__asm_pgfault_handler,UXSTACKTOP)<0){
		panic("page fault handler setup failed.\n");
		return 0;
	}
	//we need to set the child env status to ENV_RUNNABLE,we must use syscall_set_env_status.
	syscall_set_env_status(newenvid,ENV_RUNNABLE);
	return newenvid;
}

// Challenge!
int
sfork(void)
{
	user_panic("sfork not implemented");
	return -E_INVAL;
}
