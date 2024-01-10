#include "hal_gotoapp.h"
#include "stm32f10x.h"

typedef  void (*iapfun)(void);	

iapfun JumpApp;

__asm void MSR_MSP(u32 addr) 
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}
////

static void Driver_CloseAllInt(void)
{
	TIM_DeInit(TIM4);  
	TIM_Cmd(TIM4, DISABLE);

	USART_DeInit(USART1);  
	USART_DeInit(USART2);
	USART_DeInit(USART3);
	USART_DeInit(UART4);
	USART_DeInit(UART5);
	
	USART_Cmd(USART1,DISABLE);  
	USART_Cmd(USART2,DISABLE);
	USART_Cmd(USART3,DISABLE);
	USART_Cmd(UART4,DISABLE);
	USART_Cmd(UART5,DISABLE);

	SPI_I2S_DeInit(SPI2);
	SPI_I2S_DeInit(SPI3);

	SPI_Cmd(SPI2, DISABLE); 	
	SPI_Cmd(SPI3, DISABLE);

	RCC_RTCCLKCmd(DISABLE); 
	__disable_irq();	
}

void GotoApp(void)
{///
	u32 JumpAddr; 
	//u32 armAddr;
	//armAddr = *(vu32*)APP_FLASH_ADDR;
	////#define APP_FLASH_ADDR	 0x0800C800    200081C0
	if(((*(vu32*)APP_FLASH_ADDR)&0x2FFF0000)==0x20000000)	 
	{ ////���ջ����ַ�Ƿ�Ϸ�.  0x2000 0000 - 0x2000 FFFF
		//�ж��û��Ƿ��Ѿ����س�����Ϊ��������´˵�ַ��ջ��ַ��
        //��û����һ��Ļ�����ʹû�����س���Ҳ�����������ܷɡ�
		Driver_CloseAllInt();///�ر����д򿪵�����
		JumpAddr = *(vu32*)(APP_FLASH_ADDR+4);///�û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)
		
		JumpApp = (iapfun)JumpAddr;
		
		MSR_MSP(*(vu32*)APP_FLASH_ADDR); //��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)			
		JumpApp();	//��ת��APP.						 
	}///
}







