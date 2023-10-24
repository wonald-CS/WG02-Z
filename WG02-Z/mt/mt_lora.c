#include "mt_lora.h"
#include "hal_usart.h"
#include "OS_System.h"
#include "Algorithm.h"

static volatile Queue256 LoraRxMsg;			//lora 数据接收队列
static Queue256 LoraTxMsg;



loraRxDataPro_callback_t loracomm_callback;
loraRxDataApplyNet_callback_t loracommApplyNet_callback;

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


void mt_lora_TxMsgt(unsigned char *dat,unsigned char len)
{
	unsigned char *data;
	data = dat;
	if((QueueDataLen(LoraTxMsg) + len)< 256)
	{
		QueueDataIn(LoraTxMsg,data,len); 
	}
}

void mt_loraTx_Sent(Detector_EVENT cmd,Data_AppNet_Handle *pstRec_APPNet_DatHa)
{
    unsigned char kLoop,iLoop = 1;
    unsigned char Data[20];
    unsigned char SumCheck;

    SumCheck = 0;
    Data[0] = 0xFE;
    switch(pstRec_APPNet_DatHa->cmd){
        case Detector_AppNet:
            Data[iLoop++] = 0x10;
            Data[iLoop++] = pstRec_APPNet_DatHa->cmd + 0X80;
            Data[iLoop++] = pstRec_APPNet_DatHa->node[0];
            Data[iLoop++] = pstRec_APPNet_DatHa->node[1];
            for(kLoop=0; kLoop<12; kLoop++){
				Data[iLoop++] = pstRec_APPNet_DatHa->Macaddr[kLoop];    //读MAC地址
            }
            Data[iLoop++] = pstRec_APPNet_DatHa->detectorType;      	//探测器类型
            
			for(kLoop=0; kLoop<(App_Net_Len-3); kLoop++){
                SumCheck += Data[2+kLoop];
            }
            //SumCheck = SumCheck & 0xff;                                                 //取低8位
            Data[iLoop++] = SumCheck;

        break;
        

		case Detector_HeartBeat:          
		case Detector_LowBatrery:            
  		case Detector_SysAlert:            
  		case Detector_Disarm :          
  		case Detector_AwayArm:                 
  		case Detector_HomeArm:             
  		case Detector_DoorClose :
        
            Data[iLoop++] = 0x03;
            Data[iLoop++] = pstRec_APPNet_DatHa->cmd + 0X80;
            Data[iLoop++] = pstRec_APPNet_DatHa->node[0];
            Data[iLoop++] = pstRec_APPNet_DatHa->node[1];                                         
            for(kLoop=0; kLoop<(Else_Move_Len-3); kLoop++){
                SumCheck += Data[2+kLoop];
            }
            //SumCheck = SumCheck & 0xff;
            Data[iLoop++] = SumCheck;
			break;

		default:  
        break;
        
    }
   	mt_lora_TxMsgt(Data,iLoop);	
}




void mt_Det_TxDatePack(unsigned char *pNode,unsigned char len)
{
	str_LoraAppNetState applynetsta;
    unsigned short crc16dat;
	unsigned char state;
	unsigned char iLoop;
	Data_AppNet_Handle pstRec_APPNet_DatHa;


	if(len > 1)
	{
		pstRec_APPNet_DatHa.cmd = pNode[0];
		pstRec_APPNet_DatHa.node[0] = pNode[1];
		pstRec_APPNet_DatHa.node[1] = pNode[2];

		switch (pstRec_APPNet_DatHa.cmd)
		{
		case Detector_AppNet:
		{
			for (iLoop = 0; iLoop < 12; iLoop++)
			{
				pstRec_APPNet_DatHa.Macaddr[iLoop] = pNode [3 + iLoop];
			}
			pstRec_APPNet_DatHa.detectorType = pNode[15];	
						
			crc16dat = Algorithm_crc16(&pstRec_APPNet_DatHa.Macaddr[0],12);

			if(crc16dat == ((pstRec_APPNet_DatHa.node[0]) | (pstRec_APPNet_DatHa.node[1] << 8)))
			{
				if(loracommApplyNet_callback)
				{
					applynetsta = loracommApplyNet_callback(Detector_AppNet,pstRec_APPNet_DatHa);
					if(applynetsta.state == LORADET_LEARN_OK)
					{
						mt_loraTx_Sent(Detector_AppNet,&pstRec_APPNet_DatHa);	
					}
						
				}						 
			}
		}
			break;


		case Detector_HeartBeat:             
		case Detector_LowBatrery:           
		case Detector_SysAlert:              
		case Detector_Disarm :           	
		case Detector_AwayArm:                 
		case Detector_HomeArm:              
		case Detector_DoorClose:
		{
			if(loracomm_callback)
			{
				state = loracomm_callback((Detector_EVENT)pstRec_APPNet_DatHa.cmd,pstRec_APPNet_DatHa);
				if(state == 0)
				{/// not node   没有找到节点号，需要重新入网 
				//	mt_lora_TxDataDail(LORA_COM_RESPOSE_NONODE,&LoraApplyNetDat);
				}
				else
				{
					mt_loraTx_Sent((Detector_EVENT)pstRec_APPNet_DatHa.cmd,&pstRec_APPNet_DatHa);	
				}
			}
		}	
		break;
		}



	}
}



