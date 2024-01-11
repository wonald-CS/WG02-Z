#ifndef ____MT_4G_H_
#define ____MT_4G_H_

#include "OS_System.h"

typedef enum
{

	EC200_AT_CPIN=0,  //
	EC200_AT_INIT_ATE0,    ////1 回显关闭
	EC200_AT_INIT_IPR,    ////2 设置波特率为115200	
	EC200_AT_QIDEACT,     //"AT+QIDEACT=1\0",
	EC200_AT_QIACT,       //"AT+QIACT=1\0",	
	
	EC200_AT_INIT_CREG,    ////"AT+CREG?\0",
	EC200_AT_INIT_CGREG,  //"AT+CGREG?\0",
	
	EC200_AT_GSN,       //"AT+GSN\0",	
  EC200_AT_CIMI,       //"AT+CIMI=1\0",		
	EC200_AT_SMSINIT_CSCA,//		"AT+CSCA?\0",				//1 查询短信中心号码
	EC200_AT_SMSINIT_CMGF,//	"AT+CMGF=1\0",				//配置短信设置的方式	
	EC200_AT_SMSINIT_CSCS,//	"AT+CSCS=\"UCS2\"\0",				//配置短信设置的方式	////	"AT+CSCS=\"GSM\"\0",				//配置短信设置的方式		
	EC200_AT_SMSINIT_CCSMP,//	"AT+CSCS=AT+CSMP=49,167,0,25\0",				//配置短信设置的方式	
	EC200_AT_SMSINIT_CNMI,//	"AT+CNMI=2,1,0,1,0\0",		

  EC200_AT_DIAL_CLVL,        //	"AT+CLVL=5\0",				//33 扬声器等级最低
	EC200_AT_DIAL_CLVL_CLOSE,     //"AT+CLVL=0\0",
	EC200_AT_MQTT_VERSION,       //"AT+QMTCFG=\"version\",0,4\0",
	EC200_AT_MQTT_RECMODE,       //"AT+QMTCFG=\"recv/mode\",0,0,1",
	EC200_AT_MQTT_SETMODE,       //"AT+QMTCFG=\"send/mode\",0,0",	
	
	EC200_AT_CSQ,         //"AT+CSQ\0",					  //26		

	////MQTT通讯部分
	EC200_AT_MQTT_OPEN,       //"AT+QMTOPEN=0,\"119.91.158.8\",1883",
	EC200_AT_MQTT_QMTCNN,       //"AT+QMTCONN=0,\"rytwj01wwncy26A\",\"admin\",\"7d19d4f675ea0b05553a953bbd86b041\"",
	EC200_AT_MQTT_SUB,       //"AT+QMTSUB=0,1,\"NewFirmware_down\",0",
	EC200_AT_MQTT_PUB,       //"AT+QMTPUBEX=0,0,0,0,\"rytwj01wwncy26A_up\",6",

  /////////
  EC200_AT_SMS_CMGS,
	//报警拨打电话部分
	EC200_AT_DIAL_ATD,     //"ATD\0", 					//15 拨打电话 例如：ATD12345678902;
	EC200_AT_DIAL_CLCC,     //"AT+CLCC\0",				//16 查询当前呼叫
	EC200_AT_DIAL_CPAS,     //"AT+CLCC\0",				//16 查询当前呼叫
	EC200_AT_DIAL_ATH,     //"ATH\0",				    //17 挂机
	EC200_AT_DIAL_ATA,     //"ATA\0",  		 			//18 来电收到RING ,接通电话。  


	EC200_AT_DIAL_QAUDMOD,     //	"AT+QAUDMOD=2\0",			//34 麦克风 0-主通道,0-增益等级		
	EC200_AT_DIAL_CMUT,     //	"AT+CMUT=1\0",				//35 MIC静音打开
	EC200_AT_DIAL_QTONEDET,     //	"AT+QTONEDET=1\0",			//38 打开DTMF检测
	EC200_AT_DIAL_QWDTMF,     //	"AT+QWDTMF=7,0,\"8,100,100,8,100,100\"\0",	//39					//短嘀2声	

  EC200_AT_MAX,
	
    EC200_AT_MQTT_SENTDATA,	
    U15_AT_SMSENTCONTENT,	
 	U15_AT_SMSENTCONTENT_END,
 	U15_AT_SMSENTCONTENT_FAIL,///1B
}U15_AT_TYPEDEF;
///

