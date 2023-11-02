#include "mt_4g.h"
#include "mt_api.h"
#include "hal_GPIO.H"
#include "OS_System.h"
#include "hal_uart.h"
#include "string.h"

EC200C_variable  ES200C_Var;

static void EC200S_PutOnHandler(void);

void mt_4g_Init(void)
{
    ES200C_Var.powerKeytime = 0;
    hal_GPIO_4GPowerKey_L(); 
}
void mt_4g_pro(void)
{
    EC200S_PutOnHandler();
}

/****************************************************
功能:4G模块开机任务函数
********************************************************/
static void EC200S_PutOnHandler(void)
{
	if(ES200C_Var.powerKeytime < 100)
	{
		ES200C_Var.powerKeytime ++;
		if(ES200C_Var.powerKeytime == 5)
			hal_GPIO_4GPowerKey_H();
		else  if(ES200C_Var.powerKeytime == 80) //75*10 =750MS
		{
			hal_GPIO_4GPowerKey_L(); 
		}
	}
}













