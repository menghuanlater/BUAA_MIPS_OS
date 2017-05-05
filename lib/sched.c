#include <env.h>
#include <pmap.h>
#include <printf.h>
#define LEN 7 //测试一共7个进程
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
/*void sched_yield(void)
{
	static long position = -1;
	int i;
	for(i=0;i<NENV;i++){
		position++;//we must carry this at head,because put it back will cause all env use same position
		if(position>=7){ //when it over limit range,then we reset it to the array head.
			position = 0;
		}
		if(envs[position].env_status == ENV_RUNNABLE){
			env_run(&envs[position]);
			return;
		}
	}
}*/
//基于优先级时间片轮转算法
void sched_yield(void){
	static int myEnv[LEN] = {1,1,1,1,1,1,1};
	int i;
	static int count = 1;
	for(i=0;i<LEN;i++){
		myEnv[i] = ((i+count)*4399)%66;
	}
	int max = -1; int pos = -1;
	printf("\n");
	for(i=0;i<LEN;i++){
		if(envs[i].env_status != ENV_RUNNABLE){
			continue;
		}
		if(myEnv[i]>max){
			max = myEnv[i];
			pos = i;
		}
		printf("%c:%d***\t",'A'+i,myEnv[i]);
	}
	count++;
	if(max==-1)
		return;
	printf("\nchoose:%c\n",pos+'A');
	env_run(&envs[pos]);	
}
