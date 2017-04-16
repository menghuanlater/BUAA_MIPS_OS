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
	int i;
	for(i=0;i<NENV;i++){
		position++;//we must carry this at head,because put it back will cause all env use same position
		if(position==NENV){ //when it over limit range,then we reset it to the array head.
			position = 0;
		}
		//if(envs[position].env_id == 4097){
		//	envs[position].env_status = ENV_RUNNABLE;
		//}
		if(envs[position].env_status == ENV_RUNNABLE){
			env_run(&envs[position]);
			return;
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


