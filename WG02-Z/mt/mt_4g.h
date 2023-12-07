#ifndef ____MT_4G_H_
#define ____MT_4G_H_

#define GSM_TX_QUEUE_SUM			    13					//发送网关命令队列组个数	
#define GSM_TX_BUFFSIZE_MAX				256    				//AT指令缓存字节最大数



typedef enum
{
	EC200_AT_ATE_CPIN=0,  						//
	EC200_AT_INIT_ATE0,    						//1 回显关闭
	EC200_AT_INIT_IPR,    						//2 设置波特率为115200	
	EC200_AT_QIDEACT,     						//"AT+QIDEACT=1\0",
	EC200_AT_QIACT,       						//"AT+QIACT=1\0",	
	
	EC200_AT_INIT_CREG,    						//"AT+CREG?\0",
	EC200_AT_INIT_CGREG,  						//"AT+CGREG?\0",
	
	EC200_AT_GSN,      							//"AT+GSN\0",	
  	EC200_AT_CIMI,       						//"AT+CIMI=1\0",		
	EC200_AT_SMSINIT_CSCA,						//"AT+CSCA?\0",									//1 查询短信中心号码
	EC200_AT_SMSINIT_CMGF,						//"AT+CMGF=1\0",								//配置短信设置的方式	
	EC200_AT_SMSINIT_CSCS,						//"AT+CSCS=\"UCS2\"\0",							//配置短信设置的方式	
	EC200_AT_SMSINIT_CCSMP,						//"AT+CSCS=AT+CSMP=49,167,0,25\0",				//配置短信设置的方式	
	EC200_AT_SMSINIT_CNMI,						//"AT+CNMI=2,1,0,1,0\0",		

  	EC200_AT_DIAL_CLVL,     					//"AT+CLVL=5\0",								//33 扬声器等级最低
	EC200_AT_DIAL_CLVL_CLOSE,     				//"AT+CLVL=0\0",
	EC200_AT_MQTT_VERSION,       				//"AT+QMTCFG=\"version\",0,4\0",
	EC200_AT_MQTT_RECMODE,      				//"AT+QMTCFG=\"recv/mode\",0,0,1",
	EC200_AT_MQTT_SETMODE,       				//"AT+QMTCFG=\"send/mode\",0,0",	
	
	EC200_AT_CSQ,         						//"AT+CSQ\0",					

	////MQTT通讯部分
	EC200_AT_MQTT_OPEN,       					//"AT+QMTOPEN=0,\"119.91.158.8\",1883",
	EC200_AT_MQTT_QMTCNN,       				//"AT+QMTCONN=0,\"rytwj01wwncy26A\",\"admin\",\"7d19d4f675ea0b05553a953bbd86b041\"",
	EC200_AT_MQTT_SUB,       					//"AT+QMTSUB=0,1,\"NewFirmware_down\",0",
	EC200_AT_MQTT_PUB,       					//"AT+QMTPUBEX=0,0,0,0,\"rytwj01wwncy26A_up\",6",

  	EC200_AT_SMS_CMGS,
	//报警拨打电话部分
	EC200_AT_DIAL_ATD,     						//"ATD\0", 											//15 拨打电话 例如：ATD12345678902;
	EC200_AT_DIAL_CLCC,     					//"AT+CLCC\0",										//16 查询当前呼叫
	EC200_AT_DIAL_CPAS,     					//"AT+CLCC\0",										//16 查询当前呼叫
	EC200_AT_DIAL_ATH,     						//"ATH\0",				    						//17 挂机
	EC200_AT_DIAL_ATA,     						//"ATA\0",  		 								//18 来电收到RING ,接通电话。  


	EC200_AT_DIAL_QAUDMOD,     					//"AT+QAUDMOD=2\0",									//34 麦克风 0-主通道,0-增益等级		
	EC200_AT_DIAL_CMUT,     					//"AT+CMUT=1\0",									//35 MIC静音打开
	EC200_AT_DIAL_QTONEDET,     				//"AT+QTONEDET=1\0",								//38 打开DTMF检测
	EC200_AT_DIAL_QWDTMF,     					//"AT+QWDTMF=7,0,\"8,100,100,8,100,100\"\0",		//39 短嘀2声	

  	EC200_AT_MAX,
	
  	EC200_AT_MQTT_SENTDATA,	
  	U15_AT_SMSENTCONTENT,	
 	U15_AT_SMSENTCONTENT_END,
 	U15_AT_SMSENTCONTENT_FAIL,///1B
}U15_AT_TYPEDEF;




typedef struct 
{
	unsigned int powerKeytime;
}EC200C_variable;//variable

void mt_4g_Init(void);
void mt_4g_pro(void);

#endif

