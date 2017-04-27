#include "lib.h"
#include <mmu.h>
#include <env.h>

#define TMPPAGE		(BY2PG)
#define TMPPAGETOP	(TMPPAGE+BY2PG)

int
init_stack(u_int child, char **argv, u_int *init_esp)
{
	int argc, i, r, tot;
	char *strings;
	u_int *args;

	// Count the number of arguments (argc)
	// and the total amount of space needed for strings (tot)
	tot = 0;
	for (argc=0; argv[argc]; argc++)
		tot += strlen(argv[argc])+1;

	// Make sure everything will fit in the initial stack page
	if (ROUND(tot, 4)+4*(argc+3) > BY2PG)
		return -E_NO_MEM;

	// Determine where to place the strings and the args array
	strings = (char*)TMPPAGETOP - tot;
	args = (u_int*)(TMPPAGETOP - ROUND(tot, 4) - 4*(argc+1));

	if ((r = syscall_mem_alloc(0, TMPPAGE, PTE_V|PTE_R)) < 0)
		return r;
	// Replace this with your code to:
	//
	//	- copy the argument strings into the stack page at 'strings'
//printf("come 1\n");
	char *ctemp,*argv_temp;
	u_int j;
	ctemp = strings;
//printf("tot=%d\n",tot);
	for(i = 0;i < argc; i++)
	{
		argv_temp = argv[i];
		for(j=0;j < strlen(argv[i]);j++)
		{
			*ctemp = *argv_temp;
			ctemp++;
			argv_temp++;
		}
		*ctemp = 0;
		ctemp++;
	}
	//	- initialize args[0..argc-1] to be pointers to these strings
	//	  that will be valid addresses for the child environment
	//	  (for whom this page will be at USTACKTOP-BY2PG!).
//printf("come 2\n");
	ctemp = (char *)(USTACKTOP - TMPPAGETOP + (u_int)strings);
	for(i = 0;i < argc;i++)
	{
		args[i] = (u_int)ctemp;
//writef("args[i]=%x\n",args[i]);
		ctemp += strlen(argv[i])+1;
	}
	//	- set args[argc] to 0 to null-terminate the args array.
//printf("come 3\n");
	ctemp--;
	args[argc] = ctemp;
//writef("args[argc]=%x\n",args[argc]);
	//	- push two more words onto the child's stack below 'args',
	//	  containing the argc and argv parameters to be passed
	//	  to the child's umain() function.
	u_int *pargv_ptr;
//printf("come 4	args=%x\n",(u_int)args);
	pargv_ptr = args - 1;
	*pargv_ptr = USTACKTOP - TMPPAGETOP + (u_int)args;
//writef("*pargv_ptr=%x\n",*pargv_ptr);
	pargv_ptr--;
	*pargv_ptr = argc;
	//
	//	- set *init_esp to the initial stack pointer for the child
	//
//printf("come 5\n");
//writef("TMPPAGETOP - pargv_ptr =%x,	pargv_ptr=%x\n",TMPPAGETOP - (u_int)pargv_ptr,pargv_ptr);
	*init_esp = USTACKTOP - TMPPAGETOP + (u_int)pargv_ptr;
//	*init_esp = USTACKTOP;	// Change this!

	if ((r = syscall_mem_map(0, TMPPAGE, child, USTACKTOP-BY2PG, PTE_V|PTE_R)) < 0)
		goto error;
	if ((r = syscall_mem_unmap(0, TMPPAGE)) < 0)
		goto error;

	return 0;

error:
	syscall_mem_unmap(0, TMPPAGE);
	return r;
}


int spawn(char *prog, char **argv)
{
		int fd;
		int r;
		int size;
		u_int esp;

		if((fd = open(prog, O_RDWR/*O_ACCMODE*/))<0)
			user_panic("spawn:open %s:%e",prog,fd);

		u_int child_envid;
		child_envid = syscall_env_alloc();
		if(child_envid < 0)
		{
			writef("spawn:alloc the new env is wrong\n");
			return child_envid;
		}

	init_stack(child_envid, argv,&esp);

	size = ((struct Filefd*)num2fd(fd))->f_file.f_size;

		
		u_int i;
		u_int *blk;
		int text_start;
		text_start = 0;
		for(i = 0x1000; i < size; i += BY2PG)
		{
			if((r = read_map(fd, i, &blk))<0)
			{
				writef("mapping text region is wrong\n");
				return r;
			}

			syscall_mem_map(0, blk, child_envid, UTEXT+text_start, PTE_V |PTE_R);		
			text_start += BY2PG;
		}

	struct Trapframe *tf;
		writef("\n::::::::::spawn size : %x  sp : %x::::::::\n",size,esp);
		tf = &(envs[ENVX(child_envid)].env_tf);
		tf->pc = UTEXT;
		tf->regs[29]=esp;


		u_int pdeno = 0;
		u_int pteno = 0;
		u_int pn = 0;
		u_int va = 0;
//writef("spawn begin to share \n");
		for(pdeno = 0;pdeno<PDX(UTOP);pdeno++)
		{
			if(!((* vpd)[pdeno]&PTE_V))
				continue;
			for(pteno = 0;pteno<=PTX(~0);pteno++)
			{
				pn = (pdeno<<10)+pteno;
				if(((* vpt)[pn]&PTE_V)&&((* vpt)[pn]&PTE_LIBRARY))
				{
					va = pn*BY2PG;

					if((r = syscall_mem_map(0,va,child_envid,va,(PTE_V|PTE_R|PTE_LIBRARY)))<0)
					{

						writef("va: %x   child_envid: %x   \n",va,child_envid);
						user_panic("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
						return r;
					}
				}
			}
		}
	
	
		if((r = syscall_set_env_status(child_envid, ENV_RUNNABLE)) < 0)
		{
			writef("set child runnable is wrong\n");
			return r;
		}
//writef("spawn:end of spawn\n");		
		return child_envid;		

}

int
spawnl(char *prog, char *args, ...)
{
	return spawn(prog, &args);
}
