#include "OS_System.h"
#include "hal_uart.h"
#include "mt_wifi.h"
#include "hal_Gpio.h"
#include "string.h"
#include "mt_api.h"

///////注意：ESP8266只能配置2.4G频段的WIFI//////

volatile Queue1K  Wifi_RxIdxMsg;			
volatile Queue16  Wifi_TxIdxMsg;	

unsigned char WIFI_TxQueuePos;             		//下一条发送数据所在数组位置
unsigned char WIFI_TxBuff[WIFI_TX_QUEUE_SUM][WIFI_TX_BUFFSIZE_MAX];
unsigned char WIFI_RxBuff[WIFI_RXBUFFSIZE_MAX];


en_Esp12_sta WIFI_Sta;
en_WIFI_NetSta WIFI_NetSta;


const char ESP12_AT[ESP12_AT_MAX][ESP12_AT_LEN]=
{
	"AT+RST",
	"AT\0",                                     ///WIFI 测试指令
	"ATE1\0",                                   //打开回显
	"AT+CWMODE=1\0",    						//配置WIFI工作模式 				0:关闭WIFI   1:Station模式  2:softAP模式  3:softAP+ Station模式
	"AT+CWAUTOCONN=1\0",						//								0:上电不自动链接AP    1:上电自动链接AP
	"AT+CWSTARTSMART=2\0",						//启动某种类型的SmartConfig模式  1：ESP=TOUCH  2:AirKiss  3:AirKiss+Esptouch
	"AT+CWSTOPSMART\0",							//停止SmartConfig	
	"AT+CWSTATE?\0",      						//获取WIFI 的链接状态        	 
	"AT+CWLAP=\"\0",							//获取WIFI的信号强弱

	"AT+MQTTUSERCFG=0,1,\"",  					//MQTT CONFESP12_AT_MQTTUSERCFG
	"AT+MQTTCONN=0,\"",    						//MQTT CONNESP12_AT_MQTTCONN,
	"AT+MQTTPUB=0,\"",     						
	"AT+MQTTSUB=0,\"",     						
	"AT+MQTTCLEAN=0",      						
};


const unsigned char ESP12_AT_ResPonse[ESP12_AT_RESPONSE_MAX][27]=
{
	"WIFI CONNECTED\0",   						//WIFI已连接
	"WIFI DISCONNECT\0",						//WIFI未连接
	"+CWSTATE:\0",    							//获取WIFI的链接状态	
												//0: ESP32 station 尚未进行任何 Wi-Fi 连接
												//1: ESP32 station 已经连接上 AP，但尚未获取到 IPv4 地址
												//2: ESP32 station 已经连接上 AP，并已经获取到 IPv4 地址
												//3: ESP32 station 正在进行 Wi-Fi 连接或 Wi-Fi 重连
												//4: ESP32 station 处于 Wi-Fi 断开状态
	"+CWJAP:\0",								//获取WIFI的信号值
	"ERROR\0",           						//WIFI模块返回值	
	"Smart get wifi info\0",					//配置WIFI的名称和密码
	"smartconfig connected wifi\0",     		//配网成功
	"+CWLAP:\0",
	"+MQTTCONNECTED:0\0",
	"+MQTTDISCONNECTED:0\0",
	"+MQTTSUBRECV:0,\0",
	"OK\r\n\0",           						//WIFI模块返回值OK
};


/*******************************************************************************************
*@description:WIFI状态改变
*@param[in]：sta：改变的枚举值
*@return：无
*@others：
********************************************************************************************/
void mt_wifi_changState(en_Esp12_sta sta)
{
	WIFI_Sta = sta;
	QueueEmpty(Wifi_RxIdxMsg);
}


