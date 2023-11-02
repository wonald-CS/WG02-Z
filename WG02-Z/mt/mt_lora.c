#include "hal_uart.h"
#include "mt_lora.h"
#include "mt_api.h"
#include "os_system.h"

static volatile Queue256 LoraRxMsg;			//lora  
static Queue256 LoraTxMsg;

static void mt_lora_RxMsgInput(unsigned char dat);
static void mt_loraTx_Pro(void);
static void mt_loraRx_Pro(void);

loraRxDataPro_callback_t loracomm_callback;
loraRxDataApplyNet_callback_t loracommApplyNet_callback;

void mt_lora_init(void)
{
	QueueEmpty(LoraRxMsg);
	QueueEmpty(LoraTxMsg);
    loracomm_callback = 0;
    loracommApplyNet_callback = 0;
  hal_usart_Uart5DateRxCBSRegister(mt_lora_RxMsgInput);	
}

void mt_lora_Pro(void)
{
	mt_loraTx_Pro();
	mt_loraRx_Pro();
}

static void mt_lora_RxMsgInput(unsigned char dat)
{	
	unsigned char data;
	data = dat;
	QueueDataIn(LoraRxMsg,&data,1);
}

void mt_lora_TxMsgInput(unsigned char *dat,unsigned char len)
{	
	unsigned char *data;
	data = dat;
	if((QueueDataLen(LoraTxMsg) + len)< 256)
	{
		QueueDataIn(LoraTxMsg,data,len); 
	}
}

static void mt_loraTx_Pro(void)
{
	unsigned char len,i;
	unsigned char loraTxBuf[256];
	len = QueueDataLen(LoraTxMsg);
	if(len >0)
	{
		for(i=0;i<len;i++)
		{
			QueueDataOut(LoraTxMsg,&loraTxBuf[i]);
		}
		hal_usart_Uart5DateTx(&loraTxBuf[0],len);///??Lora?? ??		
	}	
}




void mt_lora_loracomm_callback_Register(loraRxDataPro_callback_t pCBS)
{
	if(loracomm_callback == 0)
	{
		loracomm_callback = pCBS;
	}
}

void mt_loraRxApplyNet_callback_Register(loraRxDataApplyNet_callback_t pCBS)
{
	if(loracommApplyNet_callback == 0)
	{
		loracommApplyNet_callback = pCBS;
	}
}

static unsigned char mt_lora_getSumCrc(unsigned char *buf,unsigned char len)
{
	unsigned char sumCheck,i;
	sumCheck = 0;
	for(i=0;i<len;i++)
	{
		sumCheck += *buf;
		buf ++;
	}
	return sumCheck;
}

