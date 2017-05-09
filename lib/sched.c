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
	static long position = -1;
	static int a_count = 0;
	while(1){
		position++;
		if(position>=2)
			position = 0;
		if(position==0 && envs[position].env_status == ENV_RUNNABLE){
			if(a_count>=10)
				continue;
			else{
				a_count++;
				env_run(&envs[position]);
			}
		}else if(position==1 && envs[position].env_status == ENV_RUNNABLE)
			env_run(&envs[position]);
	}
}