/*******************************************************************************************
*@description:WIFI联网状态和WIFI名称获取
*@param[in]：*p：WIFI_RxBuff
*@return：无
*@others：
********************************************************************************************/
static void mt_wifi_CwstateResposePro(unsigned char *p)
{
	unsigned char *pdata;
	unsigned char i = 0;

	pdata = p;
	//0x32   0 1 2 3 4 
	//+CWSTATE:2,"Guest"
	while(*pdata != ':') 
	{
		pdata++;	
	}
	pdata++;

	WIFI_NetSta.WIFI_Net_Sta = *pdata - 0x30;

	while(*pdata != '"')
	{
		pdata++;	
	}			
	pdata++;

	//记录WIFI名称
	while(*pdata != '"') 
	{	
		WIFI_NetSta.WIFI_SSid[i] = *pdata;
		i++;
		pdata++;
		
	}
}



/*******************************************************************************************
*@description:WIFI模块外电或电源接入
*@param[in]：fuc：复位或者空闲
*@return：无
*@others： 1. 如果没电断电的情况i下， 关闭WIFI电源   2. 电源复位操作
********************************************************************************************/
static unsigned char mt_wifi_PowerManage(en_wifiPowerManageSta fuc)
{
	static unsigned short StartTime = 2000;
	if(hal_Gpio_AcStateCheck() == STA_AC_LINK)
	{///USB 外部供电的情况下 
		if(fuc == STA_WIFI_POWER_RESET)
		{//Close WIFI Power
			StartTime = 0;	
			mt_wifi_changState(ESP12_STA_WIFI_POWER);
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
			{
				StartTime = 2000;
				hal_GPIO_WIFIPowerEN_L();
			}
			return 0;
		}	
		return 1;
	}
	else
	{// 电池供电  关闭WIFI 功能
		StartTime = 0;
		mt_wifi_changState(ESP12_STA_WIFI_POWER);
		hal_GPIO_WIFIPowerEN_H();
		return 0;
	}
}



/*******************************************************************************************
*@description:WIFI数据入列
*@param[in]：
*@return：无
*@others：
********************************************************************************************/
static void mt_wifi_RxMsgInput(unsigned char dat)
{	
	QueueDataIn(Wifi_RxIdxMsg,&dat,1);
}


/*******************************************************************************************
*@description:缓存数组存入二维数组
*@param[in]：*pdata：缓存数组
*@return：无
*@others：
********************************************************************************************/
void WIFI_TxMsgInput(unsigned char *pData)
{
	unsigned char i,len;
	len = pData[0];
	for (i = 0; i < len; i++)
	{
		WIFI_TxBuff[WIFI_TxQueuePos][i] = pData[i];
	}

	QueueDataIn(Wifi_TxIdxMsg,&WIFI_TxQueuePos,1);   //记录该指令的位置，存入队列
	WIFI_TxQueuePos++;
	if(WIFI_TxQueuePos >= WIFI_TX_QUEUE_SUM)
	{
		WIFI_TxQueuePos = 0;
	}
}


/*******************************************************************************************
*@description:AT指令数据组包存入缓存数组
*@param[in]：cmd:AT指令，*pdata：AT指令需要传入的数字（如0、1、2）
*@return：1：成功，0：失败
*@others：AT指令最后两位需要传入0X0D和0X0A（换行）
********************************************************************************************/
void mt_wifi_DataPack(unsigned char cmd,unsigned char *pdata)
{
	unsigned char DataPack_Array[WIFI_TX_BUFFSIZE_MAX];
	unsigned i;

	if (cmd < ESP12_AT_MAX)
	{//前1位为长度，后面为AT指令，最后2位为换行
		for (i = 0; i < ESP12_AT_LEN; i++)
		{
			if (ESP12_AT[cmd][i] != 0)
			{
				DataPack_Array[1+i] = ESP12_AT[cmd][i];
			}else
			{
				if(*pdata != 0xff)
				{
					if(cmd == ESP12_AT_CWLAP)
					{
						while(*pdata != 0xff)
						{
							DataPack_Array[1+i]=*pdata;	
							i++;
							pdata++;
						}
						
					}else{
						while(*pdata != 0)
						{
							DataPack_Array[1+i]=*pdata;	
							i++;
							pdata++;
						}	
					}
				}
				
				DataPack_Array[i+1]=0x0D;	
				i++;
				DataPack_Array[i+1]=0x0A;	
				i++;	
				DataPack_Array[0] = i+1;
				WIFI_TxMsgInput(DataPack_Array);
				break;
			}
		}			
	}
		

}


