#include "stm32F10x.h"
#include "hal_GPIO.H"


#define WIFI_POWEREN_PORT			  GPIOC
#define WIFI_POWEREN_PIN 			  GPIO_Pin_4

#define EC200S_POWERKEY_PORT			GPIOA
#define EC200S_POWERKEY_PIN			  GPIO_Pin_5

static unsigned char hal_GPIO_GetACState(void);
static en_AcLinkSta AcState;// 外电的状态 静态全局变量

/////GPIO 初始化函数
void hal_GpioConfig_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA , ENABLE); 
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB , ENABLE); 
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC , ENABLE); 		
	
    GPIO_InitStructure.GPIO_Pin = WIFI_POWEREN_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_Init(WIFI_POWEREN_PORT, &GPIO_InitStructure);
	
    GPIO_InitStructure.GPIO_Pin = EC200S_POWERKEY_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_Init(EC200S_POWERKEY_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = CHECK_ACSTATE_PIN;     
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;   //输入悬空
	GPIO_Init(CHECK_ACSTATE_PORT, &GPIO_InitStructure);	
	
    AcState = (en_AcLinkSta)hal_GPIO_GetACState();  ///上电获取外电的状态   
	hal_GPIO_WIFIPowerEN_L();
	hal_GPIO_4GPowerKey_L();
}

// 获取PB1端口状态（高低电平）
static unsigned char hal_GPIO_GetACState(void)
{
	return (GPIO_ReadInputDataBit(CHECK_ACSTATE_PORT, GPIO_Pin_1));		
}
void delay_1msTest(void)
{
	unsigned int i=0;
	i = 7200;
	while(i--);   
}

////
en_AcLinkSta hal_Gpio_AcStateCheck(void)
{
	en_AcLinkSta state;
	static unsigned char times = 0;	//静态延时计数
	
	state = (en_AcLinkSta)hal_GPIO_GetACState();
	
	delay_1msTest();
	if(state == AcState)
	{////如果本次获取的状态和上次一样，则清零延时计数
		times	= 0;
	}
	else if(state != AcState)
  {///如果状态有变化
		times	++; //计数增加
		if(times > 20)
		{////如果不一样的状态计数超过20次  则更新 AcState 状态。
			times = 0;
			AcState = state;
		}
	}
	return AcState;///
}
///////////////////////////////////
void hal_GPIO_WIFIPowerEN_H(void)
{
	GPIO_ResetBits(WIFI_POWEREN_PORT,WIFI_POWEREN_PIN);	
}

void hal_GPIO_WIFIPowerEN_L(void)
{
	GPIO_SetBits(WIFI_POWEREN_PORT,WIFI_POWEREN_PIN);
}


void hal_GPIO_4GPowerKey_H(void)
{
	GPIO_SetBits(EC200S_POWERKEY_PORT,EC200S_POWERKEY_PIN);
}

void hal_GPIO_4GPowerKey_L(void)
{
	GPIO_ResetBits(EC200S_POWERKEY_PORT,EC200S_POWERKEY_PIN);
}