//Lora数据发送处理
void mt_lora_TxHandle(void)
{
	unsigned char len,i;
	unsigned char Lora_Tx_Queue[256];
	len = QueueDataLen(LoraTxMsg);
	if (len)
	{
		for (i = 0; i < len; i++)
		{
			QueueDataOut(LoraTxMsg,&Lora_Tx_Queue[i]);
		}
		
	}

	hal_lora_DataSent(&Lora_Tx_Queue[0],len);
	
}




//底层uart5 lora的RX接收到的数据就会传到 LoraRxMsg
void mt_lora_RxMsg(unsigned char dat)
{
	unsigned char data;
	data = dat;
	QueueDataIn(LoraRxMsg,&data,1);
}


//Lora数据接收处理
// 程序处理逻辑:
// 第一步：先判断帧头，并获取数据长度  条件：队列中至少有  2个字节
// 第二步：获取有效数据和校验位             条件：队列中的数据长度要满足大于等于len
// 第三步：数据校验                                  和校验
// 第四步：有效数据有效，处理数据。

void mt_lora_RxHandle(void)
{
    unsigned char i,Data_Sum = 0;
	static unsigned char DataLen = 0;
    unsigned char pstRecpNodef[30];
	static unsigned char Else_Move_Flag = 1;
    Data_AppNet_Receive *pstRec_APPNet_Data;
	Data_Else_Receive *pstRec_Else_Data;
    
	if(QueueDataLen(LoraRxMsg)){
		DataLen = QueueDataLen(LoraRxMsg);
		if (DataLen == Else_Move_Len)
		{
			if (Else_Move_Flag)
			{
				Else_Move_Flag = 0;
				return;
			}	
		}
		
		if ((DataLen == App_Net_Len) || (DataLen == Else_Move_Len))
		{
			Else_Move_Flag = 1;
			for(i=0;i<DataLen;i++)
			{
				QueueDataOut(LoraRxMsg,&pstRecpNodef[i]); 
			}
		
			//先这样写，后面看看能不能合并
			if(pstRecpNodef[0] == 0xFE)
			{
				switch (DataLen)
				{
				case Else_Move_Len:
					pstRec_Else_Data = (Data_Else_Receive*)pstRecpNodef;   
					for(i=0; i<(DataLen - 3); i++){
						Data_Sum += pstRec_Else_Data->Data[i];                   
					}	

					if(Data_Sum == pstRec_Else_Data->CheckSum){
						mt_Det_TxDatePack(pstRec_Else_Data->Data,3);    //有效数据3位
						USART1_PutInDebugString(&pstRec_Else_Data->Data[0],7);
					}else{
						QueueEmpty(LoraRxMsg);
					}   
					break;

				case App_Net_Len:
					pstRec_APPNet_Data = (Data_AppNet_Receive*)pstRecpNodef;
					for(i=0; i<(DataLen - 3); i++){
						Data_Sum += pstRec_APPNet_Data->Data[i];                   
					}

					if(Data_Sum == pstRec_APPNet_Data->CheckSum){
						mt_Det_TxDatePack(pstRec_APPNet_Data->Data,16);	//有效数据16位
						USART1_PutInDebugString(&pstRec_Else_Data->Data[0],20);
					}else{
						QueueEmpty(LoraRxMsg);
					}   
					break;
				
				default:
					break;
				}
			}else{
				QueueEmpty(LoraRxMsg);
			}
		}else{
				QueueEmpty(LoraRxMsg);
			}
		



	}
}







void mt_lora_Init(void)
{
	QueueEmpty(LoraRxMsg);
	QueueEmpty(LoraTxMsg);
    hal_usart_Uart5DateRxCBSRegister(mt_lora_RxMsg);
	loracomm_callback = 0;
	loracommApplyNet_callback = 0;
}


void mt_lora_Config(void)
{
    mt_lora_RxHandle();
	mt_lora_TxHandle();
}
