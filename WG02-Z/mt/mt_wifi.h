#ifndef ____HAL_WIFI_H_
#define ____HAL_WIFI_H_


#define ESP12_AT_LEN 70

#define WIFI_TX_QUEUE_SUM			    10		//发送网关命令队列组个数	
#define WIFI_TX_BUFFSIZE_MAX			200     //2 + 198


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
	
	ESP12_AT_MQTTUSERCFG,// "AT+MQTTUSERCFG=0,1,\"", 
	ESP12_AT_MQTTCONN,// "AT+MQTTCONN=0,\"",   
	ESP12_AT_MQTTPUB,// "AT+MQTTPUB=0,\"",    
	ESP12_AT_MQTTSUB,// "AT+MQTTSUB=0,\"",     
	ESP12_AT_MQTTCLEAN,// "AT+MQTTCLEAN=0",    
	ESP12_AT_MAX,
};






void mt_wifi_init(void);
void mt_wifi_pro(void);

#endif
