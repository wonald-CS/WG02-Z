#include "hal_task.h"
#include "mt_task.h"
#include "app.h"

#include "OS_System.h"
#include "CPU.h"

int main(void)
{
	hal_CPUInit();
	OS_TaskInit();
	
	hal_task_init();	
	OS_CreatTask(OS_TASK1,hal_task,1,OS_RUN);	
	
	mt_task_init();	
	OS_CreatTask(OS_TASK2,mt_task,1,OS_RUN);	
	
	app_task_init();	
	OS_CreatTask(OS_TASK3,app_task,1,OS_RUN);	
	
	OS_Start();	 
}
