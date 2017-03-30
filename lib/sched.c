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
 
void sched_yield(void)
{
	static long position = 0;
	for(;position<NENV;position++){
		if(envs[position].env_status == ENV_RUNNABLE){
			env_run(&envs[position]);
			if(position==NENV-1)
				position = 0;
			else
				position++;
			return;
		}
	}
}
/*void sched_yield(void){
	static long i = -1;
	int n = 0;
	for(;n<NENV;n++){
		i++;
		if(i==NENV)
			i=0;
		if(envs[i].env_status == ENV_RUNNABLE)
			env_run(&envs[i]);
	}
}*/
