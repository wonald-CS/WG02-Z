#include "mt_wifi.h"
#include "OS_System.h"
#include "hal_usart.h"
#include "hal_power.h"

volatile Queue1K  Wifi_RxIdxMsg;	



static void mt_wifi_RxMsgInput(unsigned char dat)
{	
	QueueDataIn(Wifi_RxIdxMsg,&dat,1);
}


void mt_wifi_RxHandle(void)
{
 	unsigned char len,i;
    unsigned char rxbuffer[20];
    len = QueueDataLen(Wifi_RxIdxMsg) ;
	if(len> 3)
	{//0D 0A
        if(len>=20)
            len = 20;
        for(i=0;i<len;i++)
        {
            QueueDataOut(Wifi_RxIdxMsg,&rxbuffer[i]);		
        }
	}
   
}




void mt_wifi_Init(void)
{
    QueueEmpty(Wifi_RxIdxMsg);
    hal_usart_Usart3DateRxCBSRegister(mt_wifi_RxMsgInput);
}


void mt_wifi_Config(void)
{
    mt_wifi_RxHandle();
}
