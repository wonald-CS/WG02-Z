#include "OS_System.h"
#include "hal_uart.h"
#include "mt_wifi.h"
#include "hal_Gpio.h"


volatile Queue1K  Wifi_RxIdxMsg;					    //

static void mt_wifi_RxMsgInput(unsigned char dat);
static void hal_WifiRx_Pro(void);

void mt_wifi_init(void)
{
		QueueEmpty(Wifi_RxIdxMsg);
    hal_usart_Uart3DateRxCBSRegister(mt_wifi_RxMsgInput);	
}

void mt_wifi_pro(void)
{
	hal_WifiRx_Pro();
}

static void mt_wifi_RxMsgInput(unsigned char dat)
{	
	QueueDataIn(Wifi_RxIdxMsg,&dat,1);
}

static void hal_WifiRx_Pro(void)
{
	unsigned char len,i;
	unsigned char rxbuffer[20];
	
  len = QueueDataLen(Wifi_RxIdxMsg);
	if(len> 3)
	{//0D 0A
		 if(len>=20)
				len = 20;
			for(i=0;i<len;i++)
			{
					QueueDataOut(Wifi_RxIdxMsg,&rxbuffer[i]);		
			}
      USART1_PutInDebugString(rxbuffer,len); 			
	}
}

//void mt_wifi_pro(void)
//{
//    hal_WifiRx_Pro();
//}









