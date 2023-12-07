#include "stm32F10x.h"
#include "string.h"
#include "hal_uart.h"
#include "OS_System.h"

#define DEBUG_TXBUFF_SIZE_MAX         512
unsigned char DebugTxDMAMapBuff[DEBUG_TXBUFF_SIZE_MAX];	
volatile unsigned char DebugIsBusy; 	////0 DMA空闲    1DMA 忙

#define DEBUG_TX_PORT	GPIOA
#define DEBUF_TX_PIN	        GPIO_Pin_9
#define DEBUG_RX_PORT	GPIOA
#define DEBUF_RX_PIN	        GPIO_Pin_10
#define DEBUG_USART_PORT	USART1   

#define GSM_TX_PORT			GPIOA
#define GSM_TX_PIN			GPIO_Pin_2
#define GSM_RX_PORT			GPIOA
#define GSM_RX_PIN    	GPIO_Pin_3
#define GSM_USART_PORT	USART2

#define WIFI_TX_PORT	GPIOB
#define WIFI_TX_PIN	GPIO_Pin_10
#define WIFI_RX_PORT	GPIOB
#define WIFI_RX_PIN	GPIO_Pin_11
#define WIFI_USART_PORT	USART3

#define LORA_TX_PORT	GPIOC
#define LORA_TX_PIN	GPIO_Pin_12
#define LORA_RX_PORT	GPIOD
#define LORA_RX_PIN	GPIO_Pin_2
#define LORA_USART_PORT	UART5

Uart2DateRx Uart2DateRxCBS;
Uart3DateRx Uart3DateRxCBS;
Uart5DateRx Uart5DateRxCBS;

volatile Queue512 DebugTxMsg;	

static void hal_uart1_DMA_DatSent(void);
static void hal_DMAC4_Enable(unsigned long size);

static void hal_usart_Config(void)
{ 
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure; 
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA,ENABLE);
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC,ENABLE);
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOD,ENABLE);

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1,ENABLE);
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART2,ENABLE);
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART3,ENABLE);
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_UART5,ENABLE);



/****************GPIO Setting *******************************************/
///  USART1_TX -> PA9 , 		
	GPIO_InitStructure.GPIO_Pin = DEBUF_TX_PIN;	         
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DEBUG_TX_PORT, &GPIO_InitStructure);		   
//  USART1_RX ->	PA10
	GPIO_InitStructure.GPIO_Pin = DEBUF_RX_PIN;	        
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_Init(DEBUG_RX_PORT, &GPIO_InitStructure);
	
 //// USART2_TX -> PA2 ,		
	GPIO_InitStructure.GPIO_Pin = GSM_TX_PIN;	         
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GSM_TX_PORT, &GPIO_InitStructure);		
	
 ////USART2_RX ->	PA3
	GPIO_InitStructure.GPIO_Pin = GSM_RX_PIN;	        
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_Init(GSM_RX_PORT, &GPIO_InitStructure);
 
///  USART3_TX -> PB10 ,			
	GPIO_InitStructure.GPIO_Pin = WIFI_TX_PIN;	         
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(WIFI_TX_PORT, &GPIO_InitStructure);	
	
/// USART3_RX ->	PB11
	GPIO_InitStructure.GPIO_Pin = WIFI_RX_PIN;	        
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_Init(WIFI_RX_PORT, &GPIO_InitStructure);
	
///  USART5_TX -> PC12 ,			
	GPIO_InitStructure.GPIO_Pin = LORA_TX_PIN;	         
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LORA_TX_PORT, &GPIO_InitStructure);	
	
/// USART5_RX ->	PD2
	GPIO_InitStructure.GPIO_Pin = LORA_RX_PIN;	        
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_Init(LORA_RX_PORT, &GPIO_InitStructure);


	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	
	USART_Init(DEBUG_USART_PORT, &USART_InitStructure); 
	USART_Init(GSM_USART_PORT, &USART_InitStructure); 
	USART_Init(WIFI_USART_PORT, &USART_InitStructure); 
    USART_Init(LORA_USART_PORT, &USART_InitStructure); 


	  //Usart1 NVIC  
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//
	NVIC_Init(&NVIC_InitStructure);	//
  
	//Usart2 NVIC 
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=4 ;	
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;			
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			
	NVIC_Init(&NVIC_InitStructure);	

    //Usart3 NVIC  
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=5 ;// 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		// 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			// 
	NVIC_Init(&NVIC_InitStructure);	// 

  
