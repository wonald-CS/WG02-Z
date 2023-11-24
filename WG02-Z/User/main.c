#include "hal_task.h"
#include "mt_task.h"
#include "app.h"
#include "para.h"

#include "OS_System.h"
#include "CPU.h"

//2023.11.02 因为自己写的工程在移植eeprom的时候出问题：应该是前面定时器的配置出了问题导致死机（无法排查），所以直接用无际的课程
int main(void)
{
	hal_CPUInit();
	OS_TaskInit();
	ParaInit();
	
	hal_task_init();	
	OS_CreatTask(OS_TASK1,hal_task,1,OS_RUN);	
	
	mt_task_init();	
	OS_CreatTask(OS_TASK2,mt_task,1,OS_RUN);	
	
	app_task_init();	
	OS_CreatTask(OS_TASK3,app_task,1,OS_RUN);	
	
	OS_Start();	 
}
