#include "hal_task.h"
#include "hal_timer.h"
#include "hal_led.h"
#include "hal_usb.h"
#include "hal_wtn6.h"
#include "hal_al6630.h"
#include "hal_key.h"
#include "hal_usart.h"
#include "hal_power.h"
#include "hal_adc.h"


void hal_task_Init(void)
{
    hal_timerInit();
    hal_UsbInit();
    hal_LedInit();
    hal_Wtn6Init();
    hal_Al6630_Init();
    hal_key_Init();
    hal_UsartInit();
    hal_PowerInit();
    hal_AdcInit();
}



void hal_task(void)
{
    hal_GetTemHum_Proc();
    hal_KeyProc();
    hal_UsartProc();
    hal_Adc_Bat_Proc();
}