///UART5 NVIC
    NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=5 ;// 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;		// 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			// 
	NVIC_Init(&NVIC_InitStructure);	// 
	
	
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);// 
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);// 
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//		
	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);// 
	
	USART_Cmd(USART1, ENABLE);
	USART_Cmd(USART2, ENABLE);
	USART_Cmd(USART3, ENABLE);
	USART_Cmd(UART5, ENABLE);
	// 	
	USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE);
}


static void hal_DMA_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);// 
	DMA_DeInit(DMA1_Channel4);
	//USART1 
	DMA_InitStructure.DMA_PeripheralBaseAddr = (unsigned long)(&USART1->DR);	
	DMA_InitStructure.DMA_MemoryBaseAddr = (unsigned long)DebugTxDMAMapBuff;			//
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;						
	DMA_InitStructure.DMA_BufferSize = DEBUG_TXBUFF_SIZE_MAX;				
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	 //
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;				 
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;		 	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;		  	//
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;	   //
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;	  		//
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);		  	//
	DMA_ITConfig(DMA1_Channel4,DMA_IT_TC,ENABLE);		//

	//DAM1  中断配置
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);		
}

void hal_UsartInit()
{
	hal_usart_Config();
	hal_DMA_Config();	
	DebugIsBusy = 0;
	Uart2DateRxCBS = 0;
	Uart3DateRxCBS = 0;
	Uart5DateRxCBS = 0;
	QueueEmpty(DebugTxMsg);
}
////
void hal_UsartProc(void)
{		
	static unsigned short  usart2_4GTestDelay = 0;
	hal_uart1_DMA_DatSent();
	usart2_4GTestDelay  ++;
	if(usart2_4GTestDelay  > 299)
	{
		usart2_4GTestDelay = 0;
		//Hal_Uart3_Send_Data("AT\r\n",4);	
	} 
}






//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////DEBUG    USART1//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void hal_uart1_DMA_DatSent(void)
{
	unsigned int i,len;
    if(DebugIsBusy)
		return;
	
	len = QueueDataLen(DebugTxMsg);

	for(i=0; i<len; i++)
	{
	    QueueDataOut(DebugTxMsg,&DebugTxDMAMapBuff[i]);
	}
	if(len)
	{
		hal_DMAC4_Enable(len);
		DebugIsBusy = 1;
	} 
}
static void hal_DMAC4_Enable(unsigned long size)
{
	DMA1_Channel4->CCR &= ~(1<<0);
	DMA1_Channel4->CNDTR = size;
	DMA1_Channel4->CCR |= 1<<0;
}

//debug usart1
void DMA1_Channel4_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_IT_TC4) != RESET)
	{
		DMA_ClearITPendingBit(DMA1_IT_TC4);	
		DMA_Cmd(DMA1_Channel4, DISABLE);				 
		DebugIsBusy = 0;   ///DAM 空闲  
	}
}

void USART1_PutInDebugInfo(const char pData[])
{
	 unsigned char lon;
	 lon = (unsigned char )strlen(pData);
	 if((lon + QueueDataLen(DebugTxMsg)) < 512)
	 {
		QueueDataIn(DebugTxMsg,(unsigned char *)pData,lon);	
	 }
}

void USART1_PutInDebugString(unsigned char pData[],unsigned char len)
{
	 if((len + QueueDataLen(DebugTxMsg)) < 512)
	 {
			QueueDataIn(DebugTxMsg,(unsigned char *)pData,len);	
	 }
}

