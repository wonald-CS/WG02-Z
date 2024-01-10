#include "OS_System.h"
#include "hal_uart.h"
#include "mt_wifi.h"
#include "mt_API.h"
#include "mt_mqtt.h"
#include "hal_Gpio.h"
#include "string.h"
#include "mt_protocol.h"


//安信可ESP12 模块AT指令集网址: https://docs.espressif.com/projects/esp-at/zh_CN/latest/AT_Command_Set/index.html
const unsigned char ESP12_AT[ESP12_AT_MAX][70]=
{
	"AT+RST",
	"AT\0",////,             ///WIFI 测试指令
	"ATE1\0",            //打开回显
	"AT+CWSTATE?\0",        //
	"AT+CWMODE=1\0",    ///配置WIFI工作模式 =0 关闭WIFI   =1 Station模式 =2 softAP模式 =3 softAP+ Station模式
	"AT+CWAUTOCONN=1\0",//0 上电不自动链接AP   =1 上电自动链接AP
	"AT+CWSTARTSMART=2\0",//启动某种类型的SmartConfig模式  1：ESP=TOUCH  2:AirKiss 3 AirKiss+Esptouch
	"AT+CWSTOPSMART\0",//	停止SmartConfig	
	"AT+CWSTATE?\0",      //获取WIFI 的链接状态 
//+CWSTATE:<state>,<"ssid">
//	<state>：当前 Wi-Fi 状态
//0: ESP station 尚未进行任何 Wi-Fi 连接
//1: ESP station 已经连接上 AP，但尚未获取到 IPv4 地址
//2: ESP station 已经连接上 AP，并已经获取到 IPv4 地址
//3: ESP station 正在进行 Wi-Fi 连接或 Wi-Fi 重连
//4: ESP station 处于 Wi-Fi 断开状态
//<”ssid”>：目标 AP 的 SSID
	"AT+CWLAP=\"\0",	///获取WIFI的信号 强弱
	"AT+MQTTUSERCFG=0,1,\"",  ///MQTT  CONFESP12_AT_MQTTUSERCFG,// 
	"AT+MQTTCONN=0,\"",    ///MQTT CONNESP12_AT_MQTTCONN,
	"AT+MQTTPUB=0,\"",     ///????ESP12_AT_MQTTPUB,
	"AT+MQTTSUB=0,\"",     ///????ESP12_AT_MQTTSUB,// 
	"AT+MQTTCLEAN=0",      ///??MQTT ??	ESP12_AT_MQTTCLEAN,//
};

//ESP12_AT_RESPONSE_CWSTATE
//mt_wifi.c
const unsigned char ESP12_AT_ResPonse[ESP12_AT_RESPONSE_MAX][27]=
{
	"WIFI CONNECTED\0",   //
	"WIFI DISCONNECT\0",  //0
	"+CWSTATE:\0",          //获取WIFI的链接状态	
	"+CWJAP:\0",			//获取WIFI的信号值
	"ERROR\0",           //WIFI模块返回值	
	"Smart get wifi info\0",//表示获取到了WIFI 的名称和密码
	"smartconfig connected wifi\0",     //获取WIFI密码成功 配网成功
	"+CWLAP:\0",
	"+MQTTCONNECTED:0\0",
	"+MQTTDISCONNECTED:0\0",//+MQTTDISCONNECTED:0
	"+MQTTSUBRECV:0,\0",
	"OK\r\n\0",           //WIFI模块返回值
};
/////


Queue16	 Usart2TxIdxMsg;				   	//网关串口发送数据帧的下标队列	
unsigned char Usart2TxQueuePos;             //下一条发送数据的保存在二位数组的位置
unsigned char Usart2GWTxBuff[WIFI_TX_QUEUE_SUM][WIFI_TX_BUFFSIZE_MAX];

unsigned char wifi_Rx_Buffer[WIFI_RXBUFFSIZE_MAX];
volatile Queue1K  Wifi_RxIdxMsg;					    //

//mt_wifi.c   WIFI工作状态变量
en_Esp12_sta  esp12Sta;
en_Esp12_link Esp12_link;
en_mqtt_sta wifiMqttSta;  ///MQTT 工作状态


//mt_wifi.c
unsigned char WifiSsid[WIFI_SSIDLEN_MAX];


					    //
