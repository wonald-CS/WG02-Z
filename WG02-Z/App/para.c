#include "para.h"

//volatile声明的变量表示该变量随时可能发生变化
volatile stu_system_time stuSystemtime;	
unsigned char STM32_UID[12];
	

void ParaInit(void)
{
    unsigned char i;
	for(i=0; i<12; i++)
	{
		STM32_UID[i] = *((unsigned char *)(STM32_UID_ADDR+i));
	}
}


/*******************************************************************************************
*@description:本地时间
*@param[in]：
*@return：无
*@others：
********************************************************************************************/
void SystemTime_local(void)
{
	static unsigned int systemDelaytimer = 0;
	systemDelaytimer ++;
	if(systemDelaytimer > 20000)
	{
		systemDelaytimer = 0;
		stuSystemtime.sec ++;
		if(stuSystemtime.sec  == 60)
		{
			stuSystemtime.sec = 0;
			stuSystemtime.min++;
			if(stuSystemtime.min  == 60)
			{
				stuSystemtime.min = 0;
				stuSystemtime.hour ++;
				if(stuSystemtime.hour == 24)
				{
					stuSystemtime.hour = 0;
					stuSystemtime.day ++;
					stuSystemtime.week++;
					if(stuSystemtime.week ==7)
					{
						stuSystemtime.week = 0;
					}
					if(stuSystemtime.day == 30)
					{		
						stuSystemtime.day = 0;
						stuSystemtime.mon ++;
						if(stuSystemtime.mon == 13)
						{
							stuSystemtime.mon = 1;
							stuSystemtime.year++;
						}
					}						
				}
			}
		}	
	}
}
