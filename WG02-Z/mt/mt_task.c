#include "mt_task.h"
#include "mt_flash.h"
#include "mt_Tftlcd.h"
#include "mt_lora.h"
#include "mt_wifi.h"

void mt_task_Init(void)
{  
    mt_flashInit();
    mt_tftlcd_init();
    mt_lora_Init();
    mt_wifi_Init();
}


void mt_task(void)
{
    mt_lora_Config();
    //mt_wifi_Config();
}