static void mt_wifi_RxMsgInput(unsigned char dat);
static void hal_WifiRx_Pro(void);
static void mt_WifiTx_Pro(void);
static unsigned char mt_Wifi_charIdentify(unsigned char *q,unsigned char *zone,unsigned char *pIdx,unsigned short num);
static void mt_WifiResponseProc(unsigned char *pData,en_esp12_atResponse res,unsigned short strlon);
static unsigned char mt_wifi_PowerManage(en_wifiPowerManageSta fuc);
static void mt_wifi_changMqttState(en_mqtt_sta sta);
static unsigned char mt_wifi_MQTTRECVResposePro(unsigned char *p,unsigned char *RecDat);
static unsigned char mt_wifi_Mqtt_Pro(void);


void mt_wifi_init(void)
{
	unsigned char i;
 
	QueueEmpty(Wifi_RxIdxMsg);
	memset(&wifi_Rx_Buffer[0],0,WIFI_RXBUFFSIZE_MAX);	
	hal_usart_Uart3DateRxCBSRegister(mt_wifi_RxMsgInput);		
	
	QueueEmpty(Usart2TxIdxMsg);
	Usart2TxQueuePos = 0;
	for(i=0; i<WIFI_TX_QUEUE_SUM; i++)
	{
		memset(&Usart2GWTxBuff[i], 0, sizeof(Usart2GWTxBuff[i]));
	}	
	//void mt_wifi_init(void) 函数中   初始化 en_Esp12_sta
    esp12Sta = ESP12_STA_DETEC_WIFIMODULE;

	memset(WifiSsid, 0, sizeof(WifiSsid));	
  Esp12_link = ESP12_LINK_FAIL;

  //wifiMqttSta = STA_MQTT_FREE;
mt_wifi_changMqttState(STA_MQTT_FREE);
}

void mt_wifi_pro(void)
{
	if(mt_wifi_PowerManage(STA_WIFI_POWER_IDLE))
	{
		hal_WifiRx_Pro();
		mt_WifiTx_Pro();			
	}

}

//en_mqtt_sta wifiMqttSta; ///MQTT 工作状态
//// 修改MQTT 工作状态
static void mt_wifi_changMqttState(en_mqtt_sta sta)
{
	wifiMqttSta = sta;
	QueueEmpty(Wifi_RxIdxMsg);
}
///获取MQTT 工作状态
unsigned char mt_wifi_get_wifi_Mqtt_state(void)
{
	return wifiMqttSta;
}




////// 获取WIFI模块的工作状态
unsigned char mt_wifi_get_wifi_work_state(void)
{
	return esp12Sta;
}
//////设置WIFI模块的工作状态
void mt_wifi_changState(en_Esp12_sta sta)
{
	esp12Sta = sta;
	QueueEmpty(Wifi_RxIdxMsg);
}
static void mt_wifi_RxMsgInput(unsigned char dat)
{	
	QueueDataIn(Wifi_RxIdxMsg,&dat,1);
}

// static void hal_WifiRx_Pro(void)
// {
	// unsigned char len,i;
  // unsigned char rxbuffer[20];
	
  // len = QueueDataLen(Wifi_RxIdxMsg);
	// if(len> 3)
	// {//0D 0A
		 // if(len>=20)
				// len = 20;
			// for(i=0;i<len;i++)
			// {
					// QueueDataOut(Wifi_RxIdxMsg,&rxbuffer[i]);		
			// }
      // USART1_PutInDebugString(rxbuffer,len); 			
	// }
// }

