#include "hal_task.h"
#include "hal_task.h"
#include "hal_timer.h"
#include "hal_led.h"
#include "hal_gpio.h"
#include "hal_wtn6.h"
#include "hal_al6630.h"
#include "hal_key.h"
#include "hal_uart.h"
#include "hal_adc.h"
#include "hal_eeprom.h"


void hal_task_init(void)
{
	hal_timerInit();	
	hal_GpioConfig_init();		
	hal_LedInit();
	hal_wtn6();
	hal_timer_capInit();
	hal_keyInit();
  	hal_UsartInit();
  	hal_UsartProc();
	hal_Adc_batInit();
	hal_eepromInit();
}

void hal_task(void)
{
	hal_GetTemHumProc();
	hal_KeyProc();	
	hal_UsartProc();
	hal_BatCheckProc();
}