///////////////////////////
static void mt_lora_TxDataDail(en_lora_eventTypedef lora_event,str_cmdApplyNet *loradat)
{
	unsigned char loratxbuffer[50];
	unsigned char idx;
	en_lora_eventTypedef event;
	event = lora_event;
	loratxbuffer[0] = 0xfe;
	idx  = 1;
	switch((unsigned char)event)
	{
		
		case LORA_COM_APPLY_NET:	
			loratxbuffer[idx ++] = 0x10;  // len
			loratxbuffer[idx ++] = event+0X80;// //功能码 申请入网
			loratxbuffer[idx ++] = loradat ->node[0];//网关分配的节点号1	
		    loratxbuffer[idx ++] = loradat ->node[1];////网关分配的节点号2

			loratxbuffer[idx ++] = loradat ->addr[0];//地址1
		    loratxbuffer[idx ++] = loradat ->addr[1];////地址2		
			loratxbuffer[idx ++] = loradat ->addr[2]; //地址3
			loratxbuffer[idx ++] = loradat ->addr[3]; //地址4
			loratxbuffer[idx ++] = loradat ->addr[4]; //地址5
			loratxbuffer[idx ++] = loradat ->addr[5]; //地址6
			loratxbuffer[idx ++] = loradat ->addr[6]; //地址7	
			loratxbuffer[idx ++] = loradat ->addr[7]; //地址8
			loratxbuffer[idx ++] = loradat ->addr[8]; //地址9
			loratxbuffer[idx ++] = loradat ->addr[9]; //地址10
			loratxbuffer[idx ++] = loradat ->addr[10]; //地址11
			loratxbuffer[idx ++] = loradat ->addr[11]; //地址12	
			loratxbuffer[idx ++] = loradat->detectorType; ////探测器类型	
			loratxbuffer[idx ++]  = mt_lora_getSumCrc(&loratxbuffer[2],(idx-2));
			mt_lora_TxMsgInput(loratxbuffer,idx);		
		break;		
		case LORA_COM_HEAT:///心跳包
		case LORA_COM_BAT_LOW://电池低压
		case LORA_COM_ALARM://系统报警
		case LORA_COM_TAMPER://防拆按键
		case LORA_COM_DISARM://系统撤防  遥控器	
		case LORA_COM_AWAYARM://离家布防
		case LORA_COM_HOMEARM://在家布防
		case LORA_COM_CLOSE://门磁开门
		case LORA_COM_RESPOSE_NONODE:  ///没有找到节点号	
			loratxbuffer[idx ++] = 0x03;  // len
		  if(event == LORA_COM_RESPOSE_NONODE)
			{
				loratxbuffer[idx ++] = LORA_COM_RESPOSE_NONODE;// //功能码 申请入网
			}
			else
			{
				loratxbuffer[idx ++] = (event+0X80);// //功能码 申请入网
			}
			loratxbuffer[idx ++] = loradat ->node[0];//网关分配的节点号1	
			loratxbuffer[idx ++] = loradat ->node[1];////网关分配的节点号2
			loratxbuffer[idx ++] = mt_lora_getSumCrc(&loratxbuffer[2],(idx-2));
			mt_lora_TxMsgInput(loratxbuffer,idx);				
		break;
	}
}

//static unsigned char mt_lora_getSumCrc(unsigned char *buf,unsigned char len)
//{
//	unsigned char sumCheck,i;
//	sumCheck = 0;
//	for(i=0;i<len;i++)
//	{
//		sumCheck += *buf;
//		buf ++;
//	}
//	return sumCheck;
//}