//41 54 0D 0A 0D 0A 4F 4B 0D 0A 41 54 45 31 0D 0A 0D 0A 4F 4B 0D 0A 41 54 2B 43 57 53 54 41 54 45 3F 0D 0A 2B 43 57 53 54 41 54 45 3A 32 2C 22 78 75 6D 69 6E 22 0D 0A 0D 0A 4F 4B 0D 0A 
static void hal_WifiRx_Pro(void)
{
	static unsigned short rxbufferIDX = 0;
	unsigned char msgDat;
	unsigned char flag;
	unsigned char StrAddr;
	
	en_esp12_atResponse reponse;
	while(QueueDataLen(Wifi_RxIdxMsg) > 1)
	{//0D 0A
		if(rxbufferIDX >= (WIFI_RXBUFFSIZE_MAX-5))
		{///// 
			rxbufferIDX = 0;
			return;
		}				
		QueueDataOut(Wifi_RxIdxMsg,&msgDat);
		wifi_Rx_Buffer[rxbufferIDX++] = msgDat;

		if((msgDat == 0x0D)||(msgDat == 0x0A))
		{	
			if(msgDat == 0x0D)
			{
				wifi_Rx_Buffer[rxbufferIDX++] = 0X0A;
				QueueDataOut(Wifi_RxIdxMsg,&msgDat);
			}
			
			if(rxbufferIDX > 2)
			{	

		//wifi_Rx_Buffer[] = {0x41,0x54,0x0D,0x0A};  //4  AT/ OK
		    flag=mt_Wifi_charIdentify(&wifi_Rx_Buffer[0],&reponse,&StrAddr,rxbufferIDX);
				if(flag == 0)
				{					
					mt_WifiResponseProc(&wifi_Rx_Buffer[0],reponse,rxbufferIDX);	
				}				
			}
			memset(&wifi_Rx_Buffer[0],0,WIFI_RXBUFFSIZE_MAX);
			rxbufferIDX = 0;					
			return;
		}
	}
}


/////////////////////////////////////////////////////////////
//usart2网关数据帧入列,入列的帧数据[0],[1]下标必须为该帧的长度
void hal_usart2TxMsgInput(unsigned char *pData)
{
	unsigned short len,i;
	unsigned char *pGWTxData;
	pGWTxData = pData;
	
	len = pGWTxData[0]<<8;
	len |= pGWTxData[1];
	
	for(i=0; i<len; i++)
	{
		Usart2GWTxBuff[Usart2TxQueuePos][i] = pGWTxData[i];
	}
	
	QueueDataIn(Usart2TxIdxMsg,&Usart2TxQueuePos,1);
	
	Usart2TxQueuePos++;
	if(Usart2TxQueuePos >= WIFI_TX_QUEUE_SUM)
	{
		Usart2TxQueuePos = 0;
	}
}

///////////////////////////////////////////////////////////////////
//"AT+MQTTUSERCFG=0,1,\"\0",
//AT+MQTTUSERCFG=0,1,"038FFFFFF3032533551310743","38FFFFFF3032533551310743","618e74ee42ce21744318db9847375b5c",0,0,""0
//fuc   AT 指令
//*dat  AT指令对应的参数，如果无，则传入0xff
void mt_sendATToWifi(unsigned char fuc,unsigned char *dat)
{
	unsigned short i;
    unsigned char DataBuff[WIFI_TX_BUFFSIZE_MAX];
	///0 1 len   2-data
	if(fuc<ESP12_AT_MAX)
	{
		for(i=0;i<WIFI_TX_BUFFSIZE_MAX;i++)
		{
			if(ESP12_AT[fuc][i]!=0)
			{
				if(fuc<ESP12_AT_MAX)
				   DataBuff[i+2] = ESP12_AT[fuc][i];  //DataBuff[2] = 'A'  DataBuff[3] = 'T'   0
			}
			else
			{
				if(*dat!=0xff)
				{//0X30
					if(fuc == ESP12_AT_CWLAP)
					{
						while(*dat != 0xff)
						{
							DataBuff[i+2]=*dat;	i++;
							dat ++;
						}
						DataBuff[i+2]='"';	i++;
					}
					else 
					{
						while(*dat != 0)
						{
							DataBuff[i+2]=*dat;	i++;
							dat ++;
						}	
					}
				}
				DataBuff[i+2]=0x0D;	//DataBuff[4] = 0X0D
				i++;
				DataBuff[i+2]=0x0A;	//DataBuff[5] = 0X0D   4;
				i++;	
				DataBuff[0] = ((i+2)>>8)&0xFF;   //0
				DataBuff[1] = (i+2)&0xFF;        //6
				hal_usart2TxMsgInput(DataBuff);
				break;
			}
		}
	}
}