typedef enum
{
	U15_AT_RESPONSE_CPIN=0,
	U15_AT_RESPONSE_CREG=1,//	"+CREG: 0,1\0",
	U15_AT_RESPONSE_CGREG=2,//"+CGREG: 0,1\0",
	U15_AT_RESPONSE_MQTTOPEN1,//		"+QMTOPEN: 0,0\0",   ///OPEN
	U15_AT_RESPONSE_MQTTOPEN,//		"+QMTOPEN: 0,0\0",   ///OPEN
	
	U15_AT_RESPONSE_QMTCONN,//	"+QMTSTAT: 0,1\0",		
	U15_AT_RESPONSE_MQTTSUB,//"+QMTSUB: 0,1,0,0\0",
	U15_AT_RESPONSE_MQTTBEX,//"+QMTPUBEX: 0,0,0\0",
	U15_AT_RESPONSE_QMTSTAT,
	U15_AT_RESPONSE_QMTSTAT_FAIL,
	U15_AT_RESPONSE_WAITSENTDAT,  //">\0",
	//U15_AT_RESPONSE_WAITSENTDAT
	U15_AT_RESPONSE_QMTRECV,//"+QMTRECV: 0,0,\0",
	 
	
	U15_AT_RESPONSE_CMTI,
	U15_AT_RESPONSE_CMGS,
	U15_AT_RESPONSE_CLCC_0,		//电话接通
	U15_AT_RESPONSE_CLCC_3,		//电话拨通
	U15_AT_RESPONSE_CPAS0,
	U15_AT_RESPONSE_CLCC_NO_CARRIER,
	
	
    U15_AT_RESPONSE_TONE,
	U15_AT_RESPONSE_RING,
	
	U15_AT_RESPONSE_NOCARRIER,		//		"NO CARRIER",
	U15_AT_RESPONSE_BUSY,		      //	"BUSY",
	U15_AT_RESPONSE_QTTS,
	U15_AT_RESPONSE_CSQ,
	U15_AT_RESPONSE_CMGR,
	U15_AT_RESPONSE_OK, //
	U15_AT_RESPONSE_IPR,	
	U15_AT_RESPONSE_IMEI_86,	
	U15_AT_RESPONSE_POWEREDDOWN,

	U15_AT_RESPONSE_MAX,
}en_u15atRespose;



typedef enum 
{
	//// GSM 初始化部分
	GSM_STATE_NO_CARD=0,//////初始化部分
	GSM_STATE_WAIT_INIT,//	"AT+CPIN?\0",
	GSM_STATE_INIT,
	GSM_STATE_GET_CREG,
	GSM_STATE_GET_CGREG,
	GSM_STATE_SMSMQTT_INIT,

	GSM_STATE_READY,//	"ATE1\0",				//1 回显关闭
	///MQTT 部分
	//GSM_MQTT_INIT,
	GSM_MQTT_OPEN,
	GSM_MQTT_CONN,
	GSM_MQTT_SUB1,
	GSM_MQTT_READY,
	GSM_MQTT_PUB,
	GSM_MQTT_PUB_WAITSENTDAT,
	GSM_MQTT_PUB_SENTDAT,
	GSM_MQTT_PUB_DELAY,
	GSM_MQTT_PUB_END,

	//	////报警拨打电话
	GSM_STATE_SMSDIAL_READY,  ///没有打电话或这发送短信  处于空闲状态  free;
	
	
	GSM_STATE_DIAL_ALARM_START,  ///开始拨号 
	
	GSM_STATE_DIAL_ALARM_ATD,    //拨号
	GSM_STATE_DIAL_ALARM_RINGING,	
	GSM_STATE_DIAL_ALARM_CALLING,	///暂时没有用到
	GSM_STATE_DIAL_ALARM_DELAY,///通话时间延时
	GSM_STATE_DIAL_ALARM_ATH,	
	GSM_STATE_DIAL_ALARM_END,	
	
	////报警主机电话功能
	//GSM_STATE_PHONECOMEIN_START,
	GSM_STATE_DIAL_ATD,  ///拨号
	GSM_STATE_DIAL_RING, ///振铃
	GSM_STATE_DIAL_CALLING,	///通话
	GSM_STATE_DIAL_ATH, //挂机
	GSM_STATE_DIAL_NOCARRIER, //拨号没有回应
	GSM_STATE_DIAL_END,//拨号结束  
	
	
	//// GSM 发送短信部分
	GSM_STATE_SMS_SENT_START,
	GSM_STATE_SMS_SENT_WAITWRITE,
	GSM_STATE_SMS_SENT_WRITE,
	GSM_STATE_SMS_SENT_DELAY,
	GSM_STATE_SMS_SENT_FAIL,
	GSM_STATE_SMS_SENT_END,		
//	//报警拨打电话部分
	GSM_STATE_ALARM_HANDLE,////SIM卡处于处理报警信息处理
	GSM_STATE_ALARM_HANDLEDLEY,	///备注：通过测试证明，发送短信结束后需要等待一段时间之后才可以操作其他的拨号或者发送短信的任务，否则操作会失败，所以加了GSM_STATE_ALARM_HANDLEDLEY
	////电话拨入设置
	//GSM_STATE_PHONECOMEIN_START,
	GSM_STATE_PHONECOMEIN_ATA,
	GSM_STATE_PHONECOMEIN_PUTINPW,///输入密码部分	
	GSM_STATE_PHONECOMEIN_CALLING,	
	GSM_STATE_PHONECOMEIN_ATH,	
	GSM_STATE_PHONECOMEIN_END,

	///电话功能
	GSM_STATE_TELPHONE_START,
	GSM_STATE_TELPHONE_ATD,
	GSM_STATE_TELPHONE_OPERING,	
	GSM_STATE_TELPHONE_ATH,	
	GSM_STATE_TELPHONE_END,	
}GSM_STATE_TYPE;
 