/*******************************************************************************************
*@description:通过串口向ESP8266发送二维数组的数据
*@param[in]：*pdata：二维数组
*@return：无
*@others：
********************************************************************************************/
void WIFI_TxMsg_Send(unsigned char *pData)
{
	unsigned char WIFITxDataBuff[WIFI_TX_BUFFSIZE_MAX];	
	unsigned char i,SendBuff_len;
	SendBuff_len = pData[0];
	SendBuff_len -= 1;
	for (i = 0; i < SendBuff_len; i++)
	{
		WIFITxDataBuff[i] = pData[i+1];
	}

	Hal_Uart3_Send_Data(WIFITxDataBuff,SendBuff_len);
}



/*******************************************************************************************
*@description:解析通过串口接收ESP8266的应答数据
*@param[in]：*pData：WIFI_RxBuff[0],  *res:接收的数据的枚举值,  *pIdx:字符串相同的起始下标,  num:接收数据长度
*@return：0：匹配成功； 0xff：匹配失败；
*@others：
********************************************************************************************/
unsigned char WIFI_RxMsg_Analysis(unsigned char *pData,unsigned char *res,unsigned char *pIdx,unsigned short num)
{
	unsigned char i,j;
	for (j = 0; j < ESP12_AT_RESPONSE_MAX; j++)
	{
		i = SeekSrting(pData,(unsigned char*)ESP12_AT_ResPonse[j],num);
		if (i != 0xff)
		{
			*res = j;
			*pIdx = i;
			return 0;
		}
		
	}
	return 0xff;
}


/*******************************************************************************************
*@description:处理已解析完成的接收数据，执行相应动作
*@param[in]：*pData：WIFI_RxBuff[0],  res:接收的数据的枚举值, strlon：接收数据长度
*@return：
*@others：
********************************************************************************************/
static void Wifi_Rx_Response_Handle(unsigned char *pData,en_esp12_atResponse res,unsigned short strlon)
{
	switch((unsigned char)res)
	{
		case ESP12_AT_RESPONSE_WIFI_CONNECTED:
		{

		}
		break;

		case ESP12_AT_RESPONSE_WIFI_DISCONNECT:
		{

		}
		break;

		case ESP12_AT_RESPONSE_CWSTATE:
		{
			memset(WIFI_NetSta.WIFI_SSid, 0, sizeof(WIFI_NetSta.WIFI_SSid));
			mt_wifi_CwstateResposePro(pData);
			if (WIFI_NetSta.WIFI_Net_Sta == ESP12_LINK_SUC)
			{
				mt_wifi_changState(ESP12_STA_DETEC_READY);
			}else
			{
				mt_wifi_changState(ESP12_STA_WIFISTA);
			}
			
			
		}
		break;

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
			if(WIFI_Sta == ESP12_STA_WIFI_POWER)
			{
				mt_wifi_changState(ESP12_STA_INIT);
			}
			
		}
		break; 

		case ESP12_AT_RESPONSE_CWLAP:
		{
		
		}
		break;

		case ESP12_AT_RESPONSE_SMART_GET_WIFIWINFO:
		{//获取WIFI名称和密码
		
		}
		break;

		case ESP12_AT_RESPONSE_SMART_SUC:
		{  //配网成功
			if (WIFI_Sta == ESP12_STA_WIFIConfig_Wait)
			{
				mt_wifi_changState(ESP12_STA_GETING_SUC);
			}			
		}
		break;	

		case ESP12_AT_RESPONSE_MQTTCONN://	"+MQTTCONNECTED:0\0",
		{
			
		}
		break;

		case ESP12_AT_RESPONSE_MQTTDISCONN:
		{
		}
		break;

		case ESP12_AT_RESPONSE_MQTTRECV://"+MQTTSUBRECV:0,\0",
		{
			  
		}
		break;
	}
}