static void mtWifi_SentString(unsigned char *pData)
{
	unsigned char WIFITxDataBuff[WIFI_TX_BUFFSIZE_MAX];	
	unsigned char *pGateWayData;
	unsigned short i,GateWayBuffLen;
	
	pGateWayData = pData;
	GateWayBuffLen = pGateWayData[0];
	GateWayBuffLen <<= 8;
	GateWayBuffLen |= pGateWayData[1];
	
	GateWayBuffLen -= 2;		//减去非网关协议数据长度2个字节  AT   6;   4
	
	for(i=0; i<GateWayBuffLen; i++)
	{
 		WIFITxDataBuff[i] = pGateWayData[i+2];	
#ifdef DEBUG_TO_HARDUART_WIFI_TX
		QueueDataIn(DebugTxMsg,&WIFITxDMAMapBuff[i],1); //usart1的调试输出函数放这里		
#endif		
	}
	//Hal_Uart3_Send_Data(&pGateWayData[i+2],GateWayBuffLen);
	Hal_Uart3_Send_Data(WIFITxDataBuff,GateWayBuffLen);
}
/////////////////////////////////////

//mt_wifi.c    函数中 static void mt_WifiTx_Pro(void)
static void mt_WifiTx_Pro(void)
{
	unsigned char Idx,i;
	static unsigned int Time_Delay_WifiSent = 0; /// 
    static unsigned int Time_Delay_WifiSta = 0;
	static unsigned short AtResentTimes = 0;
	if(QueueDataLen(Usart2TxIdxMsg))
	{
		  Time_Delay_WifiSent ++;
			if(Time_Delay_WifiSent > 10)
			{/////WIFI 
			   Time_Delay_WifiSent = 0;				
			   QueueDataOut(Usart2TxIdxMsg,&Idx);
				 mtWifi_SentString(&Usart2GWTxBuff[Idx][0]);
			}
	}
	
	
	switch(mt_wifi_get_wifi_work_state())
	{
		case ESP12_STA_RESET:
		{
				Time_Delay_WifiSta ++;
			  if(Time_Delay_WifiSta > 200)
				{
					i = 0xFF;
					Time_Delay_WifiSta = 0;
				  	mt_sendATToWifi(ESP12_STA_RESET,&i);
					mt_wifi_changState(ESP12_STA_DETEC_WIFIMODULE);
				}
		}
		break;
		case ESP12_STA_DETEC_WIFIMODULE:
		{
				Time_Delay_WifiSta ++;
			    if(Time_Delay_WifiSta > 300)
				{
					i = 0xFF;
					Time_Delay_WifiSta = 0;
				  	mt_sendATToWifi(ESP12_AT_AT,&i);
				}
		}
		break;
		case ESP12_STA_DETEC_INIT:       ////WIFIÄ£¿é³õÊ¼»¯
		{
				Time_Delay_WifiSta ++;
			    if(Time_Delay_WifiSta > 200)
				{
					  i = 0xFF;
					  Time_Delay_WifiSta = 0;
					  mt_sendATToWifi(ESP12_AT_ATE,&i);
					  mt_sendATToWifi(ESP12_AT_CWMODE,&i);
					  mt_sendATToWifi(ESP12_AT_CWAUTOCONN,&i);
					  mt_wifi_changState(ESP12_STA_DETEC_STA);				  
				}
		}
		break;
		case ESP12_STA_DETEC_STA:        ////检测WIFI 状态
		{
				Time_Delay_WifiSta ++;
			    if(Time_Delay_WifiSta > 200)
				{
					AtResentTimes ++;
					if(AtResentTimes > 10)
					{
						AtResentTimes = 0;
					 	mt_wifi_PowerManage(STA_WIFI_POWER_RESET);
						return;
					}
					i = 0xFF;
					Time_Delay_WifiSta = 0;
					mt_sendATToWifi(ESP12_AT_CWSTATE,&i);
				}
		}
		break;
		case ESP12_STA_GET_PASSWORD:     ///
		{
			i = 0xFF;
			QueueEmpty(Usart2TxIdxMsg);
			mt_sendATToWifi(ESP12_AT_CWSTOPSMART,&i);
			mt_sendATToWifi(ESP12_AT_CWSTARTSMART,&i);
			mt_wifi_changState(ESP12_STA_GETING_PASS);	
			Time_Delay_WifiSta = 0;
		}
		break;
		case ESP12_STA_GETING_PASS:
		case ESP12_STA_GET_SMART_WIFINFO:
		{
			Time_Delay_WifiSta ++;
			if(Time_Delay_WifiSta > 30000)
		    {//5分钟
			    Time_Delay_WifiSta = 0;
				AtResentTimes = 0;	
				mt_wifi_changState(ESP12_STA_GETING_FAIL);	
			}		
		}	
		break;
		case ESP12_STA_GETING_FAIL:
		case ESP12_STA_GETING_SUC:
		{//100ms
			AtResentTimes ++;
			if(AtResentTimes > 10)
			{
				AtResentTimes = 0;
			    i = 0xFF;
			    QueueEmpty(Usart2TxIdxMsg);
			    mt_sendATToWifi(ESP12_AT_CWSTOPSMART,&i);
				mt_wifi_changState(ESP12_STA_DETEC_STA);	
			}	
		}	
		break;

		case ESP12_STA_DETEC_READY:      ////
		{
			
	
			Time_Delay_WifiSta ++;
			i = 0xFF;
			if(Time_Delay_WifiSta == 2500)
			{
					mt_sendATToWifi(ESP12_AT_CWSTATE,&i);
			}
			else if(Time_Delay_WifiSta > 3000)
			{
				mt_proto_PutInEvent(ENDPOINT_UP_QUERY_NEW_FIRMWARE,0);	
			    mt_proto_PutInEvent(ENDPOINT_UP_TYPE_GET_TIME,0);;
				Time_Delay_WifiSta = 0;				
			}
			
 		  if(!mt_wifi_Mqtt_Pro())
			{///如果MQTT 有数据发送的时候  Time_Delay_WifiSta 清零
				Time_Delay_WifiSta = 0;	
			}		
			
			
		}
		break;
		case ESP12_STA_UP_ALARMDAT:
		{
		
		}
		break;
	}
}










