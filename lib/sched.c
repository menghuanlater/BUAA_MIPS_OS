#include <env.h>
#include <pmap.h>
#include <printf.h>

/* Overview:
 *  Implement simple round-robin scheduling.
 *  Search through 'envs' for a runnable environment ,
 *  in circular fashion starting after the previously running env,
 *  and switch to the first such environment found.
 *
 * Hints:
 *  The variable which is for counting should be defined as 'static'.
 */
//when occur next exception break,carry out this function again. 
void sched_yield(void)
{
	static int pos = -1;
	while(1){
		pos = (pos+1)%NENV;
		/*if(pos==0){
			printf("is:%d\n",envs[pos].env_status);
		}*/
		if(envs[pos].env_status==ENV_RUNNABLE){
			env_run(&envs[pos]);
		}
	}

}
/*void sched_yield(void){
	static int i = -1;
	int n=0;
	for(;n<NENV-1;n++){
		i++;
		if(i==NENV)
			i=0;
		if(envs[i].env_status == ENV_RUNNABLE)
			env_run(&envs[i]);
	}
}*/


