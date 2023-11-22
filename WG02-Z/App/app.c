#include "app.h"
#include "hal_al6630.h"
#include "hal_key.h"
#include "hal_adc.h"
#include "hal_gpio.h"
#include "mt_tftlcd.h"
#include "mt_lora.h"
#include "lcdfont.h"
#include "mt_wifi.h"


static void temHum_icon_Display(unsigned char fuc);
static void KeyEventHandle(EN_KEYNUM keys,KEY_VALUE_TYPEDEF sta);
static void PowerState_icon_Display(void);

static str_LoraAppNetState stgMenu_LoraDetectorApplyNetPro(en_lora_eventTypedef event,str_cmdApplyNet pData);
static unsigned char str_lora_loracommPro(en_lora_eventTypedef event,str_cmdApplyNet pData);
void app_task_init(void)
{
	hal_KeyScanCBSRegister(KeyEventHandle); 
	mt_loraRxApplyNet_callback_Register(stgMenu_LoraDetectorApplyNetPro);
	mt_lora_loracomm_callback_Register(str_lora_loracommPro);
}

void app_task(void)
{
	temHum_icon_Display(0);
	PowerState_icon_Display();
}


static void temHum_icon_Display(unsigned char fuc)
{
	static unsigned short humidityValue = 0;
	static unsigned short temperatureValue = 0;
	
	unsigned char displayBuf[6];
	unsigned char idx,updateFlag = 0;
	
	if(temperatureValue != hal_GetTemperatureDat())
	{
		temperatureValue = hal_GetTemperatureDat();
		updateFlag = 1;
	}
	if(humidityValue != hal_GethumidityDat())
	{
		humidityValue = hal_GethumidityDat();
		updateFlag = 1;
	}	
	
	
	if(fuc)
		updateFlag = 1;
	
	if(updateFlag == 1)
	{
		idx = 0;
		if((temperatureValue > 99) && (temperatureValue < 1000)) 
		{
			displayBuf[idx ++] = temperatureValue/100 +'0';  //1 +0x30 =0x31		
		}
		displayBuf[idx ++] = temperatureValue%100/10 +'0';
		displayBuf[idx ++] = '.';
		displayBuf[idx ++] = temperatureValue%10 +'0';
		displayBuf[idx ++] = '!';  ///?
		displayBuf[idx ++] = 0;	
		LCD_ShowString(70,3,displayBuf,HUE_LCD_FONT,HUE_LCD_BACK,24,0);
		
		idx = 0;
		if((humidityValue > 99) && (humidityValue < 1000)) 
		{
			displayBuf[idx ++] = humidityValue/100 +'0';
		}
		displayBuf[idx ++] = humidityValue%100/10 +'0';
		displayBuf[idx ++] = '.';
		displayBuf[idx ++] = humidityValue%10 +'0';
		displayBuf[idx ++] = '%';
		displayBuf[idx ++] = 0;		
		LCD_ShowString(135,3,displayBuf,HUE_LCD_FONT,HUE_LCD_BACK,24,0);		
	}
}