///////////////////////////////////
void USART1_IRQHandler(void)
{
    // unsigned char dat;
	// if(USART_GetITStatus(USART1,USART_IT_RXNE) != RESET)
	// {							
	// 	 dat = USART_ReceiveData(USART1);
	// 	 USART_ClearITPendingBit(USART1,USART_IT_RXNE);
	// 	 QueueDataIn(DebugTxMsg,&dat,1);		
	// }
    unsigned char dat;
	if(USART_GetITStatus(USART1,USART_IT_RXNE) != RESET)
	{							
		dat = USART_ReceiveData(USART1);
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
		#ifdef DEBUG_HAL_GSM
		USART_SendData(USART2,dat);
		#endif 
		// QueueDataIn(DebugTxMsg,&dat,1);		
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////4G    USART2/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//buf: sent data start addrs
//len: len of sentdatas
//static void Hal_Uart2_Send_Data(unsigned char  *buf,unsigned int  len)
//{
//	unsigned int  t;
//    for(t=0;t<len;t++)		 
//	{		   
//		while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);	  
//		USART_SendData(USART2,buf[t]);
//	}	 
//}
///

void hal_Usart2_SentString(unsigned char *buf)
{
	while(*buf)
	{
		USART_SendData(USART2,*buf);
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);	  
		buf  ++;
	}
}

void hal_usart_Uart2DateRxCBSRegister(Uart3DateRx pCBS)
{
	if(Uart2DateRxCBS == 0)
	{
		Uart2DateRxCBS = pCBS;
	}
}	


void USART2_IRQHandler(void)
{
	unsigned char dat;
	if(USART_GetITStatus(USART2,USART_IT_RXNE) != RESET)
	{		
		USART_ClearITPendingBit(USART2,USART_IT_RXNE);
		dat = USART_ReceiveData(USART2);	
		if(Uart2DateRxCBS)	
		{
			Uart2DateRxCBS(dat);
		}
		#ifdef DEBUG_HAL_GSM
		USART1_PutInDebugString(&dat,1);
		#endif 
	   
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////WIFI USART3//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////
//buf: sent data start addrs
//len: len of sentdatas
void Hal_Uart3_Send_Data(unsigned char  *buf,unsigned int  len)
{
	unsigned int  t;
     for(t=0;t<len;t++)		// 
	{		   
		while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);	  
		USART_SendData(USART3,buf[t]);
	}	 	
}

void hal_usart_Uart3DateRxCBSRegister(Uart3DateRx pCBS)
{
	if(Uart3DateRxCBS == 0)
	{
		Uart3DateRxCBS = pCBS;
	}
}


void USART3_IRQHandler(void)
{
	unsigned char dat;
	if(USART_GetITStatus(USART3,USART_IT_RXNE) != RESET)
	{							
		dat = USART_ReceiveData(USART3);
		USART_ClearITPendingBit(USART3,USART_IT_RXNE);
		////mt_wifi_RxMsgInput();
		if(Uart3DateRxCBS)
		{
			Uart3DateRxCBS(dat);
		}
		#ifdef DEBUG_HAL_WIFI
		USART1_PutInDebugString(&dat,1);
		#endif 
		
		
	}
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////LORA  UART5//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void hal_usart_Uart5DateTx(unsigned char *buf,unsigned char len)
{
		unsigned char lenx;
		lenx = len;
		while(lenx)
		{
			USART_SendData(UART5,*buf);
			while(USART_GetFlagStatus(UART5, USART_FLAG_TC) == RESET);	  
			buf++;
			lenx --;
		}
}



void hal_usart_Uart5DateRxCBSRegister(Uart5DateRx pCBS)
{
	if(Uart5DateRxCBS == 0)
	{
		Uart5DateRxCBS = pCBS;
	}
}	

void UART5_IRQHandler(void)
{
	unsigned char dat;
	if(USART_GetITStatus(UART5,USART_IT_RXNE) != RESET)
	{							
		dat = USART_ReceiveData(UART5);
		USART_ClearITPendingBit(UART5,USART_IT_RXNE);
		if(Uart5DateRxCBS)///unsigned char 
		{
			Uart5DateRxCBS(dat);
		} 
	}
}




