#include "mt_task.h"
#include "mt_flash.h"
#include "mt_tftlcd.h"
#include "mt_lora.h"
#include "mt_wifi.h"
#include "mt_4g.h"
#include "mt_mqtt.h"
#include "mt_driver.h"
#include "mt_protocol.h"



void mt_task_init(void)
{
	mt_mqtt_init();
	mt_flashInit();
	mt_tftlcd_init();
    mt_lora_init();
	mt_wifi_init();
	mt_4g_Init();
	mt_proto_init();
	mt_drive_init();
}

void mt_task(void)
{
	mt_lora_Pro();
	mt_wifi_pro();
	mt_4g_pro();
	mt_drive_pro();
}






