#include "mt_4g.h"
#include "mt_WIFI.h"
#include "mt_api.h"
#include "mt_mqtt.h"
#include "mt_protocol.h"
#include "hal_GPIO.H"
#include "OS_System.h"
#include "hal_uart.h"
#include "string.h"

const unsigned char U15_AT[EC200_AT_MAX][70]=
{
	////初始化部分
	"AT+CPIN?\0",
	"ATE1\0",				//1 回显关闭
	"AT+IPR=115200\0",		//2 设置波特率为115200		
	"AT+QIDEACT=1\0",
	"AT+QIACT=1\0",
	"AT+CREG?\0",
	"AT+CGREG?\0",
	/*****/
	"AT+GSN\0",	
	"AT+CIMI\0",		
	//// 短信初始化
	"AT+CSCA?\0",				//1 查询短信中心号码
	"AT+CMGF=1\0",				//配置短信设置的方式	
	
#ifdef ___SENTSMS_UCS2		
	"AT+CSCS=\"UCS2\"\0",				//配置短信设置的方式		 
	"AT+CSMP=49,167,0,25\0",				//配置短信设置的方式	
	"AT+CNMI=2,1,0,1,0\0",	
#else
  "AT+CSCS=\"GSM\"\0",				//配置短信设置的方式	
//  "AT+CSMP=49,167,0,241\0",				//配置短信设置的方式	
  "AT+CSMP=17,167,0,0\0",				//配置短信设置的方式	
	"AT+CNMI=2,1,0,0,0\0",
#endif

	"AT+CLVL=5\0",				//33 扬声器等级最低 
 	"AT+CLVL=0\0", 
	///MQTT 初始化部分
	////MQTT通讯部分
	"AT+QMTCFG=\"version\",0,4\0",
	"AT+QMTCFG=\"recv/mode\",0,0,1",
	"AT+QMTCFG=\"send/mode\",0,0",	
	
	"AT+CSQ\0",	
	"AT+QMTOPEN=0,\"",//119.91.158.8\",1883",
	"AT+QMTCONN=0,\"",//   rytwj01wwncy26A\",\"admin\",\"7d19d4f675ea0b05553a953bbd86b041\"",
	"AT+QMTSUB=0,1,\"",//NewFirmware_down\",0",
	"AT+QMTPUBEX=0,0,0,0,\"",//rytwj01wwncy26A_up\",6",
    
  ///发送短信部分
   "AT+CMGS=\0",
	 
	//报警拨打电话部分
	"ATD\0", 					//15 拨打电话 例如：ATD12345678902;
	"AT+CLCC\0",				//16 查询当前呼叫
	"AT+CPAS\0",
	"ATH\0",				    //17 挂机
	"ATA\0",  		 			//18 来电收到RING ,接通电话。  
		
	"AT+QAUDMOD=2\0",			//34 麦克风 0-主通道,0-增益等级		
	"AT+CMUT=1\0",				//35 MIC静音打开
	"AT+QTONEDET=1\0",			//38 打开DTMF检测
	"AT+QWDTMF=7,0,\"8,100,100,8,100,100\"\0",	//39					//短嘀2声
};

const unsigned char U15_AT_ResPonse[U15_AT_RESPONSE_MAX][20]=
{
	"+CPIN: READY\0",	
	"+CREG: 0,1\0",  ///+CREG: 0,1
	"+CGREG: 0,1\0",
	"+QMTOPEN: 0,2\0",
	"+QMTOPEN: 0,0\0",   ///OPEN
	
	"+QMTCONN: 0,0,0\0",
	"+QMTSUB: 0,1,0,0\0",
	"+QMTPUBEX: 0,0,0\0",
	"+QMTSTAT: 0,1\0",
	"+QMTSTAT: 0,4\0",
	">\0",
	
	"+QMTRECV: 0,0,\0",
	
	"+CMTI:\0",			//收到新的短信
	"+CMGS:\0",
	
	"+CLCC: 1,0,0,0,0,\0",//+CLCC: 1,0,0,0,0,		//呼叫成功返回
	"+CLCC: 1,0,3,0,0,\0",											//电话拨通返回
	"+CPAS: 0\0",
    "NO CARRIER\0",   ///
	
	"+QTONEDET:\0",//+QTONEDET： 36
	"RING\0",			//来电振铃
	"NO CARRIER\0",		//通话连接时挂断
	"BUSY\0",
	"+QTTS:\0",	
	"+CSQ:\0",
	"+CMGR:\0",
	"OK\0",
	"AT+IPR=\0",
	"86\0",
    "POWERED DOWN\0",
};


////ES200S USART TX 
EC200C_variable  ES200C_Var;
Queue16	         ES200_4GTxIdxMsg;         ///4G AT发送队列
unsigned char ES200_4GTxQueuePos;  ///4G 发送数据队列序号 
unsigned char ES200_4GTxBuff[MT_GSM_TXQUEUE_SUM][MT_GSM_TXBUFFSIZE_MAX];///AT 指令缓存
unsigned char ES200_4GTxdTempBuff[MT_GSM_TXBUFFSIZE_MAX];;
static void EC200S_PutOnHandler(void);
static void mt_ES200C_UsartTxd_Handler(void);
static void ES200C_ApplicationTxd_ManagementFuction(void);



////ES200S USART RX 
volatile Queue1K ES200_4GRxMsg;		  //GSM串口接收队列
unsigned char ES200_4GRxBuff[MT_GSM_RXBUFFSIZE_MAX];
static void mt_4G_RxMsgInput(unsigned char dat);
static void mt_ES200C_UsartRxd_Handler(void);
static void mt_gsmResponseProc(unsigned char *pData,unsigned char Zon);
static unsigned char mt_charIdentify(unsigned char *q,unsigned char *zone,unsigned char *pIdx,unsigned short num);

str_GsmState  GsmState;        //4G 模块的初始化的状态。  READY   
str_GsmState  GsmSmsDialState; //定义4G打电话和发送短信的变量
str_GsmState  GsmMqttState; 
//4G-MQTT
  
//110   有小偷
//119   我警情


#define GSM_TX_MESSAGE_BUFF_MAX         10	//GSM发送短信缓存大小
#define GSM_TX_MESSAGE_BUFF_BYTE_MAX	160	//GSM发送短信每个缓存的字节数，2个字节+20个字节+138字节
/////////////////////////////////////前20个字节保存电话号码   后面140个字节保存发送短信的内容
unsigned char GSMTxMessageBuff[GSM_TX_MESSAGE_BUFF_MAX][GSM_TX_MESSAGE_BUFF_BYTE_MAX];
unsigned char gsmTxMessageQueuePos;

Queue16  GSMTxMessageIdMsg;//发送短信的队列



//void mt_4g_Init(void) 
static unsigned char ES200C_DialSMSPro(void);



//定义SIM卡状态变量
unsigned char gsmSIMStatus;			//SIM 的状态   =0 没有检测到SIM卡    =1 表示检测到了SIM卡
unsigned char gsmCsqVaule;          //定义CSQ 的值
static unsigned char mt_4g_GetCsqValue(unsigned char *pdat);
static unsigned short mt_4g_GetMqttRecvDat(unsigned char *pdat,unsigned char *mqttrecDat);
static unsigned char ES200C_MqttPro(void);




void mt_4g_Init(void)
{
//	unsigned char i;
	ES200C_Var.powerKeytime = 0;
	hal_GPIO_4GPowerKey_L(); 
    
	ES200_4GTxQueuePos = 0;
	QueueEmpty(ES200_4GTxIdxMsg);
	QueueEmpty(ES200_4GRxMsg);
	hal_usart_Uart2DateRxCBSRegister(mt_4G_RxMsgInput);
	
	GsmState.State = GSM_STATE_WAIT_INIT;
	GsmState.maxTimes = 10;
	
	//初始化    void mt_4g_Init(void)
	GsmSmsDialState.State = GSM_STATE_SMSDIAL_READY; ///
	//QueueEmpty(GSMTxMessageIdMsg);///发送短信队列
	gsmTxMessageQueuePos = 0;
	QueueEmpty(GSMTxMessageIdMsg);///发送短信队列
	gsmSIMStatus = 0;
	gsmCsqVaule = 0;	
	
//void mt_4g_Init(void)
	GsmMqttState.State = GSM_MQTT_OPEN;	
	
	
}