static void KeyEventHandle(EN_KEYNUM keys,KEY_VALUE_TYPEDEF sta)
{//00-15
		unsigned char keysta[3];
    keysta[0] = keys/10 + 0x30;
	  keysta[1] = keys%10 + 0x30;
	  keysta[2] = 0;
	
    LCD_ShowString(0,40,"KEYDBC:",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
	  LCD_ShowString(80,40,keysta,HUE_LCD_FONT,HUE_LCD_BACK,24,0);
	
	  LCD_ShowString(120,40,"-KEY-:",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
	
	  switch((unsigned char)keys)
	  {
			case KEY0:
			{
				mt_wifi_changState(ESP12_STA_WIFIConfig);
				LCD_ShowString(200,40,"KEY0",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
			}
			break;
			case KEY1:
			{
				LCD_ShowString(200,40,"KEY1",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
			}
			break;			
			case KEY2_UP:
			{
				LCD_ShowString(200,40,"KEY2",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
			}
			break;	
			case KEY3:
			{
				LCD_ShowString(200,40,"KEY3",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
			}
			break;	
			case KEY4_LEFT:
			{
				LCD_ShowString(200,40,"KEY4",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
			}
			break;			
			case KEY5:
			{
				LCD_ShowString(200,40,"KEY5",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
			}
			break;
			case KEY6_RIGHT:
			{
				LCD_ShowString(200,40,"KEY6",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
			}
			break;
			case KEY7:
			{
				LCD_ShowString(200,40,"KEY7",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
			}
			break;									
			case KEY8_DOWN:
			{
				LCD_ShowString(200,40,"KEY8",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
			}
			break;
			case KEY9:
			{
				LCD_ShowString(200,40,"KEY9",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
			}
			break;			
			case KEY_DISARM:
			{
				LCD_ShowString(200,40,"DISA",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
			}
			break;
			
			case KEY_HOMEARM:
			{
			  LCD_ShowString(200,40,"HOME",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
			}
			break;
			
			case KEY_AWARARM:
			{
				LCD_ShowString(200,40,"AWAY",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
			}
			break;
			
			case KEY_RETURN_DAIL:
			{
				LCD_ShowString(200,40,"DAIL",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
			}
			break;

			case KEY_SOS_DEL:
			{
				LCD_ShowString(200,40,"SOS ",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
			}
			break;
			case KEY_MENU:
			{
				LCD_ShowString(200,40,"MENU",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
			}
			break;
		}

	  keysta[0] = sta/10 + 0x30;
	  keysta[1] = sta%10 + 0x30;
		keysta[2] = 0;
		
		LCD_ShowString(0,65,"KEYSTA:",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
		LCD_ShowString(80,65,keysta,HUE_LCD_FONT,HUE_LCD_BACK,24,0);
}
/////////////////

static unsigned char str_lora_loracommPro(en_lora_eventTypedef event,str_cmdApplyNet pData)
{
	return 1;
}
/////Lora???????????
static str_LoraAppNetState stgMenu_LoraDetectorApplyNetPro(en_lora_eventTypedef event,str_cmdApplyNet pData)
{
	str_LoraAppNetState loraApplyNetSta;	
	loraApplyNetSta.state = LORADET_LEARN_FAIL;
	if(event == LORA_COM_APPLY_NET)
	{
			loraApplyNetSta.state = LORADET_LEARN_OK;
			//loraApplyNetSta.code = 1;		 
	}
	return loraApplyNetSta;
}


#define COOR_ICON_AC_X   1   ///外电状态的
#define COOR_ICON_AC_Y   1

#define COOR_ICON_BAT_X   33  //电池电量的图标
#define COOR_ICON_BAT_Y   1

static void PowerState_icon_Display(void)
{
	//static unsigned char batVolt = 0;
	static unsigned short InteralTim;
	if(hal_Gpio_AcStateCheck() == STA_AC_LINK)
	{///AC Link
		LCD_ShowPicture32PixFont(COOR_ICON_AC_X,COOR_ICON_AC_Y,ICON_32X32_ACLINK,HUE_LCD_FONT,HUE_LCD_BACK,0);
		if(hal_adc_returnVoltLevel() == LEVEL_FULL)
		{
			InteralTim = 0;
			LCD_ShowPicture32PixFont(COOR_ICON_BAT_X,COOR_ICON_BAT_Y,(ICON_32X32_BAT_LEVEL0+hal_adc_returnVoltLevel()),HUE_LCD_FONT,HUE_LCD_BACK,0);	
		}
		else 
		//if(hal_adc_ChargSta() == STA_BAT_CHARGING)
		{
			InteralTim ++;
			if(InteralTim == 50)
			{
				LCD_ShowPicture32PixFont(COOR_ICON_BAT_X,COOR_ICON_BAT_Y,ICON_32X32_BAT_LEVEL0,HUE_LCD_FONT,HUE_LCD_BACK,0);	
			}
			else if(InteralTim == 100)
			{
				LCD_ShowPicture32PixFont(COOR_ICON_BAT_X,COOR_ICON_BAT_Y,ICON_32X32_BAT_LEVEL1,HUE_LCD_FONT,HUE_LCD_BACK,0);	
			}
			else if(InteralTim == 150)
			{
				LCD_ShowPicture32PixFont(COOR_ICON_BAT_X,COOR_ICON_BAT_Y,ICON_32X32_BAT_LEVEL2,HUE_LCD_FONT,HUE_LCD_BACK,0);	
			}				
			else if(InteralTim == 200)
			{
				LCD_ShowPicture32PixFont(COOR_ICON_BAT_X,COOR_ICON_BAT_Y,ICON_32X32_BAT_LEVEL3,HUE_LCD_FONT,HUE_LCD_BACK,0);	
			}
			else if(InteralTim == 250)
			{
				LCD_ShowPicture32PixFont(COOR_ICON_BAT_X,COOR_ICON_BAT_Y,ICON_32X32_BAT_LEVEL4,HUE_LCD_FONT,HUE_LCD_BACK,0);	
			}	
			else if(InteralTim == 300)
			{
				InteralTim = 0;
				LCD_ShowPicture32PixFont(COOR_ICON_BAT_X,COOR_ICON_BAT_Y,ICON_32X32_BAT_LEVEL5,HUE_LCD_FONT,HUE_LCD_BACK,0);	
			}	
		}
	}
	else
	{///AC break; add wifi state
		LCD_ShowPicture32PixFont(COOR_ICON_AC_X,COOR_ICON_AC_Y,ICON_32X32_ACBREAK,HUE_LCD_FONT,HUE_LCD_BACK,0);
		////电池图标显示部分
		if(hal_adc_returnVoltLevel() <= LEVEL_VOLT_1)
		{
			InteralTim ++;
			if(InteralTim == 50) //500ms
			{
				LCD_ShowPicture32PixFont(COOR_ICON_BAT_X,COOR_ICON_BAT_Y,(ICON_32X32_BAT_LEVEL0+hal_adc_returnVoltLevel()),HUE_LCD_FONT,HUE_LCD_BACK,0);		
			}
			else if(InteralTim > 100)
			{
				InteralTim = 0;
				//hal_Tftlcd_ClearIcon(33,1);
				LCD_ShowPicture32PixFont(COOR_ICON_BAT_X,COOR_ICON_BAT_Y,ICON_32X32_CLEAR,HUE_LCD_FONT,HUE_LCD_BACK,0);		
			}
		}
		else
		{
			InteralTim = 0;
			LCD_ShowPicture32PixFont(COOR_ICON_BAT_X,COOR_ICON_BAT_Y,(ICON_32X32_BAT_LEVEL0+hal_adc_returnVoltLevel()),HUE_LCD_FONT,HUE_LCD_BACK,0);			
		}
	}
}	







