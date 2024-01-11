#include "hal_GPIO.H"
#include "hal_adc.h"
#include "hal_wTn6.h"
#include "mt_driver.h"
#include "mt_protocol.h"

static en_AcLinkSta ACLinkstate;
static unsigned char en_BatLowSta;

static void hal_Ac_Check_pro(void);
static void hal_Bat_Sta_Check_pro(void);

void mt_drive_init(void)
{
	ACLinkstate = hal_Gpio_AcStateCheck();
	en_BatLowSta = hal_adc_returnVoltLevel();	
}

void mt_drive_pro(void)
{
	hal_Ac_Check_pro();
	hal_Bat_Sta_Check_pro();
}
//////
static void hal_Ac_Check_pro(void)
{
	 if(ACLinkstate != hal_Gpio_AcStateCheck())
	 {
		 ACLinkstate = hal_Gpio_AcStateCheck();
		 if(ACLinkstate == STA_AC_LINK)
		 {
				hal_Wtn6_PlayVolue(WTN6_AC_RECOVER);
                mt_proto_PutInEvent(ENDPOINT_UP_TYPE_HOST_ACRESTORE,0xff);			 
		 }
		 else
		 {
				hal_Wtn6_PlayVolue(WTN6_AC_DOWN);
				mt_proto_PutInEvent(ENDPOINT_UP_TYPE_HOST_ACREMOVE,0xff);	
		 }
	 }
}
//////
static void hal_Bat_Sta_Check_pro(void)
{
	if(ACLinkstate == STA_AC_BREAK)
	{
		if(en_BatLowSta != hal_adc_returnVoltLevel())
		{
			en_BatLowSta = hal_adc_returnVoltLevel();		
			if(en_BatLowSta == LEVEL_LOW)		
			{
				hal_Wtn6_PlayVolue(WTN6_VOLUE_VOLT_LOW);
				mt_proto_PutInEvent(ENDPOINT_UP_TYPE_HOST_BATLOW,0xff);				
			}				
		}	
	}
}
 