void mt_4g_pro(void)
{
  EC200S_PutOnHandler();
	ES200C_ApplicationTxd_ManagementFuction();
	mt_ES200C_UsartTxd_Handler();
	mt_ES200C_UsartRxd_Handler();
	
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





///////////////////////////////////////////
// function:   Push the data sent by the 4G serial port into the queue
// name：      static void mt4g_TxMsgInput(unsigned char *pData)
// parameter:  unsigned char *pData  ---> Send the first address of the data pointer
// author:     wuji mcu programm     xuming
/************************************************************************/

static void mt4g_TxMsgInput(unsigned char *pData)
{
	unsigned short len,i;
	unsigned char *pGWTxData;
	pGWTxData = pData;
	len = pGWTxData[0]<<8;
	len |= pGWTxData[1];
	
	
	memset(ES200_4GTxBuff[ES200_4GTxQueuePos],0,MT_GSM_TXBUFFSIZE_MAX);
	for(i=0; i<len; i++)
	{
		ES200_4GTxBuff[ES200_4GTxQueuePos][i] = pGWTxData[2+i];//ES200_4GTxdTempBuff[i];
	}
	QueueDataIn(ES200_4GTxIdxMsg,&ES200_4GTxQueuePos,1);
	
	
	ES200_4GTxQueuePos++;
	if(ES200_4GTxQueuePos>=MT_GSM_TXQUEUE_SUM)
	{
		ES200_4GTxQueuePos = 0;
	}
}

///////////////////////////////////////////
// function:   4G different functions of data packaging function
// name：      static void hal_usart1TxDataPack(unsigned char fuc,unsigned char *dat)
// parameter:   
//             unsigned char fuc:   ES200S command;
//             unsigned char *dat:  Sent data pointer;
// author:     wuji mcu programm     xuming
/************************************************************************/
static void hal_4GTxDataPack(unsigned char fuc,unsigned char *dat)
{
	unsigned short i;
	memset(ES200_4GTxdTempBuff,0,sizeof(ES200_4GTxdTempBuff));
	if(fuc<EC200_AT_MAX)
	{
		
		// const unsigned char U15_AT[EC200_AT_MAX][70]=
// {
	//初始化部分
	// "AT+CPIN?\0",
	// "ATD123456\0",	   1			//1 回显关闭     0
	// "AT+IPR=115200\0",		//2 设置波特率为115200		
	// "AT+QIDEACT=1\0",
	// "AT+QIACT=1\0",
	// "AT+CREG?\0",
		
		for(i=0;i<MT_GSM_TXBUFFSIZE_MAX;i++)
		{
			if(U15_AT[fuc][i]!=0)
			{
				if(fuc<EC200_AT_MAX)
					ES200_4GTxdTempBuff[i+2] = U15_AT[fuc][i];
			}
			else
			{
				while(*dat != 0)
				{
					ES200_4GTxdTempBuff[i+2] = *dat;   //119.91.158.8",1883   0x00
					dat ++;
					i++;
				}
				ES200_4GTxdTempBuff[i+2]=0x0D;	
				i++;
				ES200_4GTxdTempBuff[i+2]=0x0A;	
				i++;
				ES200_4GTxdTempBuff[0] = ((i+2)>>8)&0xFF;
				ES200_4GTxdTempBuff[1] = (i+2)&0xFF;
				mt4g_TxMsgInput(ES200_4GTxdTempBuff);
				break;
 			}
		}
	}
	else 
	{
		if(fuc == EC200_AT_MQTT_SENTDATA)
		{
			mt4g_TxMsgInput(dat);
		}
		else if(fuc == U15_AT_SMSENTCONTENT)
		{
			mt4g_TxMsgInput(dat);
		}
		else if(fuc == U15_AT_SMSENTCONTENT_END)
		{////U15_AT_SMSENTCONTENT_END
			ES200_4GTxdTempBuff[2]=0x1A;
			ES200_4GTxdTempBuff[0] = 0;
			ES200_4GTxdTempBuff[1] = 3;
			mt4g_TxMsgInput(ES200_4GTxdTempBuff);
		}
		else if(fuc == U15_AT_SMSENTCONTENT_FAIL)
		{
			ES200_4GTxdTempBuff[2]=0x1B;
			ES200_4GTxdTempBuff[0] = 0;
			ES200_4GTxdTempBuff[1] = 3;
			mt4g_TxMsgInput(ES200_4GTxdTempBuff);
		}
	}
}
////////

////////
//发送AT指令到GSM
static void mt_sendATToGSM(unsigned char fuc,unsigned char *dat)
{
  hal_4GTxDataPack(fuc,dat);
}

////
// void ES200S_SentString(unsigned char *buf);
// {
	// hal_Usart2_SentString(buf);
// }

/************************************************************************/
// function:  4G usart tx  data handler
// name：     static unsigned char mt_ES200C_Usarttxd_Handler(void)  
// author:    wuji mcu programm     xuming
/************************************************************************/
static void mt_ES200C_UsartTxd_Handler(void)
{
	static unsigned int tim = 0;
	unsigned char Idx;	
	//发送
	if(QueueDataLen(ES200_4GTxIdxMsg))
	{
		tim ++;
		if(tim > 15)
		{//10*15  150MS
			tim = 0;
			QueueDataOut(ES200_4GTxIdxMsg,&Idx);
			hal_Usart2_SentString(&ES200_4GTxBuff[Idx][0]);
			//ES200S_SentString(&ES200_4GTxBuff[Idx][0]);
		}
	}	
	else
	{
		tim = 0;
	}
}
///////
//workSta   需要改变的工作状态
//PreTim    发送AT指令前的延时时间   单位:10MS
//outTim
//MaxTim    发送AT指令最大的次数 
//*dailNo
//*pda
static void mt_4GWorkStateChange(unsigned char workSta,unsigned short PreTim,unsigned short outTim,unsigned char MaxTim,unsigned char *dailNo,unsigned char *pdat)
{
	unsigned short i;
	GsmState.State = workSta;
//	GsmState.Timer = 0;
	GsmState.PreDelayTime = PreTim;          //发送之前前延时时间
	GsmState.timeout = outTim;               //超时时间
	GsmState.maxTimes = MaxTim;              //发送的最大的次数	
	for(i=0;i<20;i++)
	{
		GsmState.PhoneNo[i] = *dailNo; 
	  dailNo ++;
	}
	for(i=0;i<MQTT_SENTDATA_SIZE_MAX;i++)
	{
		GsmState.SentBuffer[i] = *pdat; 
	    pdat ++;
	}		
}
/////////////

///////////////////////////////////////////
// function:   ES2000 Usart Tx application layer processing functions
// name：      static void hal_usart1TxDataPack(unsigned char fuc,unsigned
// parameter:   
// author:     wuji mcu programm     xuming
/************************************************************************/
static void ES200C_ApplicationTxd_ManagementFuction(void)
{
	unsigned char i;
	static unsigned short timeDelay = 0;
	i = 0;
	switch(GsmState.State)
	{
		case GSM_STATE_NO_CARD:
		{
		}
		case GSM_STATE_WAIT_INIT:
		{//AT+CPIN?
			timeDelay++;					
			if(timeDelay > 200)		//延时2秒	
			{
				timeDelay= 0;
				i = 0;
			    mt_sendATToGSM(EC200_AT_CPIN,&i);
				gsmSIMStatus = 0;  ///没有检测到SIM卡
				if(GsmState.maxTimes)
				{
					GsmState.maxTimes --;			
				}
				else
				{//没有收到对应的数据，则重启4G模块
					 GsmState.maxTimes = 6;
					 ES200C_Var.powerKeytime = 0;
					 hal_GPIO_4GPowerKey_H();
				} 
			}
		}	
		break;
		case GSM_STATE_INIT:
			
			timeDelay++;					
			if(timeDelay > 50)		//50*10ms  =500ms
			{
				 timeDelay= 0;
				 i = 0;
				 
				// "ATE1\0",				//1 回显关闭
				// "AT+IPR=115200\0",		//2 设置波特率为115200		
				// "AT+QIDEACT=1\0",
				// "AT+QIACT=1\0",
				 mt_sendATToGSM(EC200_AT_INIT_ATE0,&i);	
				 mt_sendATToGSM(EC200_AT_INIT_IPR,&i);	
				 mt_sendATToGSM(EC200_AT_QIDEACT,&i);	
				 mt_sendATToGSM(EC200_AT_QIACT,&i);	
				 mt_4GWorkStateChange(GSM_STATE_GET_CREG,0,0,30,0,0);
			}			
		break;		
		case GSM_STATE_GET_CREG:
			if(GsmState.PreDelayTime)
			{
				GsmState.PreDelayTime --;
				timeDelay = 0;
			}
			else
			{
				timeDelay++;					
				if(timeDelay > 200)		//延时3秒	
				{
					 timeDelay= 0;
					 i = 0;
					 mt_sendATToGSM(EC200_AT_INIT_CREG,&i);	//	"AT+CREG?\0",
					
					if(GsmState.maxTimes)
					{
						GsmState.maxTimes --;			
					}
					else
					{//没有收到对应的数据，则重启4G模块
						 //GsmState.maxTimes = 6;
						 ES200C_Var.powerKeytime = 0;
						 hal_GPIO_4GPowerKey_H();
						 mt_4GWorkStateChange(GSM_STATE_WAIT_INIT,0,0,6,0,0);
					} 	
				}	
			}				
		break;		
		case GSM_STATE_GET_CGREG:
			timeDelay++;					
			if(timeDelay > 200)		//延时3秒	
			{
				 timeDelay= 0;
				 i = 0;
				
				 mt_sendATToGSM(EC200_AT_INIT_CGREG,&i);	// "AT+CGREG?\0",	
				
					if(GsmState.maxTimes)
					{
						GsmState.maxTimes --;			
					}
					else
					{//没有收到对应的数据，则重启4G模块
						 //GsmState.maxTimes = 6;
						 ES200C_Var.powerKeytime = 0;
						 hal_GPIO_4GPowerKey_H();
						 mt_4GWorkStateChange(GSM_STATE_WAIT_INIT,0,0,6,0,0);
					} 	
			}				
		break;
         case GSM_STATE_SMSMQTT_INIT:
		{
			if(GsmState.PreDelayTime)
			{
				GsmState.PreDelayTime --;
				timeDelay = 0;
			}
			else
			{
				 i = 0;
				 mt_sendATToGSM(EC200_AT_GSN,&i);	//已讲解	
				 mt_sendATToGSM(EC200_AT_CIMI,&i);//已讲解		
				 mt_sendATToGSM(EC200_AT_SMSINIT_CSCA,&i);   //已讲解	
				 mt_sendATToGSM(EC200_AT_SMSINIT_CMGF,&i);	 //已讲解
				 mt_sendATToGSM(EC200_AT_SMSINIT_CSCS,&i);	 //已讲解
				 mt_sendATToGSM(EC200_AT_SMSINIT_CCSMP,&i);  //已讲解 
				 mt_sendATToGSM(EC200_AT_SMSINIT_CNMI,&i);   //已讲解
				
				 mt_sendATToGSM(EC200_AT_DIAL_CLVL,&i);     //已讲解  
				
			     mt_sendATToGSM(EC200_AT_MQTT_VERSION,&i);//已讲解
				 mt_sendATToGSM(EC200_AT_MQTT_RECMODE,&i);//已讲解
				 mt_sendATToGSM(EC200_AT_MQTT_SETMODE,&i);//已讲解
			     mt_4GWorkStateChange(GSM_STATE_READY,0,0,0,0,0);
				 mt_proto_PutInEvent(ENDPOINT_UP_TYPE_GET_TIME,0);
				 mt_proto_PutInEvent(ENDPOINT_UP_QUERY_NEW_FIRMWARE,0);	

				}				
		}
		break;
		
//static void ES200C_ApplicationTxd_ManagementFuction(void)
//函数中添加		

		 case GSM_STATE_READY:
		{
			if(GsmState.PreDelayTime)
			{
				GsmState.PreDelayTime --;
				timeDelay = 0;
			}
			else
			{
				timeDelay++;					
				if(timeDelay > 3000)		//延时30秒	
				{
					 timeDelay= 0;
					 i = 0;
				 	if((mt_wifi_get_wifi_Mqtt_state() != STA_MQTT_READY))
					{///获取系统时间
						mt_proto_PutInEvent(ENDPOINT_UP_TYPE_GET_TIME,0);
						mt_proto_PutInEvent(ENDPOINT_UP_QUERY_NEW_FIRMWARE,0);	

					}		  
					mt_sendATToGSM(EC200_AT_CSQ,&i);		 
				}	
				if(mt_getSMSDialState() == GSM_STATE_SMSDIAL_READY)
				{
					if(!ES200C_MqttPro())
					{
						timeDelay= 0;
					}					
				}
				if((GsmMqttState.State!=GSM_MQTT_PUB_WAITSENTDAT) && (GsmMqttState.State!=GSM_MQTT_PUB_SENTDAT))
				{////如果MQTT 有任务，则等待处理电话和短信部分的任务
					if(ES200C_DialSMSPro() == 0xff)
					{
						timeDelay= 0;
					}
				}			
			}
		}	
		break;
	
		
		
		
 /*      case GSM_STATE_READY:
		{
			if(GsmState.PreDelayTime)
			{
				GsmState.PreDelayTime --;
				timeDelay = 0;
			}
			else
			{/////////
                i = 0;
				timeDelay++;					
				if(timeDelay > 3000)		//延时30秒	
				{
					timeDelay= 0;
					i = 0;			 
					mt_sendATToGSM(EC200_AT_CSQ,&i);		 
				}
				//if((GsmMqttState.State!=GSM_MQTT_PUB_WAITSENTDAT) && (GsmMqttState.State!=GSM_MQTT_PUB_SENTDAT))
				{////如果MQTT 有任务，则等待处理电话和短信部分的任务
					if(ES200C_DialSMSPro() != 0xff)
					{///如果返回值是0xFF  就表示当前没有发送AT指令操作
						timeDelay= 0;
					}
				}
			}
		}	
		break;
		*/
		
		
		/*case GSM_STATE_READY:
		{
			if(GsmState.PreDelayTime)
			{
				GsmState.PreDelayTime --;
				timeDelay = 0;
			}
			else
			{
                 i = 0;
				 GsmState.PreDelayTime = 300;
	      // mt_sendATToGSM(EC200_AT_CSQ,&i);	
			}
		}	
		break;*/
	}
}










//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/************************************************************************/
// function:  4G hardUsart layer Data hander function, 
//            Push the data received by the serial port into the queue
// name：     static void mt_4G_RxMsgInput(unsigned char dat)
// parameter: unsigned char dat  --->Usart get original data;
// author:    wuji mcu programm     xuming
/************************************************************************/
static void mt_4G_RxMsgInput(unsigned char dat)
{	
	 QueueDataIn(ES200_4GRxMsg,&dat,1);
}



/************************************************************************/
// function:  4G usart rx  data handler
// name：     static unsigned char mt_ES200C_UsartRxd_Handler(void)  
// author:    wuji mcu programm     xuming
/************************************************************************/
static void mt_ES200C_UsartRxd_Handler(void)
{
	static unsigned short es200_rxidx = 0;
	static unsigned short queueDelay = 0;
	unsigned short queueLen;
    unsigned char msgDat;	
    unsigned char flag;
    unsigned char StrAddr;
	en_u15atRespose Zones;
	queueLen = QueueDataLen(ES200_4GRxMsg);
	///  > 
	if(queueLen > 0)
	{
		  queueDelay++;
		  if(queueDelay > 10)
		  {//10MS*10
			  queueDelay = 0;	
			  while(queueLen--)
			  {
				  QueueDataOut(ES200_4GRxMsg,&msgDat);
				  if(msgDat == 0x3E)
				  {///特殊情况  如果获取的数值是 > 
					  QueueDataOut(ES200_4GRxMsg,&msgDat);
					  if(msgDat == 0x20)
					  {
						  Zones = U15_AT_RESPONSE_WAITSENTDAT;
						  mt_gsmResponseProc(0,Zones);						
					  }
				  }			
			  }
		  }
	}
	
	while(queueLen > 3)
    {//0D 0A		//ES200_4GRxBuff[MT_GSM_RXBUFFSIZE_MAX];
		queueDelay = 0;
		if(es200_rxidx >= (MT_GSM_RXBUFFSIZE_MAX-5))
		{/////防止移除  
				es200_rxidx = 0;
				return;
		}	
		QueueDataOut(ES200_4GRxMsg,&msgDat);
		ES200_4GRxBuff[es200_rxidx++] = msgDat;
		
		
		if(msgDat == 0x0A)//||(msgDat == 0x0A))
		{	//OK   0X0D  0X0A
			if(es200_rxidx > 2)
			{
				if(ES200_4GRxBuff[0] == '>')
				{///特殊情况  如果获取的数值是 > 
					Zones = U15_AT_RESPONSE_WAITSENTDAT;
					mt_gsmResponseProc(0,Zones);		
				}
				else
				{
					flag = mt_charIdentify(&ES200_4GRxBuff[0],&Zones,&StrAddr,es200_rxidx);
					if(flag == 0)
					{//U15_AT_RESPONSE_OK
						mt_gsmResponseProc(&ES200_4GRxBuff[0],Zones);	
					}					
				}		
			}
			memset(&ES200_4GRxBuff[0],0,MT_GSM_RXBUFFSIZE_MAX);
			es200_rxidx = 0;					
			return;
		}
	}	
}

/*
字符串匹配
返回值: *zone->收到的指令位置  *pIdx->字符串匹配正确在接收数组里的位置
*/
///
//unsigned char *q  被查找的字符串的首地址 
//unsigned char *zone  查找的结果 我们查找到了数组  U15_AT_ResPonse 中下边zone 对应的字符
//unsigned char *pIdx,  被查找到的数据在 字符串 *q 中的位置
//unsigned short num    被查找的字符串 *q的长度
//返回值： 0表示查找到了有效的数据  0xff 表示没有查找到
static unsigned char mt_charIdentify(unsigned char *q,unsigned char *zone,unsigned char *pIdx,unsigned short num)
{
	unsigned int p,i;
	for(i=0;i<U15_AT_RESPONSE_MAX;i++)
	{ 
 		p=SeekSrting(q,(unsigned char *)U15_AT_ResPonse[i],num);
		if(p != 0xff)			
		{////系统正在振铃中	
			*zone = i;		
			*pIdx = p;
			return 0;
		}	
	}
	return 0xff;
}
/////////////////////
static void mt_gsmResponseProc(unsigned char *pData,unsigned char Zon)
{
	unsigned char *pGsmDat,i;
	unsigned char MqttRecBufferAck[500];
	unsigned char MqttRecBufferHex[250];
	unsigned short lenx;
    str_GsmState sta;
	pGsmDat = pData;
	switch(Zon)
	{
		case U15_AT_RESPONSE_POWEREDDOWN:
		break;
		case U15_AT_RESPONSE_CPIN:			//GSM响应准备就绪	
		{
			//gsmRspSIMFlag = 1;
			mt_4GWorkStateChange(GSM_STATE_INIT,0,0,0,0,0);	
		}
		break;
		case U15_AT_RESPONSE_OK:
	    {
			
			
			
		}
		break;
		case U15_AT_RESPONSE_CREG://	"+CREG: 0,1\0",
		{
			mt_4GWorkStateChange(GSM_STATE_GET_CGREG,0,0,30,0,0);			
		}
		break;		
		case U15_AT_RESPONSE_CGREG://"+CGREG: 0,1\0",
		{
			mt_4GWorkStateChange(GSM_STATE_SMSMQTT_INIT,0,0,0,0,0);
			gsmSIMStatus = 1;  //检测到了SIM卡
	        gsmCsqVaule = 20;
			
		}
		break;	
		
		

//static void mt_gsmResponseProc(unsigned char *pData,unsigned char Zon) 函数中添加


		case U15_AT_RESPONSE_MQTTOPEN://		"+QMTOPEN: 0,0\0",   ///OPEN
			sta.State = GSM_MQTT_CONN;
			sta.SentBuffer[0] = 0xff;
			mt_setgsmMqttState(sta);
		break;

		case U15_AT_RESPONSE_QMTCONN:////
			sta.State = GSM_MQTT_SUB1;
			sta.SentBuffer[0] = 0xff;
			mt_setgsmMqttState(sta);			
		break;
		case U15_AT_RESPONSE_MQTTSUB://"+QMTSUB: 0,1,0,0\0",
		{
			sta.State = GSM_MQTT_READY;
			sta.SentBuffer[0] = 0xff;
			mt_setgsmMqttState(sta);		
		}
		break;		
		case U15_AT_RESPONSE_WAITSENTDAT:
		{
			if(mt_getgsmMqttState() == GSM_MQTT_PUB_WAITSENTDAT) 
			{
				sta.State = GSM_MQTT_PUB_SENTDAT;
				sta.SentBuffer[0] = 0xff;
				mt_setgsmMqttState(sta);					
			}
			if(mt_getSMSDialState() == GSM_STATE_SMS_SENT_WAITWRITE) 
			{
				GsmSmsDialState.State = GSM_STATE_SMS_SENT_WRITE;
				GsmSmsDialState.PreDelayTime = 20;  //发送之前前延时时间	
			}
		}
		break;	
		case U15_AT_RESPONSE_MQTTBEX://"+QMTPUBEX: 0,0,0\0",
		{
			if(mt_getgsmMqttState() == GSM_MQTT_PUB_DELAY) 
			{
				sta.State = GSM_MQTT_PUB_END;
				sta.SentBuffer[0] = 0xff;
				mt_setgsmMqttState(sta);					
			}	
		}
		break;			
		case U15_AT_RESPONSE_QMTSTAT: ///如果MQTT 链接状态断开，则重新链接MQTT 服务器
		case U15_AT_RESPONSE_QMTSTAT_FAIL:	
		
			sta.State = GSM_MQTT_OPEN;
			sta.SentBuffer[0] = 0xff;
			mt_setgsmMqttState(sta);	
		break;
	 

		

		case U15_AT_RESPONSE_QMTRECV:
		{/////////////////////////////
			lenx = mt_4g_GetMqttRecvDat(pGsmDat,MqttRecBufferAck);
			if(!(lenx %2))
			{///lenx 是偶数   ///AA05    0xaa 0x05
				asciiToHexConversion(MqttRecBufferAck,lenx,MqttRecBufferHex);
				mt_protocol_4GMqttRecHandle(&MqttRecBufferHex[0],lenx/2);			
			}
		}
		break;

		
		// case U15_AT_RESPONSE_WAITSENTDAT:
		// {////">\0",
			// /*if(mt_getgsmMqttState() == GSM_MQTT_PUB_WAITSENTDAT) 
			// {
				// sta.State = GSM_MQTT_PUB_SENTDAT;
				// sta.SentBuffer[0] = 0xff;
				// mt_setgsmMqttState(sta);					
			// } */
			// if(mt_getSMSDialState() == GSM_STATE_SMS_SENT_WAITWRITE) 
			// {
				// GsmSmsDialState.State = GSM_STATE_SMS_SENT_WRITE;
				// GsmSmsDialState.PreDelayTime = 20;  //发送之前前延时时间	
			// }
		// }
		// break;
		case U15_AT_RESPONSE_CMGS:		//发送短信  "+CMGS:\0",
			if(mt_getSMSDialState() == GSM_STATE_SMS_SENT_DELAY) 
			{////发送短信完成。
				GsmSmsDialState.State = GSM_STATE_SMS_SENT_END;
			}
		break;
		
		case U15_AT_RESPONSE_CMTI:		//收到新的短信
			
		break;
		
		case U15_AT_RESPONSE_CMGR://获取短信内容
		break;
		
		
		case U15_AT_RESPONSE_CLCC_0:		//电话接通
		{///gsmStateCBS(GSM_EVE_PHONE_CALL,0,0);	
			if(mt_getSMSDialState() == GSM_STATE_DIAL_ALARM_RINGING) 
			{////报警拨打电话振铃中
				 GsmSmsDialState.State = GSM_STATE_DIAL_ALARM_DELAY;
				 i = 0;
				 mt_sendATToGSM(EC200_AT_DIAL_QAUDMOD,&i);	
				 mt_sendATToGSM(EC200_AT_DIAL_QAUDMOD,&i);	///设置麦克风的增益
				 mt_sendATToGSM(EC200_AT_DIAL_QTONEDET,&i); ///打开DTMF检测
				 mt_sendATToGSM(EC200_AT_DIAL_QWDTMF,&i);   //发送DIDI 
				 GsmSmsDialState.timeout = 3000;
			}
            else if(mt_getSMSDialState() == GSM_STATE_DIAL_RING) 
            {////键盘拨打电话振铃中
				  GsmSmsDialState.State = GSM_STATE_DIAL_CALLING;
			}		
		}	
		break;
		case U15_AT_RESPONSE_CLCC_NO_CARRIER:
		case U15_AT_RESPONSE_CPAS0:
		{/////对方挂机 或者没有接听 
			switch(mt_getSMSDialState())
			{
				case GSM_STATE_DIAL_ALARM_RINGING:
				case GSM_STATE_DIAL_ALARM_DELAY:
					GsmSmsDialState.State = GSM_STATE_DIAL_ALARM_ATH;
				break;
				case GSM_STATE_DIAL_RING:
				case GSM_STATE_DIAL_CALLING:
					GsmSmsDialState.State = GSM_STATE_DIAL_NOCARRIER;
				  GsmSmsDialState.timeout = 100;
				break;
			}
		}
		break;
		
		case U15_AT_RESPONSE_CLCC_3:		//电话拨通
	
		break;
		case U15_AT_RESPONSE_TONE:		//DTMF按键触发
			if(mt_getSMSDialState() == GSM_STATE_DIAL_ALARM_DELAY) 
			{
				GsmSmsDialState.timeout = 3000;
			}
		break;		
		
		case U15_AT_RESPONSE_RING:		//电话拨入
			
		break;
		
		case U15_AT_RESPONSE_NOCARRIER:			//通话中挂断
			
		break;
		
		case U15_AT_RESPONSE_BUSY:
			
		break;
		case U15_AT_RESPONSE_QTTS:
		
		break;
	
		case U15_AT_RESPONSE_CSQ:
			gsmCsqVaule = mt_4g_GetCsqValue(pGsmDat);
		break; 
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////打电话
/// 参数状态改变函数

///unsigned char *dialnumber   拨打电话的号码
unsigned char mt_4G_AlarmingAtaOper(unsigned char *dialnumber)
{
		unsigned char dialNum[20],i;
	    if(gsmSIMStatus == 1)
		{
			for(i=0;i<20;i++)
			{
				dialNum[i] = 0xff;
				if((*dialnumber >= '0') && (*dialnumber <= '9'))
				{
					dialNum[i] = *dialnumber - 0x30;
					dialnumber ++;
				}
				else
					break;
			}
			
			if(mt_gsmDialHandle(DIALTYPE_ALARM,&dialNum[0]) == 0)
			{
					return DIALSTA_RING;	
			} 
			else
			{
					return DIALSTA_NNERROR;
			}				
		}
		else
			 return DIALSTA_NOCARD;
}


static void mt_4GSMSDialStateChange(str_GsmState sta)//
{
	unsigned short i;
	GsmSmsDialState.State = sta.State;
	GsmSmsDialState.PreDelayTime = sta.PreDelayTime;          //发送之前前延时时间
	GsmSmsDialState.timeout = sta.timeout;               //超时时间
	GsmSmsDialState.maxTimes = sta.maxTimes;              //发送的最大的次数	
	if(sta.PhoneNo[0] !=0xff)
	{
		for(i=0;i<20;i++)
		{
			GsmSmsDialState.PhoneNo[i] = sta.PhoneNo[i]; 
		}	
	}
	if(sta.SentBuffer[0] !=0xff)
	{
		for(i=0;i<MQTT_SENTDATA_SIZE_MAX;i++)
		{
			GsmSmsDialState.SentBuffer[i] = sta.SentBuffer[i]; 
		}		
	}
}

/////////////
unsigned char mt_getSMSDialState(void)
{
	return GsmSmsDialState.State;// = pgsmSta;
}

static unsigned char ES200C_DialSMSPro(void)
{
	unsigned char i;
	unsigned char idmeg;
	str_GsmState state;
	unsigned short idx;
	unsigned char SmsDataBuff[MT_GSM_TXBUFFSIZE_MAX];
	
	static unsigned short timeDelay = 0;
	switch(mt_getSMSDialState())
	{
		case GSM_STATE_SMSDIAL_READY:
		{////判断是否有拨打电话或者发送短信的任务
			if(QueueDataLen(GSMTxMessageIdMsg))
			{
				QueueDataOut(GSMTxMessageIdMsg,&idmeg); 
				state.State = GSM_STATE_SMS_SENT_START;
				state.PreDelayTime = 10;          //发送之前前延时时间
				state.timeout = 20;               //超时时间
				state.maxTimes = 1;              //发送的最大的次数		 
				for(i=0; i<20; i++)
				{
					if(GSMTxMessageBuff[idmeg][i]!=0xFF)
					{///3
						state.PhoneNo[i] = GSMTxMessageBuff[idmeg][i]-'0';
					}
					else
					{
						state.PhoneNo[i] =  GSMTxMessageBuff[idmeg][i];
					}
				}
				i = 0;
				while(GSMTxMessageBuff[idmeg][i+20]!=0)
				{
					state.SentBuffer[i] = GSMTxMessageBuff[idmeg][i+20];
					i++;
					if(i>=GSM_TX_MESSAGE_BUFF_BYTE_MAX)
					  break;
				}
				state.SentBuffer[i] = '\0';	
				
				mt_4GSMSDialStateChange(state);
			}
			else
			{
				return 0;
			}	
		}	
		break;
		case GSM_STATE_DIAL_ALARM_START:
        {
				if(GsmSmsDialState.PreDelayTime)
				{
					GsmSmsDialState.PreDelayTime --;
				}
				else
				{
					i = 0;
					mt_sendATToGSM(EC200_AT_DIAL_CLVL_CLOSE,&i);
					mt_sendATToGSM(EC200_AT_DIAL_ATH,&i);
					GsmSmsDialState.State = GSM_STATE_DIAL_ALARM_ATD;
					GsmSmsDialState.PreDelayTime = 10;          //发送之前前延时时间					
				}
		}			
		//////报警拨打电话
		break;
		
		case GSM_STATE_DIAL_ATD:
		{
			if(GsmSmsDialState.PreDelayTime)
			{
				GsmSmsDialState.PreDelayTime --;
			}
			else
			{
				i = 0;
				mt_sendATToGSM(EC200_AT_DIAL_CLVL,&i);   ///打电话拨号的时候 音量设置为5 
				mt_sendATToGSM(EC200_AT_DIAL_ATH,&i);    ///挂机操作指令
				mt_sendATToGSM(EC200_AT_DIAL_ATD,&GsmSmsDialState.PhoneNo[0]);
				GsmSmsDialState.State = GSM_STATE_DIAL_RING;
				GsmSmsDialState.PreDelayTime = 0;          //发送之前前延时时间
				GsmSmsDialState.timeout = 1000;               //超时时间
				GsmSmsDialState.maxTimes =40;              //发送的最大的次数	
				timeDelay = 0;		
				return 0;			
			}       
		}	
		break;
		case GSM_STATE_DIAL_RING:
		{///1.5 * 40 
				timeDelay ++;
			    if(timeDelay > 150)// 1.5
				{
					  timeDelay = 0;
						i = 0;
						mt_sendATToGSM(EC200_AT_DIAL_CLCC,&i); //AT+CLCC
					    mt_sendATToGSM(EC200_AT_DIAL_CPAS,&i); //AT+CPAS
					    if(GsmSmsDialState.maxTimes)
						{
							GsmSmsDialState.maxTimes --;
						}
						else
						{
							GsmSmsDialState.State = GSM_STATE_DIAL_ATH;
							GsmSmsDialState.maxTimes = 2;
							timeDelay = 120;
						}
                    return 0;
				}
				GsmSmsDialState.timeout = 1000;
		}
		break;
		case GSM_STATE_DIAL_CALLING:
		{
			timeDelay ++;
			if(timeDelay > 150)
			{
				  timeDelay = 0;
					i = 0;
				  mt_sendATToGSM(EC200_AT_DIAL_CLCC,&i);
				  mt_sendATToGSM(EC200_AT_DIAL_CPAS,&i);
				  return 0;
			}
		}
		break;
		case GSM_STATE_DIAL_NOCARRIER:
		{
			GsmSmsDialState.State = GSM_STATE_DIAL_ATH;
			GsmSmsDialState.maxTimes = 2;
		}
		break;
		case GSM_STATE_DIAL_ATH:
		{
			timeDelay ++;
			if(timeDelay > 150)
			{
				GsmSmsDialState.maxTimes --;
				if(GsmSmsDialState.maxTimes == 0)
				{
					GsmSmsDialState.State = GSM_STATE_DIAL_END;	
				}
				timeDelay = 0;
				i = 0;
				mt_sendATToGSM(EC200_AT_DIAL_ATH,&i);
				return 0;
			}
		}
		break;
        case GSM_STATE_DIAL_END:
		{
			GsmSmsDialState.State = GSM_STATE_SMSDIAL_READY;
		}	
		break;	
		

	
		case GSM_STATE_DIAL_ALARM_ATD:
		{
				if(GsmSmsDialState.PreDelayTime)
				{
					GsmSmsDialState.PreDelayTime --;
				}
				else
				{
					i = 0;
					mt_sendATToGSM(EC200_AT_DIAL_ATD,&GsmSmsDialState.PhoneNo[0]);
					GsmSmsDialState.State = GSM_STATE_DIAL_ALARM_RINGING;
					GsmSmsDialState.PreDelayTime = 0;          //发送之前前延时时间
					GsmSmsDialState.intervalTime = 20;          //间隔时间
				  GsmSmsDialState.timeout = 1000;               //超时时间
					GsmSmsDialState.maxTimes = 30;              //发送的最大的次数	
					timeDelay = 0;					
				}
		}
		break;
		case GSM_STATE_DIAL_ALARM_RINGING:
		{
				timeDelay ++;
			  if(timeDelay > 150)
				{
					  timeDelay = 0;
						i = 0;
						mt_sendATToGSM(EC200_AT_DIAL_CLCC,&i);
					  mt_sendATToGSM(EC200_AT_DIAL_CPAS,&i);
					  if(GsmSmsDialState.maxTimes)
						{
							GsmSmsDialState.maxTimes --;
						}
						else
						{
							GsmSmsDialState.State = GSM_STATE_DIAL_ALARM_ATH;
						}
				}
				GsmSmsDialState.timeout = 1000;
		}
		break;	
		case GSM_STATE_DIAL_ALARM_CALLING:
		{

		}
		break;	
		case GSM_STATE_DIAL_ALARM_DELAY:
			if(GsmSmsDialState.timeout)
			{
				GsmSmsDialState.timeout --;
			}
			else 
			{
				GsmSmsDialState.State = GSM_STATE_DIAL_ALARM_ATH;
			}				
		break;
		case GSM_STATE_DIAL_ALARM_ATH:
		{
					i = 0;
					mt_sendATToGSM(EC200_AT_DIAL_ATH,&i);
			    GsmSmsDialState.State = GSM_STATE_DIAL_ALARM_END;
			    GsmSmsDialState.timeout = 100;
		}
		break;	
		case GSM_STATE_DIAL_ALARM_END:
		{
				if(GsmSmsDialState.timeout)
				{
					GsmSmsDialState.timeout --;
				}
				else 
				{
					GsmSmsDialState.State = GSM_STATE_SMSDIAL_READY;
				}
		}
		break;	


//// GSM 发送短信部分
		case GSM_STATE_SMS_SENT_START:
		{///● AT+CMGS="目标电话号码"
			if(GsmSmsDialState.PreDelayTime)
			{//100ms
				GsmSmsDialState.PreDelayTime --;
			}
			else
			{////
				memset(SmsDataBuff,0,MT_GSM_TXBUFFSIZE_MAX);
				idx = 0;  //0
				SmsDataBuff[idx++]=0x22;		
				i = 0;
				while(GsmSmsDialState.PhoneNo[i] <10)	
				{//0-9
					SmsDataBuff[idx++] = GsmSmsDialState.PhoneNo[i]+0x30;  ////'0'
					i++;	
					if(i>16)///电话号码最长不能超过16位
						 break;					
				}
				SmsDataBuff[idx++]=0x22;	
				SmsDataBuff[idx++] = 0;	
				mt_sendATToGSM(EC200_AT_SMS_CMGS,&SmsDataBuff[0]);// "AT+CMGS=\0",
				GsmSmsDialState.State = GSM_STATE_SMS_SENT_WAITWRITE;
				GsmSmsDialState.timeout = 1000;    //超时时间
				GsmSmsDialState.maxTimes = 1;     //发送的最大的次数		
			}
		}
		break;
		case GSM_STATE_SMS_SENT_WAITWRITE:   //  > 
			if(GsmSmsDialState.timeout)
			{///超时时间 10秒
				GsmSmsDialState.timeout --;
			}
			else
			{
				GsmSmsDialState.State = GSM_STATE_SMS_SENT_FAIL;
			}
		break;
		case GSM_STATE_SMS_SENT_WRITE:
		{
			memset(SmsDataBuff,0,MT_GSM_TXBUFFSIZE_MAX);
			idx = 2;	
			i = 0;
			while(GsmSmsDialState.SentBuffer[i] != 0)
			{
				SmsDataBuff[idx++]=GsmSmsDialState.SentBuffer[i];	
				i++;
			}
			SmsDataBuff[0] = (idx>>8)&0xFF;
			SmsDataBuff[1] = idx&0xFF;
			mt_sendATToGSM(U15_AT_SMSENTCONTENT,&SmsDataBuff[0]);		
			GsmSmsDialState.State = GSM_STATE_SMS_SENT_DELAY;
			GsmSmsDialState.timeout = 10;               //超时时间
			GsmSmsDialState.maxTimes = 1;              //发送的最大的次数	
		}	
		break;
		case GSM_STATE_SMS_SENT_DELAY:
			if(GsmSmsDialState.timeout)
			{//100ms
				GsmSmsDialState.timeout --;
			}
			else
			{
				SmsDataBuff[idx++] = 0;	
				mt_sendATToGSM(U15_AT_SMSENTCONTENT_END,&SmsDataBuff[0]);	
				GsmSmsDialState.State = GSM_STATE_SMS_SENT_END;
				GsmSmsDialState.timeout = 200;    //发送之前前延时时间			
			}
		break;
		case GSM_STATE_SMS_SENT_FAIL:
		{
			SmsDataBuff[0] = 0;
			mt_sendATToGSM(U15_AT_SMSENTCONTENT_FAIL,&SmsDataBuff[0]);
			
			GsmSmsDialState.State = GSM_STATE_SMS_SENT_END;
			GsmSmsDialState.timeout = 600;     //发送之前前延时时间		
		}
		break;
		case GSM_STATE_SMS_SENT_END:
		{
			if(GsmSmsDialState.timeout)
			{
				GsmSmsDialState.timeout --;
			}
			else
			{
				GsmSmsDialState.State = GSM_STATE_SMSDIAL_READY;
			}
		}	
		break;
	}
    return 0xff;
}
	








unsigned char mt_gsmDialHandle(en_dialType type,unsigned char *pData)
{
	str_GsmState state;
	unsigned char i;
	if(mt_getSMSDialState() == GSM_STATE_SMSDIAL_READY)
	{///如果4G处于空闲状态
		if(QueueDataLen(GSMTxMessageIdMsg) == 0)  ///如果目前没有短信需要发送
		{/////发送短信优先。
			i=0;
			memset(&state.PhoneNo[0], 0, 20);
			for(i=0;i<GSM_TX_DIALPHONE_BUFF_BYTE_MAX;i++)
			{//20
				if(*pData < 10) //10086
				{//0-9  电话号码都是数字  由0-9组成
					state.PhoneNo[i] = *pData + 0x30;
					pData++;
				}
				else
					break;
			}	 
			if(i>2)   ///110 119  510 520 
			{///电话号码的个数要多余2位
				state.PhoneNo[i] = ';';
				state.PhoneNo[i+1] = 0;
				if(type == DIALTYPE_ALARM)
					state.State = GSM_STATE_DIAL_ALARM_START;///报警拨打电话
				else
					state.State = GSM_STATE_DIAL_ATD;   ///键盘拨打电话
				state.PreDelayTime = 10;          //发送之前前延时时间
				mt_4GSMSDialStateChange(state);				
				return 0;					
			}
		}
	}
	return 0xff;
}

void mt_4g_DialHandup(void)
{
	unsigned char i;
	i = 0;
	mt_sendATToGSM(EC200_AT_DIAL_ATH,&i);
	mt_sendATToGSM(EC200_AT_DIAL_ATH,&i);
	GsmSmsDialState.State = GSM_STATE_SMSDIAL_READY;
}

////////////////////////
//+CSQ: 26,99
static unsigned char mt_4g_GetCsqValue(unsigned char *pdat)
{
	unsigned char *pdata;
	unsigned char csaVaule = 0;
    pdata = pdat;
	while(*pdata != ':')
	{
		pdata++; 
	}
	
//	+CSQ: 24,99
	while(*pdata != ',')
	{
		pdata++;
		if((*pdata >= 0x30) && (*pdata <= 0x39))   //0x34
		{
			csaVaule *= 10;  //20
			csaVaule += *pdata - 0x30;   //20+4  =24
			//pdata++;
		}
	}
	return csaVaule;
}
unsigned char mt_4G_LinkState_rssi(void)
{
	 if(gsmSIMStatus == 1)
	 {
		if(gsmCsqVaule < 10)
		{///信号很差
				return 0;
		}
		else if(gsmCsqVaule < 15)
		{// 有信号
				return 1;
		}
		else if(gsmCsqVaule < 20)
		{///信号好
				return 2;
		}
		else if(gsmCsqVaule < 32)
		{//信号很好
				return 3;
		}									
	 }
	 else
		 return 0xff;
	 return 0xff;
}

/////////////////////////////////////////////////
/*
	发送短信
	*pData 短信内容
	       数据结构:[0]-[19]电话号码 [20..N]短信内容
*/
void mt_gsmSendMessage(unsigned char *pData)
{
	unsigned char i;
	i=0;
	while(pData[i]!=0)
	{
		GSMTxMessageBuff[gsmTxMessageQueuePos][i] = pData[i];
		i++;
		if(i>=GSM_TX_DIALPHONE_BUFF_BYTE_MAX)
			return;
	}
	i= 20;
	while(pData[i]!=0)
	{
		GSMTxMessageBuff[gsmTxMessageQueuePos][i] = pData[i];
		i++;
		if(i>=GSM_TX_MESSAGE_BUFF_BYTE_MAX)
			return;
	}
	GSMTxMessageBuff[gsmTxMessageQueuePos][i] = '\0';
	
	QueueDataIn(GSMTxMessageIdMsg,&gsmTxMessageQueuePos,1);
	
	gsmTxMessageQueuePos++;
	if(gsmTxMessageQueuePos >= GSM_TX_MESSAGE_BUFF_MAX)
	{
		gsmTxMessageQueuePos = 0;
	}
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////


///设置MQTT 的状态
void mt_setgsmMqttState(str_GsmState pgsmSta)
{
	unsigned char i;
	GsmMqttState.State = pgsmSta.State;
	if(pgsmSta.SentBuffer[0] != 0xff)
	{
		for(i=0;i<MQTT_SENTDATA_SIZE_MAX;i++)
		{
			GsmMqttState.SentBuffer[i]= pgsmSta.SentBuffer[i];
		}	
	}
}
////////////////////
unsigned char mt_getgsmMqttState(void)
{
	if(GsmState.State == GSM_STATE_READY)
	{
		return GsmMqttState.State;// = pgsmSta;
	}
	else
	{
		GsmMqttState.State = GSM_MQTT_OPEN;
		return GSM_MQTT_OPEN;
	};
}


////////////////////////////////////////////
////////////////////////////////////////////
static unsigned char ES200C_MqttPro(void)
{
	unsigned char i,idx,lenx;
    static unsigned int Time_Delay_4gSent = 0; //发送AT指令间隔延时时间	
	unsigned char mqttDataBuff[MT_GSM_TXBUFFSIZE_MAX];
	str_GsmState sta;
	switch(GsmMqttState.State)
	{
		case GSM_MQTT_OPEN:
		{///AT+QMTOPEN=0,"119.91.158.8",1883   //0x31
			// 	"AT+QMTOPEN=0,\"",//119.91.158.8\",1883",/
			//	 AT+QMTOPEN=0," 
			Time_Delay_4gSent++;					
			if(Time_Delay_4gSent == 300)		//延时3秒	
			{
				Time_Delay_4gSent= 0;
				idx	=0;
				i = 0;
				while(mqtt_para.serverIp[i])
				{
				    mqttDataBuff[idx ++] = mqtt_para.serverIp[i++];
					  if(i == MQTT_SERVERIP_SIZE_MAX)
						  break;	
				}
				mqttDataBuff[idx ++] = '\"';
				mqttDataBuff[idx ++] = ',';
				i = 0;
				while(mqtt_para.serverPort[i])
				{
				   mqttDataBuff[idx ++] = mqtt_para.serverPort[i++];
					if(i == MQTT_SERVERPORT_SIZE_MAX)
						break;	   
				}				         
				mqttDataBuff[idx ++] = 0;
				mt_sendATToGSM(EC200_AT_MQTT_OPEN,&mqttDataBuff[0]);
				return 0;
			}		
		}
		break;	
		case GSM_MQTT_CONN:
		{//"AT+QMTCONN=0," 
		  //AT+QMTCONN=0,"038FFFFFF3032533551310743G","38FFFFFF3032533551310743","618e74ee42ce21744318db9847375b5c"

			Time_Delay_4gSent++;					
			if(Time_Delay_4gSent == 300)		//延时3秒	
			{
				//Time_Delay_4gSent= 0;
				idx	=1;
				i = 1;
				mqttDataBuff[0] = '1';
				while(mqtt_para.linkID[i])
				{
					mqttDataBuff[idx ++] = mqtt_para.linkID[i++];
					if(i == MQTT_LINKID_SIZE_MAX)
						break;	
				}
//		       mqtt_para.linkID[0] += 1;
//				if(mqtt_para.linkID[0] > 0x39)
//				  mqtt_para.linkID[0] = 0x30;
				
				//mqttDataBuff[idx ++] = 'G';
				//MAC ID+G
				
				mqttDataBuff[idx ++] = '\"';
				mqttDataBuff[idx ++] = ',';
				mqttDataBuff[idx ++] = '\"';
				
				i = 0;
				while(mqtt_para.username[i])
				{
				mqttDataBuff[idx ++] = mqtt_para.username[i++];
					if(i == MQTT_USERNAME_SIZE_MAX)
						break;	
				}				
				mqttDataBuff[idx ++] = '\"';
				mqttDataBuff[idx ++] = ',';
				mqttDataBuff[idx ++] = '\"';			
				i = 0;
				while(mqtt_para.password[i])
				{
			mqttDataBuff[idx ++] = mqtt_para.password[i++];
					if(i == MQTT_PASSWORD_SIZE_MAX)
						break;	
				}					
				mqttDataBuff[idx ++] = '\"';	
				mqttDataBuff[idx ++] = 0;	
				mt_sendATToGSM(EC200_AT_MQTT_QMTCNN,&mqttDataBuff[0]);
				return 0;
			}	
			else if(Time_Delay_4gSent == 3000)		//延时3秒	
			{
				Time_Delay_4gSent= 0;
			}				
		}
		break;	
		case GSM_MQTT_SUB1:  //消息的订阅
		{//  AT+QMTSUB=0,1,"38FFFFFF3032533551310743_down",0
			Time_Delay_4gSent++;					
			if(Time_Delay_4gSent > 50)		//延时3秒	
			{
				Time_Delay_4gSent= 0;
				idx	=0;
				i = 0;
				while(mqtt_para.subtopic[i])
				{
				  mqttDataBuff[idx ++] = mqtt_para.subtopic[i++];
					if(i == MQTT_TPIC_SIZE_MAX)
						break;	
				}
				mqttDataBuff[idx ++] = '\"';
				mqttDataBuff[idx ++] = ',';				
				mqttDataBuff[idx ++] = '0';
				mqttDataBuff[idx ++] = 0;	
			  mt_sendATToGSM(EC200_AT_MQTT_SUB,&mqttDataBuff[0]);
				return 0;
			}		
		}
		break;
		case GSM_MQTT_READY:
		{
			mt_4G_Mqtt_DatUpdataTask(1);
			Time_Delay_4gSent = 0;
		}
		break;	
		case GSM_MQTT_PUB:
		{// AT+QMTPUBEX=0,0,0,0,"38FFFFFF3032533551310743_up",18
			Time_Delay_4gSent++;					
			if(Time_Delay_4gSent > 1)		//
			{
				Time_Delay_4gSent= 0;
				idx	=0;
				i = 0;
				while(mqtt_para.pubtopic[i])
				{
					mqttDataBuff[idx ++] = mqtt_para.pubtopic[i++];
					if(i == MQTT_TPIC_SIZE_MAX)
						break;	
				}
				mqttDataBuff[idx ++] = '\"';	
				mqttDataBuff[idx ++] = ',';
				
				lenx = GsmMqttState.SentBuffer[0] *256 + GsmMqttState.SentBuffer[1];
				if(lenx > 99)
				{
				   mqttDataBuff[idx ++] = (lenx/100 + '0'); 
				}
				if(lenx > 9)
				{
				   mqttDataBuff[idx ++] = (lenx%100/10 + '0'); 
				}
				mqttDataBuff[idx ++] = (lenx%10 + '0'); 				
				mqttDataBuff[idx ++] = 0;	
				mt_sendATToGSM(EC200_AT_MQTT_PUB,&mqttDataBuff[0]);
				sta.State = GSM_MQTT_PUB_WAITSENTDAT;
				sta.SentBuffer[0] = 0xff;
				mt_setgsmMqttState(sta);		
				return 0;
			}	
		}
		break;
		case GSM_MQTT_PUB_WAITSENTDAT:
		{
			Time_Delay_4gSent = 0;
		}
		break;		
		case GSM_MQTT_PUB_SENTDAT:
		{	
			Time_Delay_4gSent++;					
			if(Time_Delay_4gSent > 2)		//
			{
				Time_Delay_4gSent = 0;
				mt_sendATToGSM(EC200_AT_MQTT_SENTDATA,&GsmMqttState.SentBuffer[0]);
				sta.State = GSM_MQTT_PUB_DELAY;
				sta.SentBuffer[0] = 0xff;
				mt_setgsmMqttState(sta);	
			}
		}
		break;

		case GSM_MQTT_PUB_DELAY:
		{
			Time_Delay_4gSent++;
      if(Time_Delay_4gSent > 2000)
      {
				Time_Delay_4gSent = 0;
				i = 0;                 ///0X1B
			  mt_sendATToGSM(U15_AT_SMSENTCONTENT_FAIL,&i);
        sta.State = GSM_MQTT_PUB_END;
				sta.SentBuffer[0] = 0xff;
				mt_setgsmMqttState(sta);
			}				
		}
		break;
		case GSM_MQTT_PUB_END:
		{
			Time_Delay_4gSent++;					
			if(Time_Delay_4gSent > 5)		//
			{
				sta.State = GSM_MQTT_READY;
				sta.SentBuffer[0] = 0xff;
				mt_setgsmMqttState(sta);	
			}
		}
		break;
	}
	return 0xff;
}

/*
+QMTRECV: 0,0,"rytwj01wwncy26A_down",30,"AA000C2A0007E6031205171226D655"
*/
//asciiToHexConversion(DataBuff,len,hexDataBuff);
//mt_protoclo_RecDatParsing(&hexDataBuff[0],len/2);
static unsigned short mt_4g_GetMqttRecvDat(unsigned char *pdat,unsigned char *mqttrecDat)
{
	unsigned char *pdata,i;
	unsigned short datlen = 0;
    pdata = pdat;
	i = 0;
	while(*pdata != '"')
	{
		pdata++;
		i++;
		if(i>30)
			return 0;
	}
	
	pdata++;
	i = 0;
	while(*pdata != '"')
	{
		pdata++;
	  if(i>100)
			return 0;
	}	 
	pdata++;
	pdata++;
	while(*pdata != ',')
	{
		if((*pdata >= 0x30) && (*pdata <= 0x39))
		{
			datlen *= 10;
			datlen += *pdata - 0x30;
			pdata++;
		}
		else
			return 0;
	}
	pdata++;
	pdata++;
	i = 0;
	while(*pdata != '"')
	{
		if(((*pdata >= '0') && (*pdata <= '9'))||((*pdata >= 'A') && (*pdata <= 'F')))
		{
			*mqttrecDat = *pdata;
			pdata++;
			mqttrecDat ++;	
			i ++;			
		}
		else
		{
			return 0;
		}		
	}	
    if(i == datlen)
	{
		return datlen;
	}
	return 0;
}
///

/////////////////////////////////////////////////////////////
//////
en_dialstate mt_4G_AtaOper(unsigned char *dialnumber)
{
		unsigned char dialNum[20],i;
	  if(gsmSIMStatus == 1)
		{
			for(i=0;i<20;i++)
			{
				dialNum[i] = 0xff;
				if((*dialnumber >= '0') && (*dialnumber <= '9'))
				{
						dialNum[i] = *dialnumber - 0x30;
						dialnumber ++;
				}
				else
						break;
			}
			if(mt_gsmDialHandle(DIALTYPE_CALL,&dialNum[0]) == 0)
			{
					return DIALSTA_RING;	
			} 
      else
      {
					return DIALSTA_NNERROR;
			}				
		}
		else
			return DIALSTA_NOCARD;
}

en_dialstate mt_4G_GetDialSta(void)
{
	switch(mt_getSMSDialState())
	{
		case 	GSM_STATE_DIAL_RING:///
				return DIALSTA_RING;
		case 	GSM_STATE_DIAL_CALLING:
			 return DIALSTA_CALLING;
		case 	GSM_STATE_DIAL_ATH:
			return DIALSTA_OVER;
		case 	GSM_STATE_DIAL_NOCARRIER:
			return DIALSTA_OVER;
		case 	GSM_STATE_DIAL_END:
			return DIALSTA_OVER;
	}
	return DIALSTA_NNERROR;//Î´Öª´íÎó   
}

void mt_4g_systemDisarmOper(void)
{
	unsigned char i;
	i = 0;
	mt_sendATToGSM(U15_AT_SMSENTCONTENT_FAIL,&i);
	mt_sendATToGSM(EC200_AT_DIAL_ATH,&i);
	GsmSmsDialState.State = GSM_STATE_SMSDIAL_READY;
}
