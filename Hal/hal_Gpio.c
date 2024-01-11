#include "stm32F10x.h"
#include "hal_GPIO.H"


#define WIFI_POWEREN_PORT			  GPIOC
#define WIFI_POWEREN_PIN 			  GPIO_Pin_4

#define EC200S_POWERKEY_PORT			GPIOA
#define EC200S_POWERKEY_PIN			  GPIO_Pin_5

static unsigned char hal_GPIO_GetACState(void);
static en_AcLinkSta AcState;// ����״̬ ��̬ȫ�ֱ���

/////GPIO ��ʼ������
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
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;   //��������
	GPIO_Init(CHECK_ACSTATE_PORT, &GPIO_InitStructure);	
	
    AcState = (en_AcLinkSta)hal_GPIO_GetACState();  ///�ϵ��ȡ����״̬   
	hal_GPIO_WIFIPowerEN_L();
	hal_GPIO_4GPowerKey_L();
}

// ��ȡPB1�˿�״̬���ߵ͵�ƽ��
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
	static unsigned char times = 0;	//��̬��ʱ����
	
	state = (en_AcLinkSta)hal_GPIO_GetACState();
	
	delay_1msTest();
	if(state == AcState)
	{////������λ�ȡ��״̬���ϴ�һ������������ʱ����
		times	= 0;
	}
	else if(state != AcState)
  {///���״̬�б仯
		times	++; //��������
		if(times > 20)
		{////�����һ����״̬��������20��  ����� AcState ״̬��
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