/*
字符串匹配
返回值: *zone->收到的指令位置  *pIdx->字符串匹配正确在接收数组里的位置
/// 需要查找的数据
/// 返回查找的结果
/// 有效地址
/// unm 有效的数据的长度
*/
static unsigned char mt_Wifi_charIdentify(unsigned char *q,unsigned char *zone,unsigned char *pIdx,unsigned short num)
{
	unsigned int p,i;
	for(i=0;i<ESP12_AT_RESPONSE_MAX;i++)
	{ 
 		p=SeekSrting(q,(unsigned char *)ESP12_AT_ResPonse[i],num);
		if(p != 0xff)			
		{////系统正在振铃中	
			*zone = i;		
			*pIdx = p;
			return 0;
		}	
	}
	return 0xff;
}

/////获取WIFI 的状态
static unsigned char mt_wifi_CwstateResposePro(unsigned char *p,unsigned char ssid[])
{
	unsigned char *pdata;
	unsigned char state;
	
	pdata = p;
	//0x32   0 1 2 3 4 
	//+CWSTATE:2,"wuji217"
	while(*pdata != ':') 
	{
		pdata++;	
	}
	pdata++;
	  
	state = *pdata - 0x30;

	while(*pdata != '"')
	{
		pdata++;	
	}			


	pdata++;

	while(*pdata != '"') 
	{
		*ssid = *pdata;
		ssid ++;

		pdata++;	
	}
	return state;
}