//////////////////////////////////////////////////////////////////////////////
//// Lora 有效数据处理函数
static void mt_lora_RxDataComm(unsigned char *buf,unsigned char len)
{
// 	en_lora_eventTypedef cmd;
	str_LoraAppNetState applynetsta;
    unsigned short crc16dat;
	unsigned char state;
	str_cmdApplyNet  LoraApplyNetDat;
	if(len > 1)
	{
	  	    LoraApplyNetDat.cmd = buf[0];
	  	    LoraApplyNetDat.node[0] = buf[1];
		    LoraApplyNetDat.node[1] = buf[2];
			switch((unsigned char)LoraApplyNetDat.cmd)
			{
				case LORA_COM_APPLY_NET:  //, ///申请入网
				{
					if(len == STR_COMAPPLYNET_SIZE)
					{
						 LoraApplyNetDat.addr[0] = buf[3];
						 LoraApplyNetDat.addr[1] = buf[4]; 
						 LoraApplyNetDat.addr[2] = buf[5];
						 LoraApplyNetDat.addr[3] = buf[6];						
						 LoraApplyNetDat.addr[4] = buf[7];
						 LoraApplyNetDat.addr[5] = buf[8];						
						 LoraApplyNetDat.addr[6] = buf[9];
						 LoraApplyNetDat.addr[7] = buf[10]; 
						 LoraApplyNetDat.addr[8] = buf[11];
						 LoraApplyNetDat.addr[9] = buf[12];						
						 LoraApplyNetDat.addr[10] = buf[13];
						 LoraApplyNetDat.addr[11] = buf[14];								
						 LoraApplyNetDat.detectorType = buf[15];
						 
						 crc16dat = mt_api_crc16(&LoraApplyNetDat.addr[0],12);
						 
						 if(crc16dat == ((LoraApplyNetDat.node[0]) | (LoraApplyNetDat.node[1] << 8)))
					   {////处理申请入网指令
								 if(loracommApplyNet_callback)
								 {
									 applynetsta = loracommApplyNet_callback(LORA_COM_APPLY_NET,LoraApplyNetDat);
									 if(applynetsta.state == LORADET_LEARN_OK)
									 {/////Study OK
										 mt_lora_TxDataDail(LORA_COM_APPLY_NET,&LoraApplyNetDat);	
									 }
									 
								}						 
						 }
					}
				}
				break;
				case LORA_COM_HEAT:  //,///心跳包
				case LORA_COM_BAT_LOW:  //,//电池低压
				case LORA_COM_ALARM:  //,  //系统报警
				case LORA_COM_TAMPER:  //, //防拆按键
				case LORA_COM_DISARM:  //, //系统撤防  遥控器
				case LORA_COM_AWAYARM:  //,  //离家布防
				case LORA_COM_HOMEARM:  //, //在家布防
				case LORA_COM_CLOSE:  //,   //门磁开门
				{
					if(len == 3)
					{
							if(loracomm_callback)
							{/////
								state = loracomm_callback((en_lora_eventTypedef)LoraApplyNetDat.cmd,LoraApplyNetDat);
								if(state == 0)
								{/// not node   没有找到节点号，需要重新入网 
								//	mt_lora_TxDataDail(LORA_COM_RESPOSE_NONODE,&LoraApplyNetDat);
								}
								else
								{
									mt_lora_TxDataDail((en_lora_eventTypedef)LoraApplyNetDat.cmd,&LoraApplyNetDat);	
								}
							}						
					}
				}
				break;
			}
	}
}

// 程序处理逻辑:
// 第一步：先判断帧头，并获取数据长度  条件：队列中至少有  2个字节
// 第二步：获取有效数据和校验位             条件：队列中的数据长度要满足大于等于len
// 第三步：数据校验                                  和校验
// 第四步：有效数据有效，处理数据。

static void mt_loraRx_Pro(void)
{
	static unsigned char len=0;
	static unsigned char itvDelayTime=0; //每帧数据接收时间限制
	unsigned char dat,CheckSum,i;
	static unsigned char loraRxBuf[100]; 
	//接收
	if(len == 0)
	{//// 空闲状态
		if(QueueDataLen(LoraRxMsg) >1)        //判断接收队列是否有数据
		{ 
			QueueDataOut(LoraRxMsg,&dat);//数据出列
			if(dat == 0xfe)                //判断帧头
			{
				QueueDataOut(LoraRxMsg,&len);  //出列数据长度
				itvDelayTime = 0;             
			}   
		} 
	}
	
	if(len > 0)     //判断数据长度大于0
	{///检测到帧头，等待有效数据
		if(QueueDataLen(LoraRxMsg) > len)     //判断是否接收完一帧
		{
			if(len >99)
			{
				//数据长度异常，丢弃数据
				QueueEmpty(LoraRxMsg);
				len = 0;
				return;
			}
			//数据正常，继续判断和校验
			CheckSum = 0;
			for(i=0; i<len; i++)
			{
				QueueDataOut(LoraRxMsg,&loraRxBuf[i]);//出列有效数据
				CheckSum += loraRxBuf[i];      //计算和校验
			}
			QueueDataOut(LoraRxMsg,&dat);     //出列和校验值
			if(dat == CheckSum)       //和校验匹配
			{
				mt_lora_RxDataComm(&loraRxBuf[0],len);
				USART1_PutInDebugString(&loraRxBuf[0],len);//Debug数据
			}
			len = 0;
		}
		else
		{ 
			itvDelayTime ++;
			if(itvDelayTime >= 10) //判断是否延时100ms没接收完一帧数据
			{
				//接收超时，丢弃本帧数据
				itvDelayTime = 0;
				len = 0;
			}    
		}
	}
}



