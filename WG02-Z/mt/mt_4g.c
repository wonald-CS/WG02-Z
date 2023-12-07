#include "mt_4g.h"
#include "mt_api.h"
#include "hal_GPIO.H"
#include "OS_System.h"
#include "hal_uart.h"
#include "string.h"


const unsigned char GSM_AT[EC200_AT_MAX][70]=
{
	////初始化部分
	"AT+CPIN?\0",
	"ATE1\0",												//1 回显关闭
	"AT+IPR=115200\0",										//2 设置波特率为115200		
	"AT+QIDEACT=1\0",
	"AT+QIACT=1\0",
	"AT+CREG?\0",
	"AT+CGREG?\0",
	/*****/
	"AT+GSN\0",	
	"AT+CIMI\0",		
	//// 短信初始化
	"AT+CSCA?\0",											//1 查询短信中心号码
	"AT+CMGF=1\0",											//配置短信设置的方式	
	
	#ifdef ___SENTSMS_UCS2		
	"AT+CSCS=\"UCS2\"\0",									//配置短信设置的方式		 
	"AT+CSMP=49,167,0,25\0",								//配置短信设置的方式	
	"AT+CNMI=2,1,0,1,0\0",	
	#else
  	"AT+CSCS=\"GSM\"\0",									//配置短信设置的方式	
	//"AT+CSMP=49,167,0,241\0",								//配置短信设置的方式	
	"AT+CSMP=17,167,0,0\0",									//配置短信设置的方式	
	"AT+CNMI=2,1,0,0,0\0",
	#endif

	"AT+CLVL=5\0",											//33 扬声器等级最低 
 	"AT+CLVL=0\0", 
	//MQTT 初始化部分
	//MQTT通讯部分
	"AT+QMTCFG=\"version\",0,4\0",
	"AT+QMTCFG=\"recv/mode\",0,0,1",
	"AT+QMTCFG=\"send/mode\",0,0",	
	
	"AT+CSQ\0",	
	"AT+QMTOPEN=0,\"",										//119.91.158.8\",1883",
	"AT+QMTCONN=0,\"",										//rytwj01wwncy26A\",\"admin\",\"7d19d4f675ea0b05553a953bbd86b041\"",
	"AT+QMTSUB=0,1,\"",										//NewFirmware_down\",0",
	"AT+QMTPUBEX=0,0,0,0,\"",								//rytwj01wwncy26A_up\",6",
    
  	//发送短信部分
   	"AT+CMGS=\0",
	 
	//报警拨打电话部分
	"ATD\0", 												//15 拨打电话 例如：ATD12345678902;
	"AT+CLCC\0",											//16 查询当前呼叫
	"AT+CPAS\0",
	"ATH\0",				   								//17 挂机
	"ATA\0",  		 										//18 来电收到RING ,接通电话。  
	
	
	
	"AT+QAUDMOD=2\0",										//34 麦克风 0-主通道,0-增益等级		
	"AT+CMUT=1\0",											//35 MIC静音打开
	"AT+QTONEDET=1\0",										//38 打开DTMF检测
	"AT+QWDTMF=7,0,\"8,100,100,8,100,100\"\0",				//39 短嘀2声
};


volatile Queue16  GSM_TxIdxMsg;	

unsigned char GSM_TxQueuePos;             	
unsigned char GSM_TxBuff[GSM_TX_QUEUE_SUM][GSM_TX_BUFFSIZE_MAX];

EC200C_variable  ES200C_Var;




/*******************************************************************************************
*@description:缓存数组存入二维数组
*@param[in]：*pdata：缓存数组
*@return：无
*@others：
********************************************************************************************/
void GSM_TxMsgInput(unsigned char *pData)
{
	unsigned char i,len;
	len = pData[0];
	for (i = 0; i < len; i++)
	{
		GSM_TxBuff[GSM_TxQueuePos][i] = pData[i];
	}

	QueueDataIn(GSM_TxIdxMsg,&GSM_TxQueuePos,1);   //记录该指令的位置，存入队列
	GSM_TxQueuePos++;
	if(GSM_TxQueuePos >= GSM_TX_QUEUE_SUM)
	{
		GSM_TxQueuePos = 0;
	}
}


