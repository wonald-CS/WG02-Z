#include "stm32f10x.h"
#include "OS_System.h"
#include "CPU.h"
#include "hal_task.h"
#include "mt_task.h"
#include "app.h"
#include "hal_wtn6.h"


int main(void)
{
  hal_CPUInit();
  OS_TaskInit();

  hal_task_Init();
	OS_CreatTask(OS_TASK1,hal_task,1,OS_RUN);	   //10ms

  mt_task_Init();
  OS_CreatTask(OS_TASK2,mt_task,1,OS_RUN);	

  app_task_Init();
  OS_CreatTask(OS_TASK3,app_task,1,OS_RUN);	

	OS_Start();	

}
