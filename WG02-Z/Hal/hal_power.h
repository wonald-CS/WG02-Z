#ifndef _HAL_POWER_H_
#define _HAL_POWER_H_


#define WIFI_POWER_PORT			    GPIOC
#define WIFI_POWER_PIN			    GPIO_Pin_4

#define EC200S_POWER_PORT			GPIOA
#define EC200S_POWER_PIN			GPIO_Pin_15


void hal_WIFI_Power_ON (void);
void hal_WIFI_Power_OFF (void);
void hal_4G_Power_HIGH(void);
void hal_4G_Power_LOW(void);

void hal_PowerInit(void);

#endif
