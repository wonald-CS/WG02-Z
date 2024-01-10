#ifndef ____HAL_WIFI_H_
#define ____HAL_WIFI_H_

#define WIFI_TX_QUEUE_SUM			    10		//发送网关命令队列组个数	
#define WIFI_TX_BUFFSIZE_MAX			200     //2 + 198

#define WIFI_RXBUFFSIZE_MAX		        800
#define WIFI_SSIDLEN_MAX                20

#define WIFI_MQTT_SENTDATASIZE_MAX      (WIFI_TX_BUFFSIZE_MAX - 2)

enum
{
	ESP12_AT_RESET =0,
	ESP12_AT_AT,
	ESP12_AT_ATE,	
    ESP12_AT_GETWIFILIST,	
	ESP12_AT_CWMODE,	
	ESP12_AT_CWAUTOCONN,		
	ESP12_AT_CWSTARTSMART,	
	ESP12_AT_CWSTOPSMART,	
	ESP12_AT_CWSTATE,
	ESP12_AT_CWLAP,	
	
	ESP12_AT_MQTTUSERCFG,// "AT+MQTTUSERCFG=0,1,\"",  ///MQTT  CONF
	ESP12_AT_MQTTCONN,// "AT+MQTTCONN=0,\"",    ///MQTT CONN
	ESP12_AT_MQTTPUB,// "AT+MQTTPUB=0,\"",     ///????
	ESP12_AT_MQTTSUB,// "AT+MQTTSUB=0,\"",     ///????
	ESP12_AT_MQTTCLEAN,// "AT+MQTTCLEAN=0",      ///??MQTT ??	
	ESP12_AT_MAX
};



//mt_wifi.h
typedef enum
{
	ESP12_AT_RESPONSE_WIFI_CONNECTED =0,
	ESP12_AT_RESPONSE_WIFI_DISCONNECT,
	ESP12_AT_RESPONSE_CWSTATE,	
	ESP12_AT_RESPONSE_CWJAP,
	ESP12_AT_RESPONSE_ERROR,
	ESP12_AT_RESPONSE_SMART_GET_WIFIWINFO,	
	ESP12_AT_RESPONSE_SMART_SUC,	
	ESP12_AT_RESPONSE_CWLAP,
	
	ESP12_AT_RESPONSE_MQTTCONN,//	"+MQTTCONNECTED:0\0",
	ESP12_AT_RESPONSE_MQTTDISCONN,//"+MQTTDISCONNECTED:0\0",
	ESP12_AT_RESPONSE_MQTTRECV,//"+MQTTSUBRECV:0,\0",
	ESP12_AT_RESPONSE_OK,
	ESP12_AT_RESPONSE_MAX,
	
}en_esp12_atResponse;

//mt_wifi.h     WIFI工作状态
typedef enum
{
	ESP12_STA_RESET,  ///WIFI  reset
	ESP12_STA_DETEC_WIFIMODULE, ////检测WIFI模块
	ESP12_STA_DETEC_INIT,       ////WIFI模块初始化
	ESP12_STA_DETEC_STA,        ////WIFI模块联网
	ESP12_STA_GET_PASSWORD,     ///获取WIFI密码  进入WIFI配网模式
	ESP12_STA_GETING_PASS,         ///获取WIFI密码  正在进行WIFI配网
	ESP12_STA_GET_SMART_WIFINFO,     ///获取WIFI信息 ，等待联网
	ESP12_STA_GETING_SUC,         ///配网成功
	ESP12_STA_GETING_FAIL,        ///配网失败
	ESP12_STA_DETEC_READY,      ////WIFI模块工作就绪
	ESP12_STA_UP_ALARMDAT,      ///上报报警信息	
}en_Esp12_sta;

typedef enum
{
	ESP12_LINK_FAIL,
	ESP12_LINK_FAIL_0_NOPIV4,
	ESP12_LINK_SUC,
	ESP12_LINK_FAIL_3,	
	ESP12_LINK_FAIL_4,		
}en_Esp12_link;


typedef enum
{
	STA_WIFI_POWER_IDLE,
	STA_WIFI_POWER_RESET,
//		STA_WIFI_POWER_BREAK,
//		STA_WIFI_POWER_LINK,
		STA_WIFI_POWER_MAX,
}en_wifiPowerManageSta;

typedef enum 
{
	STA_MQTT_FREE=0,  ///MQTT 空闲
	STA_MQTT_CONF,    ///MQTT 参数配置
	STA_MQTT_CONN,    ///链接MQTT服务器
	STA_MQTT_SUB,     // 消息订阅
	STA_MQTT_READY,   //MQTT 准备就绪
	STA_MQTT_PUB,     //MQTT 消息发布	
}en_mqtt_sta;



void mt_wifi_init(void);
void mt_wifi_pro(void);

void mt_wifi_changState(en_Esp12_sta sta);
unsigned char mt_wifi_get_wifi_work_state(void);
unsigned char mt_wifi_get_wifi_Mqtt_state(void);
void mt_wifi_Mqtt_SentDat(unsigned char *buf);
unsigned char mt_wifi_LinkState_rssi(void);
void mt_wifi_exit_SmartConfig(void);
#endif
