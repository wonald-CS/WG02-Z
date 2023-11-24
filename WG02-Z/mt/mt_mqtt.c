#include "mt_mqtt.h"
#include "mt_MD5.h"
#include "para.h"


str_mqtt_parameter  mqtt_para;
//str_mqtt_Upevent mqtt_upEvent;


//MQTT 参数初始化
void mt_mqtt_init(void)
{
	unsigned char i,buf[32],dat;	
	buf[0] = '0';
	for(i=0;i<12;i++)
    {
		dat = MT_GET_MCU_UID(i);
		dat >>= 4;
		dat &= 0x0f;
		if(dat < 10)
		{
			buf[2*i + 1] = dat + '0';
		}
		else
		{
			buf[2*i + 1] = dat + 'A' -10;
		}	
		dat = MT_GET_MCU_UID(i);
		dat &= 0x0f;
		if(dat < 10)
		{
			buf[2*i + 2] = dat + '0';
		}
		else
		{
			buf[2*i + 2] = dat + 'A' -10;
		}
	}
	buf[25] = 0;
	mt_md5_EncipherPassWord((char *)&buf[1],(char *)&buf[1],mqtt_para.password);
}