/*******************************************************************************************
*@description:AT指令数据组包存入缓存数组
*@param[in]：cmd:AT指令，*pdata：AT指令需要传入的数字（如0、1、2）
*@return：1：成功，0：失败
*@others：AT指令最后两位需要传入0X0D和0X0A（换行）
********************************************************************************************/
void mt_GSM_DataPack(unsigned char cmd,unsigned char *pdata)
{
	unsigned char DataPack_Array[GSM_TX_BUFFSIZE_MAX];
	unsigned i;

	if (cmd < EC200_AT_MAX)
	{//前1位为长度，后面为AT指令，最后2位为换行
		for (i = 0; i < GSM_TX_BUFFSIZE_MAX; i++)
		{
			if (GSM_AT[cmd][i] != 0)
			{
				DataPack_Array[1+i] = GSM_AT[cmd][i];
			}else
			{
				while(*pdata != 0)
				{
					DataPack_Array[1+i]=*pdata;	
					i++;
					pdata++;
				}	
				
				DataPack_Array[i+1]=0x0D;	
				i++;
				DataPack_Array[i+1]=0x0A;	
				i++;	
				DataPack_Array[0] = i+1;
				GSM_TxMsgInput(DataPack_Array);
				break;
			}
		}			
	}
		

}


/*******************************************************************************************
*@description:通过串口向4g发送二维数组的数据
*@param[in]：*pdata：二维数组
*@return：无
*@others：
********************************************************************************************/
void GSM_TxMsg_Send(unsigned char *pData)
{
	unsigned char GSMTxDataBuff[GSM_TX_BUFFSIZE_MAX];	
	unsigned char i,SendBuff_len;
	SendBuff_len = pData[0];			//二维数组首位为数据长度
	SendBuff_len -= 1;
	for (i = 0; i < SendBuff_len; i++)
	{
		GSMTxDataBuff[i] = pData[i+1];	//AT指令数据
	}

	hal_Usart2_SentString(&GSMTxDataBuff[0]);
}


/*******************************************************************************************
*@description:4G模块发送函数
*@param[in]：
*@return：
*@others：
********************************************************************************************/
static void Mt_GSMTx_Pro(void)
{
	unsigned char Idx;
	static unsigned int Time_Delay_GSMSent = 0; ////发送AT指令间隔延时时间


	//发送
	if(QueueDataLen(GSM_TxIdxMsg))
	{
		Time_Delay_GSMSent ++;
		if(Time_Delay_GSMSent > 15)
		{//WIFI 指令发送间隔时间 100秒
			Time_Delay_GSMSent = 0;				
			QueueDataOut(GSM_TxIdxMsg,&Idx);     //读出队列数字，发送对应位置数据
			GSM_TxMsg_Send(&GSM_TxBuff[Idx][0]);
		}
	}
}


/*******************************************************************************************
*@description:功能:4G模块接收函数
*@param[in]：
*@return：
*@others：
********************************************************************************************/
static void Mt_GSMRx_Pro(void)
{

}


/*******************************************************************************************
*@description:4G发送指令函数
*@param[in]:
*@return：无
*@others：
********************************************************************************************/
static void ES200C_ApplicationTxd_ManagementFuction(void)
{
	unsigned char i;
	static unsigned short timeDelay = 0;
	timeDelay++;					
	if(timeDelay > 300)		//延时3秒	
	{
		timeDelay= 0;
	    i = 0;
		mt_GSM_DataPack(EC200_AT_INIT_ATE0,&i);	
		mt_GSM_DataPack(EC200_AT_INIT_IPR,&i);	
		mt_GSM_DataPack(EC200_AT_QIDEACT,&i);	
		mt_GSM_DataPack(EC200_AT_QIACT,&i);	
		//mt_sendATToGSM(EC200_AT_ATE_CPIN,&i);				
	}
}


/*******************************************************************************************
*@description:功能:4G模块开机任务函数
*@param[in]：
*@return：
*@others：
********************************************************************************************/
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


void mt_4g_Init(void)
{
    ES200C_Var.powerKeytime = 0;
	GSM_TxQueuePos = 0;
	QueueEmpty(GSM_TxIdxMsg);
	memset(&GSM_TxBuff[0], 0, sizeof(GSM_TX_BUFFSIZE_MAX));
    hal_GPIO_4GPowerKey_L(); 
}


void mt_4g_pro(void)
{
    EC200S_PutOnHandler();
	Mt_GSMTx_Pro();
	//Mt_GSMRx_Pro();
	ES200C_ApplicationTxd_ManagementFuction();
}
















