#include "stm32F10x.h"
#include "hal_power.h"


void hal_WIFI_Power_OFF(void)
{
	GPIO_ResetBits(WIFI_POWER_PORT,WIFI_POWER_PIN);	
}


void hal_WIFI_Power_ON(void)
{
	GPIO_SetBits(WIFI_POWER_PORT,WIFI_POWER_PIN);	
}


void hal_4G_Power_OFF(void)
{
	GPIO_SetBits(EC200S_POWER_PORT,EC200S_POWER_PIN);
}


void hal_4G_Power_ON(void)
{
	GPIO_ResetBits(EC200S_POWER_PORT,EC200S_POWER_PIN);
}



void hal_PowerInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC , ENABLE); 
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA , ENABLE);

	GPIO_InitStructure.GPIO_Pin = WIFI_POWER_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_Init(WIFI_POWER_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = EC200S_POWER_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_Init(EC200S_POWER_PORT, &GPIO_InitStructure);

	
	hal_WIFI_Power_ON();	
	hal_4G_Power_ON();
}
