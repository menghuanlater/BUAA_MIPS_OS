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
	static int p_flag = 0;
	while(1){
		if(p_flag){
			p_flag = 0;
			env_run(&envs[position]);
			break;
		}
		position++;//we must carry this at head,because put it back will cause all env use same position
		if(position>=2){ //when it over limit range,then we reset it to the array head.
			position = 0;
		}
		if(envs[position].env_status == ENV_RUNNABLE){
			if(position==1){
				p_flag = 1;
			}
			env_run(&envs[position]);
		}
	}
}