static void mt_WifiResponseProc(unsigned char *pData,en_esp12_atResponse res,unsigned short strlon)
{
  static unsigned char len,DataBuff[WIFI_RXBUFFSIZE_MAX];
	static unsigned char hexDataBuff[WIFI_RXBUFFSIZE_MAX];
	switch((unsigned char)res)
	{
		case ESP12_AT_RESPONSE_WIFI_CONNECTED:
		{
			if(mt_wifi_get_wifi_work_state() != ESP12_STA_GET_SMART_WIFINFO)
			{
					mt_wifi_changState(ESP12_STA_DETEC_WIFIMODULE);
					mt_wifi_changMqttState(STA_MQTT_FREE);			
			}
		}
		break;
		case ESP12_AT_RESPONSE_WIFI_DISCONNECT:
		{//WIFI break;
			  if(mt_wifi_get_wifi_work_state() != ESP12_STA_GETING_PASS)
				{
					mt_wifi_changState(ESP12_STA_DETEC_WIFIMODULE);
					mt_wifi_changMqttState(STA_MQTT_FREE);
					mt_wifi_PowerManage(STA_WIFI_POWER_RESET);					
				}
		}
		break;
	  case ESP12_AT_RESPONSE_CWSTATE:
		{
			 memset(WifiSsid,0,sizeof(WifiSsid));	 
			 Esp12_link = (en_Esp12_link)mt_wifi_CwstateResposePro(pData,WifiSsid);
			 if(Esp12_link == ESP12_LINK_SUC)
			 {
					mt_wifi_changState(ESP12_STA_DETEC_READY);
			 }
			 else 
			 {
					mt_wifi_changState(ESP12_STA_DETEC_STA);
			 }
		}
		
		case ESP12_AT_RESPONSE_CWJAP:
		{
		}
		break; 
		case ESP12_AT_RESPONSE_ERROR:
		{
		}
		break;
		case ESP12_AT_RESPONSE_OK:
		{
			switch(mt_wifi_get_wifi_work_state())
			{
				case ESP12_STA_DETEC_WIFIMODULE:
				{//AT
					mt_wifi_changState(ESP12_STA_DETEC_INIT);						
				}
				break;
				case ESP12_STA_DETEC_READY:
				{
					switch(mt_wifi_get_wifi_Mqtt_state())
					{
						case STA_MQTT_CONF:
						{
							mt_wifi_changMqttState(STA_MQTT_CONN);	
						}
						break;
						case STA_MQTT_SUB:
						{
							mt_wifi_changMqttState(STA_MQTT_READY);	
						}	
						break;
					}	

				}
				break;	
			}

		}
		break; 
		case ESP12_AT_RESPONSE_CWLAP:
		{
		
		}
		break;
		case ESP12_AT_RESPONSE_SMART_GET_WIFIWINFO:
		{
			if(mt_wifi_get_wifi_work_state() == ESP12_STA_GETING_PASS)
			{
			 	mt_wifi_changState(ESP12_STA_GET_SMART_WIFINFO);	
			}
		}
		break;
		case ESP12_AT_RESPONSE_SMART_SUC:
		{  ///
			if(mt_wifi_get_wifi_work_state() == ESP12_STA_GET_SMART_WIFINFO)
			{//配网成功
				mt_wifi_changState(ESP12_STA_GETING_SUC);	
			}
		}
		break;	
		case ESP12_AT_RESPONSE_MQTTCONN://	"+MQTTCONNECTED:0\0",
		{
			mt_wifi_changMqttState(STA_MQTT_SUB);		
		}
		break;

		case ESP12_AT_RESPONSE_MQTTDISCONN:
		{
			  mt_wifi_changState(ESP12_STA_DETEC_WIFIMODULE);
				mt_wifi_changMqttState(STA_MQTT_FREE);
				mt_wifi_PowerManage(STA_WIFI_POWER_RESET);	
		}
		break;
		case ESP12_AT_RESPONSE_MQTTRECV://"+MQTTSUBRECV:0,\0",
		{
			len = mt_wifi_MQTTRECVResposePro(pData,DataBuff);
			if(!(len %2))
			{
				asciiToHexConversion(DataBuff,len,hexDataBuff);				
				mt_protocol_WIFIMqttRecHandle(&hexDataBuff[0],len/2);
			}
			mt_mqtt_SetNewFlag(MQTT_REC_MESSAGE_NEW);			  
		}
		break;
	}
}


///////////////////////////////////////////////////
// 1. 如果没电断电的情况i下， 关闭WIFI电源，
// 2. 电源复位操作

