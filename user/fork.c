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
	u_int temp = 0x50000000;
	//first we must make sure that va is align to BY2PG
	va = ROUNDDOWN(va,BY2PG);
	u_int perm = (*vpt)[VPN(va)]& 0xfff;
	//writef("fork.c:pgfault():\t va:%x\n",va);
	if(perm & PTE_COW){
		if(syscall_mem_alloc(0,temp,perm &(~PTE_COW))<0){
			user_panic("syscall_mem_alloc error.\n");
		}
		user_bcopy((void *)va,(void *)temp,BY2PG);
		if(syscall_mem_map(0,temp,0,va,perm &(~PTE_COW))<0){
			user_panic("syscall_mem_map error.\n");
		}
		if(syscall_mem_unmap(0,temp)<0){
			user_panic("syscall_mem_unmap error.\n");
		}
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
	u_int perm;
	perm = (*vpt)[pn] & 0xfff; //取出标记位
	if(((perm & PTE_R) !=0) || ((perm & PTE_COW)!=0)){
		/*if(perm & PTE_LIBRARY){
			perm = PTE_V | PTE_R | PTE_COW | PTE_LIBRARY;
		}else{
			perm = PTE_V | PTE_R;
		}*/
		perm = perm | PTE_V | PTE_R | PTE_COW;
	}
	if(syscall_mem_map(0,pn*BY2PG,envid,pn*BY2PG,perm)<0){
		user_panic("failed page map in duppage.\n");
	}

	//user_panic("duppage not implemented");
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
	if((newenvid = syscall_env_alloc())==0){
		//in child env
		env = &envs[ENVX(syscall_getenvid())];
		return 0;
	}
	/*use vpt vpd，我们只需要将父进程中相关的用户空间的页复制到子进程用户空间即可*/
	/*注意创建一个进程的时候会调用env_vm_init函数，这个函数有个非常关键的操作,我们创建
	子进程，复制父进程的地址空间只需要复制UTOP以下的页即可，因为所有进程UTOP以上的页都是利用
	boot_pgdir作为模板复制的，不需要再次复制拷贝*/
	/*we need judge whether the pgtable is exist or the page is exist.*/
	for(i=0;i<UTOP-BY2PG;i+=BY2PG){
		if(((*vpd)[VPN(i)/1024])!=0 && ((*vpt)[VPN(i)])!=0){
			duppage(newenvid,VPN(i));
		}
	}
	//搭建异常处理栈，分配一个页，让别的进程不抢占此页
	if(syscall_mem_alloc(newenvid,UXSTACKTOP-BY2PG,PTE_V|PTE_R)<0){
		user_panic("failed alloc UXSTACK.\n");
		return 0;
	}
	//帮助子进程注册错误处理函数
	if(syscall_set_pgfault_handler(newenvid,__asm_pgfault_handler,UXSTACKTOP)<0){
		user_panic("page fault handler setup failed.\n");
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