//接收测试函数
static void hal_WifiRx_Pro(void)
{
//	unsigned char len,i;
//	unsigned char rxbuffer[20];
//	
//	len = QueueDataLen(Wifi_RxIdxMsg);
//	if(len> 3)
//	{
//		if(len>=20)
//		len = 20;
//		for(i=0;i<len;i++)
//		{
//			QueueDataOut(Wifi_RxIdxMsg,&rxbuffer[i]);		
//		}
//		USART1_PutInDebugString(rxbuffer,len); 			
//	}
	
	en_esp12_atResponse response;
	static unsigned short Rx_Data_len = 0;	
	unsigned char StrAddr,Ret;
	while (QueueDataLen(Wifi_RxIdxMsg) > 1)
	{
		//非MQTT指令
		if(Rx_Data_len >= (WIFI_RXBUFFSIZE_MAX-5))
		{//防止移除  
				Rx_Data_len = 0;
				return;
		}	

		QueueDataOut(Wifi_RxIdxMsg,&WIFI_RxBuff[Rx_Data_len++]);
		if(WIFI_RxBuff[Rx_Data_len - 1] == 0x0D)
		{
			WIFI_RxBuff[Rx_Data_len++] = 0x0A;
			
			//如OK回车换行：0x79 0x75 0x0D 0x0A     rxbufferIDX = 4
			//接收数据最低长度为4？ OK\n\r
			if (Rx_Data_len > 2)
			{	//response通过指针，从函数里面赋值
				Ret = WIFI_RxMsg_Analysis(&WIFI_RxBuff[0],&response,&StrAddr,Rx_Data_len);
				if (Ret == 0)
				{
					Wifi_Rx_Response_Handle(&WIFI_RxBuff[0],response,Rx_Data_len);
				}
				
			}
			memset(&WIFI_RxBuff[0], 0, sizeof(WIFI_RXBUFFSIZE_MAX));	
			Rx_Data_len = 0;
		}


		
	}
}



