#include "stm32f10x.h"
#include "app.h" 
#include "hal_led.h"
#include "mt_tftlcd.h"
#include "mt_flash.h"
#include "OS_System.h"
#include "CPU.h"
#include "hal_gotoapp.h"


int main(void)
{
	NVIC_SetVectorTable(0x8000000, 0x0000);
	
	mt_tftlcd_init();
    mt_flashInit();
	hal_LedInit();
	
	OS_TaskInit();

	hal_CPUInit();
	OS_CreatTask(OS_TASK1,hal_Led1Turn,1,OS_RUN);

	//GotoApp();
 	AppInit();	
 	OS_CreatTask(OS_TASK2,AppProc,1,OS_RUN);	

	OS_Start();	 
}