static unsigned char mt_wifi_PowerManage(en_wifiPowerManageSta fuc)
{
	static unsigned short StartTime = 2000;
	if(hal_Gpio_AcStateCheck() == STA_AC_LINK)
	{
		if(fuc == STA_WIFI_POWER_RESET)
		{//Close WIFI Power
			StartTime = 0;	
			mt_wifi_changState(ESP12_STA_DETEC_WIFIMODULE);
			mt_wifi_changMqttState(STA_MQTT_FREE);
		}
    if(StartTime == 0)
		{
			mt_wifi_init();
			hal_GPIO_WIFIPowerEN_H();
		}
		if(StartTime < 1000)
		{
			StartTime ++;
			if(StartTime > 150)
			{//
				StartTime = 2000;
				hal_GPIO_WIFIPowerEN_L();
			}
		}	
		return 1;
	}
	else
	{
		StartTime = 0;
		esp12Sta = ESP12_STA_DETEC_WIFIMODULE;
		Esp12_link = ESP12_LINK_FAIL;
		//wifi_rssi = 0;
		wifiMqttSta = STA_MQTT_FREE;
		hal_GPIO_WIFIPowerEN_H();
		return 0;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
///WIFI MQTT通讯部分代码   
//mt_wifi.h 文件中定义

//unsigned char mqttPubtDataTest[]={0,9,0xAA,0x00,0x06,0x29,0x00,0x00,0x08,0x27,0x55};
//mt_wifi.c 文件中定义
////函数功能： WIFI MQTT 通讯处理函数
////返回值:    0 表示MQTT有数据需要处理    0xff表示空闲
static unsigned char mt_wifi_Mqtt_Pro(void)
{
	unsigned char idx,i;
	unsigned char mqttDataBuff[WIFI_MQTT_SENTDATASIZE_MAX];//保存AT指令后面的参数的
	static unsigned int Time_Delay_WifiSent = 0; ////发送AT指令间隔延时时间
	static unsigned char reSentTimes = 0;
	memset(mqttDataBuff,0,WIFI_MQTT_SENTDATASIZE_MAX);
	
	
	
///en_mqtt_sta wifiMqttSta;  ///MQTT 工作状态	
	switch(wifiMqttSta)
	{
		case STA_MQTT_FREE:
		{
			Time_Delay_WifiSent	++;
			if(Time_Delay_WifiSent > 200)  //10*200 =2
			{
				Time_Delay_WifiSent = 0;
				mqttDataBuff[0] = 0xff;
				mt_sendATToWifi(ESP12_AT_MQTTCLEAN,&mqttDataBuff[0]);
				mt_wifi_changMqttState(STA_MQTT_CONF);
				return 0;
			}
		}
		break;
		case STA_MQTT_CONF:
		{
			Time_Delay_WifiSent	++;
			if(Time_Delay_WifiSent > 200)
			{
				//"AT+MQTTUSERCFG=0,1,\"", 0x30
				///AT+MQTTUSERCFG=0,1,"038FFFFFF3032533551310743","38FFFFFF3032533551310743","618e74ee42ce21744318db9847375b5c",0,0,""
				Time_Delay_WifiSent = 0;
				mqttDataBuff[0] = '2';
				idx	=1;
				i = 1;
				while(mqtt_para.linkID[i])
				{
					mqttDataBuff[idx ++] = mqtt_para.linkID[i++];
					if(i == MQTT_LINKID_SIZE_MAX)
						break;	
				}
				
//				mqtt_para.linkID[0] += 1;
//				if(mqtt_para.linkID[0] >= 0x39)
//				  mqtt_para.linkID[0] = 0x30;
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
				mqttDataBuff[idx ++] = ',';	
				mqttDataBuff[idx ++] = '0';
				mqttDataBuff[idx ++] = ',';	
				mqttDataBuff[idx ++] = '0';
				mqttDataBuff[idx ++] = ',';					
				mqttDataBuff[idx ++] = '\"';
				mqttDataBuff[idx ++] = '\"';
				mqttDataBuff[idx ++] = 0;
				
				
				
				mt_sendATToWifi(ESP12_AT_MQTTUSERCFG,&mqttDataBuff[0]);
				//reSentTimes = 0;
				reSentTimes = 0;
				return 0;
			}
		}
		break;
		case STA_MQTT_CONN:
		{
			Time_Delay_WifiSent	++;
			if(Time_Delay_WifiSent > 200)
			{
				Time_Delay_WifiSent = 0;
				reSentTimes ++;
				if(reSentTimes > 2)
				{
						reSentTimes = 0;
						mt_wifi_PowerManage(STA_WIFI_POWER_RESET);  ///WIFI 复位
					  return 0;
				}
				
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
				mqttDataBuff[idx ++] = ',';	
				mqttDataBuff[idx ++] = '0';
				mqttDataBuff[idx ++] = 0;
				mt_sendATToWifi(ESP12_AT_MQTTCONN,&mqttDataBuff[0]);
				return 0;	
			}
		}
		break;
		case STA_MQTT_SUB:
		{
			Time_Delay_WifiSent	++;
			if(Time_Delay_WifiSent > 200)
			{
				Time_Delay_WifiSent = 0;
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
		        mt_sendATToWifi(ESP12_AT_MQTTSUB,&mqttDataBuff[0]);
				return 0;	
			}
		}
		break;
		case STA_MQTT_READY:
		{
			Time_Delay_WifiSent	++;
			
			if(Time_Delay_WifiSent > 20)
			{
				Time_Delay_WifiSent = 0;
				mt_wifi_Mqtt_DatUpdataTask(0);
			}		
		}
		break;
		case STA_MQTT_PUB:
		{
			Time_Delay_WifiSent	++;
			if(Time_Delay_WifiSent > 500)
			{
				Time_Delay_WifiSent = 0;	
			}
		}
		break;
	}
	/*if(mt_mqtt_GetNewFlag() == MQTT_REC_MESSAGE_NEW)
	{
		mt_mqtt_SetNewFlag(MQTT_REC_MESSAGE_FREE);
		return 0;
	}*/
	return 0xff;
}


////////AT+MQTTPUB=0,"38FFFFFF3032533551310743_up","AA0006290000082755",2,0
////////AT+MQTTPUB=0,\"
void mt_wifi_Mqtt_SentDat(unsigned char *buf)
{
	unsigned char mqttDataBuff[WIFI_MQTT_SENTDATASIZE_MAX];
	unsigned char i,idx,hchar,lchar;
	unsigned short lon;
	
	if((mt_wifi_get_wifi_Mqtt_state() == STA_MQTT_READY) && (mt_wifi_get_wifi_work_state() == ESP12_STA_DETEC_READY))
	{
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
		mqttDataBuff[idx ++] = '\"';
		lon = *buf<< 8; buf++;
		lon += *buf;  buf++;
		//lon -= 2;
		while(lon--)
		{
			hexToAsciiConversion(*buf,&hchar,&lchar);
			mqttDataBuff[idx++] = hchar;	
			mqttDataBuff[idx++] = lchar;
			buf ++;
			if(idx >= (WIFI_MQTT_SENTDATASIZE_MAX-6))
				break;
		}
		mqttDataBuff[idx ++] = '\"';
		mqttDataBuff[idx ++] = ',';	
		mqttDataBuff[idx ++] = '2';
		mqttDataBuff[idx ++] = ',';	
		mqttDataBuff[idx ++] = '0';
		mqttDataBuff[idx ++] = 0;
		mt_sendATToWifi(ESP12_AT_MQTTPUB,&mqttDataBuff[0]);
		mt_mqtt_SetNewFlag(MQTT_REC_MESSAGE_NEW);
	}
}



/////+MQTTSUBRECV:0,"rytwj01wwncy26A2",16,AA00072900123467
static unsigned char mt_wifi_MQTTRECVResposePro(unsigned char *p,unsigned char *RecDat)
{
		unsigned char *pdata;//,idx;
	  unsigned char len,lon,i;
	  lon = 0;
	  pdata = p;

		while(*pdata != '"') 
		{
			pdata++;
		}
		
	  pdata++;
		while(*pdata != '"') 
		{
			pdata++;
		}
	
		pdata++;  pdata++; 
	  len = 0;
	  i = 0;
	  while(*pdata != ',')
		{
			if((*pdata >= '0') && (*pdata <= '9'))
			{
				len *= 10;
				len += *pdata - '0';
			}
			pdata++;

			i++;
			if(i > 2)
				break;
		}
	  pdata++; //  
	  lon = len;
		while(len--)
		{
			*RecDat = *pdata;

			RecDat ++;
			pdata++;	
		}
		return lon;
}

//mt_protocol_WIFIMqttRecHandle
unsigned char mt_wifi_LinkState_rssi(void)
{
	 if(Esp12_link == ESP12_LINK_SUC)
	 {
		    return 3;
//				if(wifi_rssi < 56)
//				{
//						return 3;
//				}
//				else if(wifi_rssi < 69)
//				{
//						return 2;
//				}
//				else if(wifi_rssi < 80)
//				{
//						return 1;
//				}
//				else //if(wifi_rssi < 120)
//				{
//						return 0;
//				}									
	 }
	 else
		 return 0xff;
}

///MT_WIFI.C
void mt_wifi_exit_SmartConfig(void)
{
	unsigned char i;
	mt_sendATToWifi(ESP12_AT_CWSTOPSMART,&i);
	mt_wifi_changState(ESP12_STA_DETEC_STA);		
}