typedef enum
{
	DIALSTA_NOCARD,      //Ã»ÓÐSIM¿¨
	DIALSTA_BUSY_WAIT,   //GSMÃ¦ÂµÖÐ
	DIALSTA_RING,        //ÕñÁåÖÐ
	DIALSTA_CALLING,     //Í¨»°ÖÐ
	DIALSTA_OVER,        //Í¨»°½áÊø
	DIALSTA_NNERROR,     //Î´Öª´íÎó   
}en_dialstate;



///mt_4g.h
#define MQTT_SENTDATA_SIZE_MAX         140

typedef struct 
{
	unsigned char State;				  //当前状态
	unsigned short PreDelayTime;          //发送之前前延时时间
	unsigned short intervalTime;   
	unsigned short timeout;               //超时时间
	unsigned char maxTimes;               //发送的最大的次数	
	unsigned char PhoneNo[20];            ///
	unsigned char SentBuffer[MQTT_SENTDATA_SIZE_MAX];  ///0，1
}str_GsmState;//////////mt_4g.c

typedef enum
{
	DIALTYPE_ALARM, ///报警打电话
	DIALTYPE_CALL,  ///拨号打电话
}en_dialType;

#define GSM_TX_DIALPHONE_BUFF_BYTE_MAX	20//电话号码最大长度

#define MT_GSM_TXQUEUE_SUM       13   //AT指令的缓存个数	
#define MT_GSM_TXBUFFSIZE_MAX	 256  //每条AT指令允许发送的最大的字节数量

#define MT_GSM_RXBUFFSIZE_MAX			500  

typedef struct 
{
	unsigned int powerKeytime;
}EC200C_variable;//variable

extern Queue16  GSMTxMessageIdMsg;//发送短信的队列

void mt_4g_Init(void);
void mt_4g_pro(void);
void mt_4g_DialHandup(void);
unsigned char mt_gsmDialHandle(en_dialType type,unsigned char *pData);
unsigned char mt_4G_LinkState_rssi(void);
void mt_gsmSendMessage(unsigned char *pData);
void mt_setgsmMqttState(str_GsmState pgsmSta);
unsigned char mt_getgsmMqttState(void);
en_dialstate mt_4G_AtaOper(unsigned char *dialnumber);
en_dialstate mt_4G_GetDialSta(void);
en_dialstate mt_4G_AtaOper(unsigned char *dialnumber);
unsigned char mt_getSMSDialState(void);
unsigned char mt_4G_AlarmingAtaOper(unsigned char *dialnumber);
void mt_4g_systemDisarmOper(void);
#endif

