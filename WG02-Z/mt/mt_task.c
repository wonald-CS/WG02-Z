#include "mt_task.h"
#include "mt_flash.h"
#include "mt_tftlcd.h"
#include "mt_lora.h"
#include "mt_wifi.h"
#include "mt_4g.h"

void mt_task_init(void)
{
	mt_flashInit();
	mt_tftlcd_init();
  mt_lora_init();
	mt_wifi_init();
	mt_4g_Init();
	
}

void mt_task(void)
{
	mt_lora_Pro();
	mt_wifi_pro();
	mt_4g_pro();
}






