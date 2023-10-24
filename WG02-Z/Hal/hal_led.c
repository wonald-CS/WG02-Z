#include "stm32F10x.h"
#include "hal_timer.h"
#include "hal_led.h"


static void hal_LedTurn(void)
{
	GPIO_WriteBit(LED8_PORT,LED8_PIN,(BitAction)(1-GPIO_ReadOutputDataBit(LED8_PORT,LED8_PIN)));
	GPIO_WriteBit(LED7_PORT,LED7_PIN,(BitAction)(1-GPIO_ReadOutputDataBit(LED7_PORT,LED7_PIN)));
}


static void hal_LedHandle(void)
{
	hal_LedTurn();
	hal_ResetTimer(T_LED,T_STA_START);
}

static void hal_LedConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC,ENABLE); 						 
	GPIO_InitStructure.GPIO_Pin = LED8_PIN | LED7_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; ; 
	GPIO_Init(LED8_PORT, &GPIO_InitStructure);
	
	GPIO_ResetBits(LED8_PORT,LED8_PIN);
	GPIO_SetBits(LED7_PORT,LED7_PIN);
}


void hal_LedInit(void)
{
	hal_LedConfig();  ///50uS *20000 = 1000 000 us = 1s
	hal_CreatTimer(T_LED,hal_LedHandle,20000,T_STA_START);
}





