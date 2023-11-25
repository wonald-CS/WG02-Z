#include "mt_mqtt.h"
#include "mt_MD5.h"
#include "para.h"


str_mqtt_parameter  mqtt_para;



//客户端ID
void mt_mqtt_setLinkID(unsigned char *p)
{
	unsigned char idx = 0;
	while(*p)
	{
		mqtt_para.linkID[idx++] = *p;
		p ++;
		if(idx == MQTT_LINKID_SIZE_MAX)
		{
			break;
		}
	}
}

//用户名
void mt_mqtt_setUserName(unsigned char *p)
{
	unsigned char idx = 0;
	while(*p)
	{
		mqtt_para.username[idx++] = *p;
		p ++;
		if(idx == MQTT_USERNAME_SIZE_MAX)
		{
			break;
		}
	}
}

//服务器IP
void mt_mqtt_setServerIp(unsigned char *p)
{
	unsigned char idx = 0;
	while(*p)
	{
		mqtt_para.serverIp[idx++] = *p;
		p ++;
		if(idx == MQTT_SERVERIP_SIZE_MAX)
		{
			break;
		}
	}
}

//服务器端口
void mt_mqtt_setServerPort(unsigned char *p)
{
	unsigned char idx = 0;
	while(*p)
	{
		mqtt_para.serverPort[idx++] = *p;
		p ++;
		if(idx == MQTT_SERVERPORT_SIZE_MAX)
		{
			break;
		}
	}
}

//订阅主题
void mt_mqtt_setSubTopic(unsigned char *p)
{
	unsigned char idx = 0;
	while(*p)
	{
		mqtt_para.subtopic[idx++] = *p;
		p ++;
		if(idx == MQTT_TPIC_SIZE_MAX)
		{
			break;
		}
	}
}

//发布主题
void mt_mqtt_setPubTopic(unsigned char *p)
{
	unsigned char idx = 0;
	while(*p)
	{
		mqtt_para.pubtopic[idx++] = *p;
		p ++;
		if(idx == MQTT_TPIC_SIZE_MAX)
		{
			break;
		}
	}
}

//报警主机是否有新固件
void mt_mqtt_SetNewFlag(en_mqtt_recNewFlag flag)
{
	mqtt_para.Newflag = flag;
}


unsigned char mt_mqtt_GetNewFlag(void)
{
	return mqtt_para.Newflag;
}


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
	mt_mqtt_setLinkID(&buf[0]);
	mt_mqtt_setUserName(&buf[1]);
	mt_md5_EncipherPassWord((char *)&buf[1],(char *)&buf[1],mqtt_para.password);
	mt_mqtt_setServerIp("119.91.158.8");
	mt_mqtt_setServerPort("1883");
	
	buf[25] = '_';
	buf[26] = 'd';
	buf[27] = 'o';
	buf[28] = 'w';	
	buf[29] = 'n';	
	buf[30] = 0;	
	mt_mqtt_setSubTopic(&buf[1]);

	buf[25] = '_';
	buf[26] = 'u';
	buf[27] = 'p';
	buf[28] = 0;	
	buf[29] = 0;	
	buf[30] = 0;	
	mt_mqtt_setPubTopic(&buf[1]);
	mt_mqtt_SetNewFlag(MQTT_REC_MESSAGE_FREE);	


}