//发送测试函数
static void hal_WifiTx_Pro(void)
{
	unsigned char Idx,i;
	static unsigned int Time_Delay_WifiSent = 0; ////发送AT指令间隔延时时间
	static unsigned int Time_Delay_WifiSta = 0;


	//发送
	if(QueueDataLen(Wifi_TxIdxMsg))
	{
		Time_Delay_WifiSent ++;
		if(Time_Delay_WifiSent > 10)
		{//WIFI 指令发送间隔时间 100秒
			Time_Delay_WifiSent = 0;				
			QueueDataOut(Wifi_TxIdxMsg,&Idx);     //读出队列数字，发送对应位置数据
			WIFI_TxMsg_Send(&WIFI_TxBuff[Idx][0]);
		}
	}

	switch (WIFI_Sta)
	{
		case ESP12_STA_RESET:
		{
			Time_Delay_WifiSta ++;
			if(Time_Delay_WifiSta > 200)
			{
				i = 0xFF;
				Time_Delay_WifiSta = 0;
				mt_wifi_DataPack(ESP12_STA_RESET,&i);
				mt_wifi_changState(ESP12_STA_WIFI_POWER);
			}
		}
			break;

		case ESP12_STA_WIFI_POWER:
		{
			Time_Delay_WifiSta ++;
			if (Time_Delay_WifiSta > 300)
			{
				Time_Delay_WifiSta = 0;
				i = 0xff;
				mt_wifi_DataPack(ESP12_AT_AT,&i);
				//此处在接收到ATOK再作初始化处理
				//WIFI_Sta = ESP12_STA_INIT;				
			}
		}
			break;


		case ESP12_STA_INIT:
		{
			Time_Delay_WifiSta ++;
			if (Time_Delay_WifiSta > 200)
			{
				i = 0xff;
				mt_wifi_DataPack(ESP12_AT_ATE,&i);
				mt_wifi_DataPack(ESP12_AT_CWMODE,&i);
				mt_wifi_DataPack(ESP12_AT_CWAUTOCONN,&i);
				mt_wifi_changState(ESP12_STA_WIFISTA);
			}
		}
			break;

		case ESP12_STA_WIFISTA:
		{
			static unsigned char WIFI_Sta_Count = 0;
			Time_Delay_WifiSta ++;
			if (Time_Delay_WifiSta > 300)
			{
				WIFI_Sta_Count++;
				Time_Delay_WifiSta = 0;
				i = 0xff;
				mt_wifi_DataPack(ESP12_AT_CWSTATE,&i);
				if (WIFI_Sta_Count > 5)
				{
					WIFI_Sta_Count = 0;	
					mt_wifi_PowerManage(STA_WIFI_POWER_RESET);					
					//mt_wifi_DataPack(ESP12_AT_RESET,&i);	
					//WIFI_Sta = ESP12_STA_RESET;
				}
			}
		}
			break;

		case ESP12_STA_WIFIConfig:
		{
			QueueEmpty(Wifi_RxIdxMsg);
			i = 0xff;
			mt_wifi_DataPack(ESP12_AT_CWSTOPSMART,&i);
			mt_wifi_DataPack(ESP12_AT_CWSTARTSMART,&i);
			mt_wifi_changState(ESP12_STA_WIFIConfig_Wait);
		}
			break;

		case ESP12_STA_WIFIConfig_Wait:    	
		{
			Time_Delay_WifiSta++;
			if (Time_Delay_WifiSta > 6000)
			{
				mt_wifi_changState(ESP12_STA_GETING_FAIL);
			}			
		}
			break;

		case ESP12_STA_GETING_FAIL:
		case ESP12_STA_GETING_SUC:
		{//无论配网成功还是失败，都要发送CWSTOPSMART来节省模块的资源
		 //等待100MS再重新获取WIFI联网状态
			static unsigned char Get_FailORSuc = 0;
			Get_FailORSuc++;
			if (Get_FailORSuc > 10)
			{
				QueueEmpty(Wifi_RxIdxMsg);
				Get_FailORSuc = 0;
				Time_Delay_WifiSta = 0;
				i = 0xff;
				mt_wifi_DataPack(ESP12_AT_CWSTOPSMART,&i);
				mt_wifi_changState(ESP12_STA_WIFISTA);
			}
			
			
		}
			break;

		case ESP12_STA_DETEC_READY:
		{
			
		}
			break;
		
		case ESP12_STA_UP_ALARMDAT:
		{
			
		}
			break;
	}

}



void mt_wifi_init(void)
{
	unsigned char i;
	QueueEmpty(Wifi_RxIdxMsg);
	QueueEmpty(Wifi_TxIdxMsg);
  	hal_usart_Uart3DateRxCBSRegister(mt_wifi_RxMsgInput);
	WIFI_TxQueuePos = 0;
	for(i=0; i<WIFI_TX_QUEUE_SUM; i++)
	{
		memset(&WIFI_TxBuff[i], 0, sizeof(WIFI_TxBuff[i]));
	}	
	memset(&WIFI_RxBuff[0], 0, sizeof(WIFI_RXBUFFSIZE_MAX));

	WIFI_Sta = ESP12_STA_WIFI_POWER;	
	WIFI_NetSta.WIFI_Net_Sta = ESP12_LINK_FAIL;
	memset(WIFI_NetSta.WIFI_SSid, 0, sizeof(WIFI_NetSta.WIFI_SSid));
	
}


void mt_wifi_pro(void)
{
	if(mt_wifi_PowerManage(STA_WIFI_POWER_IDLE))
	{
		hal_WifiRx_Pro();
		hal_WifiTx_Pro();
	}
}









