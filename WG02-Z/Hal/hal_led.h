#ifndef _HAL_LED_H
#define _HAL_LED_H

#define LED8_PORT			GPIOC
#define LED8_PIN			GPIO_Pin_14

#define LED7_PORT			GPIOC
#define LED7_PIN			GPIO_Pin_15

void hal_LedInit(void);
void hal_LedProc(void);
void hal_Led_Task(void);	
#endif

