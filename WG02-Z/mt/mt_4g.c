#include "mt_4g.h"
#include "hal_power.h"

static unsigned char EC200N_PowerTime;



//EC200N开机函数
void EC200N_Power_On(void)
{
    if (EC200N_PowerTime < 100)
    {
        EC200N_PowerTime++;
        //先拉高至少30ms
        if (EC200N_PowerTime == 5)
        {
            hal_4G_Power_HIGH();            //再拉低至少500MS后可以拉高或者不操作，完成开机
        }else if (EC200N_PowerTime == 85)   //800MS
        {
            hal_4G_Power_LOW();
        }
        
    }
    
}



void mt_4g_Init(void)
{
	EC200N_PowerTime = 0;
    hal_4G_Power_LOW();        //IO口低电平，EC200的POWEkey为高电平
}


void mt_4g_Config(void)
{
    EC200N_Power_On();
}

