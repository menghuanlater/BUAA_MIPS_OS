// User-level IPC library routines

#include "lib.h"
#include <mmu.h>
#include <env.h>

extern struct Env *env;
static int count;
static int x;
static int y;
static int z;
static int x1;
// Send val to whom.  This function keeps trying until
// it succeeds.  It should panic() on any error other than
// -E_IPC_NOT_RECV.
//
// Hint: use syscall_yield() to be CPU-friendly.
void
ipc_send(u_int whom, u_int val, u_int srcva, u_int perm)
{
	int r;
	while ((r = syscall_ipc_can_send(whom, val, srcva, perm)) == -E_IPC_NOT_RECV ) {
		syscall_yield();
	}
	if (r == 0) {
		return;
	}
	user_panic("error in ipc_send: %d", r);
}

// Receive a value.  Return the value and store the caller's envid
// in *whom.
//
// Hint: use env to discover the value and who sent it.
u_int
ipc_recv(u_int *whom, u_int dstva, u_int *perm)
{
	//printf("ipc_recv:come 0\n");
	syscall_ipc_recv(dstva);

	if (whom) {
		*whom = env->env_ipc_from;
	}

	if (perm) {
		*perm = env->env_ipc_perm;
	}

	return env->env_ipc_value;
}
void new_ipc_send(u_int whom,u_int val,u_int srcva,u_int perm){
	int r;
	if(count==0)
		x = val;
	if(count==1)
		y = val;
	if(count==2)
		z = val;
	while((r=syscall_ipc_can_send(whom,val,srcva,perm))==-E_IPC_NOT_RECV){
		syscall_yield();
	}
	if(r==0){
		return;
	}
}
u_int new_ipc_recv(u_int *whom,u_int dstva,u_int *perm){
	syscall_ipc_recv(dstva);
	if(whom){
		*whom = env->env_ipc_from;
	}
	if(perm){
		*perm = env->env_ipc_perm;
	}
	if(count==0){
		x1 = env->env_ipc_value;
		count++;
	}else if(count==1){
		if(env->env_ipc_value!=x+1){
			user_panic("handshake failed!");
		}
		count++;
	}else if(count==2){
		if(env->env_ipc_value!=x1){
			user_panic("handshake failed!");
		}
		count = 0;
	}
	return env->env_ipc_value;
}

