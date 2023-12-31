#ifndef ____MT_WIFI_H_
#define ____MT_WIFI_H_


#define ESP12_AT_LEN 70

#define WIFI_TX_QUEUE_SUM			    10					//发送网关命令队列组个数	
#define WIFI_TX_BUFFSIZE_MAX			200    				//2 + 198

#define WIFI_RXBUFFSIZE_MAX		    	800

#define Get_Ser_Time						1500				//30S
#define Get_Ser_Time_Power				100


typedef enum 
{
	STEP_MQTT_FREE,
	STEP_MQTT_CONF,											//用户参数配置
	STEP_MQTT_CONN,											//连接服务器
	STEP_MQTT_SUB,											//订阅主题
	STEP_MQTT_PUB,											//通过主题发布信息
}WIFI_mqtt_step;


enum
{
	ESP12_AT_RESET =0,
	ESP12_AT_AT,
	ESP12_AT_ATE,	
	ESP12_AT_CWMODE,	
	ESP12_AT_CWAUTOCONN,		
	ESP12_AT_CWSTARTSMART,	
	ESP12_AT_CWSTOPSMART,	
	ESP12_AT_CWSTATE,
	ESP12_AT_CWLAP,	
	
	ESP12_AT_MQTTUSERCFG,			// "AT+MQTTUSERCFG=0,1,\"", 
	ESP12_AT_MQTTCONN,				// "AT+MQTTCONN=0,\"",   
	ESP12_AT_MQTTPUB,				// "AT+MQTTPUB=0,\"",    
	ESP12_AT_MQTTSUB,				// "AT+MQTTSUB=0,\"",     
	ESP12_AT_MQTTCLEAN,				// "AT+MQTTCLEAN=0",    
	ESP12_AT_MAX,
};


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
	
	ESP12_AT_RESPONSE_MQTTCONN,			//	"+MQTTCONNECTED:0\0",
	ESP12_AT_RESPONSE_MQTTDISCONN,		//"+MQTTDISCONNECTED:0\0",
	ESP12_AT_RESPONSE_MQTTRECV,			//"+MQTTSUBRECV:0,\0",
	ESP12_AT_RESPONSE_OK,
	ESP12_AT_RESPONSE_MAX,
}en_esp12_atResponse;


typedef enum
{
	ESP12_STA_RESET,  				//模块重置         			AT+RST
	ESP12_STA_WIFI_POWER, 			//检测WIFI模块						
	ESP12_STA_INIT,      			//WIFI模块初始化
	ESP12_STA_WIFISTA,        		//WIFI模块联网状态
	ESP12_STA_WIFIConfig,			//WIFI模块进入配网状态      AT+CWSTARTSMART=2
	ESP12_STA_WIFIConfig_Wait,  	//WiFi配网等待ing   	
	ESP12_STA_GETING_SUC,       	//配网成功
	ESP12_STA_GETING_FAIL,       	//配网失败
	ESP12_STA_DETEC_READY,      	//WIFI模块工作就绪
	ESP12_STA_UP_ALARMDAT,     		//上报报警信息	
}en_Esp12_sta;


//WiFi联网状态
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


typedef struct 
{
	unsigned char WIFI_Net_Sta;
	unsigned char WIFI_SSid[ESP12_AT_LEN];			//连接WIFI的名称
}en_WIFI_NetSta;



void mt_wifi_init(void);
void mt_wifi_pro(void);
void mt_wifi_changState(en_Esp12_sta sta);
void mt_wifi_Mqtt_SentDat(unsigned char *buf);
unsigned char mt_wifi_GetState(void);

extern unsigned char WIFI_SERVAL_STATUS;	

#endif
