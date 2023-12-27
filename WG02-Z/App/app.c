#include "app.h"
#include "hal_al6630.h"
#include "hal_key.h"
#include "hal_adc.h"
#include "hal_gpio.h"
#include "hal_wtn6.h"
#include "hal_tftlcd.h"
#include "mt_tftlcd.h"
#include "mt_lora.h"
#include "lcdfont.h"
#include "mt_wifi.h"
#include "para.h"
#include "mt_4G.h"
#include "OS_System.h"
#include "string.h"


stu_mode_menu *pModeMenu;	///当前执行的菜单	


static void temHum_icon_Display(unsigned char fuc);
static void KeyEventHandle(EN_KEYNUM keys,KEY_VALUE_TYPEDEF sta);
static void PowerState_icon_Display(void);
static void	showSystemTime(void);
static void Gsm_icon_Display(void);
static void Wifi_icon_Display(void);
static void Menu_Init(void);
static void Screen_On_Off(unsigned char Screen_Index);
static void Screen_Display(void);

static str_LoraAppNetState stgMenu_LoraDetectorApplyNetPro(en_lora_eventTypedef event,str_cmdApplyNet pData);
static unsigned char str_lora_loracommPro(en_lora_eventTypedef event,str_cmdApplyNet pData);

unsigned int Screen_Timer;
unsigned char Screen_Display_Flag;

void app_task_init(void)
{
	Screen_Timer = 0;
	Screen_Display_Flag = FALSE;
	hal_KeyScanCBSRegister(KeyEventHandle); 
	mt_loraRxApplyNet_callback_Register(stgMenu_LoraDetectorApplyNetPro);
	mt_lora_loracomm_callback_Register(str_lora_loracommPro);
	Menu_Init();
	Screen_On_Off(TRUE);
}

void app_task(void)
{
	Screen_Display();
	showSystemTime();
	temHum_icon_Display(1);
	PowerState_icon_Display();
	Gsm_icon_Display();
	Wifi_icon_Display();
	
}



static void Screen_Display(void)
{
	//子菜单不操作20S返回原始桌面，该功能未做

	if((pModeMenu->refreshScreenCmd == SCREEN_CMD_RESET)||(pModeMenu->refreshScreenCmd == SCREEN_CMD_RECOVER))
	{
		pModeMenu->refreshScreenCmd = SCREEN_CMD_NULL;
		pModeMenu->Key_Par.keyVal = 0xFF;
		hal_Tftlcd_Clear();
		showSystemTime();
		temHum_icon_Display(1);
		PowerState_icon_Display();
		Gsm_icon_Display();
		Wifi_icon_Display();
	}

	if(Screen_Display_Flag && pModeMenu->menuPos == DESKTOP_MENU_POS)
	{
		Screen_Timer ++;
		if (Screen_Timer > Srceen_Off_Time)
		{
			Screen_Timer = 0;
			Screen_On_Off(FALSE);
		}
	}
}

stu_mode_menu generalModeMenu[GNL_MENU_SUM] =
{
	{DESKTOP_MENU_POS,SCREEN_CMD_NULL,0xFF,0},		
};	


static void Menu_Init(void)
{
	//结构体指针的赋值要先通过&赋值，给整个结构体分配内存,然后才能操作结构体变量
	pModeMenu = &generalModeMenu[GNL_MENU_DESKTOP];	
	// pModeMenu->menuPos = DESKTOP_MENU_POS;
	// pModeMenu->Key_Par.keyVal = 0;
	// pModeMenu->Key_Par.state = 5;
}


//WIFI信号强弱具体显示可以通过AT+CWLAP=获取信息，通过检索出当前连接WIFI的信息可以做WIFI信号强弱。
//目前只显示有无连接的状态
static void Wifi_icon_Display(void)
{
	if((mt_wifi_GetState()==ESP12_STA_RESET) || (mt_wifi_GetState()==ESP12_STA_WIFI_POWER))
	{
		LCD_ShowPicture32PixFont(COOR_ICON_WIFI_X,COOR_ICON_WIFI_Y,ICON_32X32_CLEAR,HUE_LCD_FONT,HUE_LCD_BACK,0);
	}else{
		LCD_ShowPicture32PixFont(COOR_ICON_WIFI_X,COOR_ICON_WIFI_Y,ICON_32X32_WIFI_S4,HUE_LCD_FONT,HUE_LCD_BACK,0);
	}
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
{	
	if(!Screen_Display_Flag)
	{
		Screen_On_Off(TRUE);
	}
	pModeMenu->Key_Par.keyVal = keys;
	pModeMenu->Key_Par.state = sta;
	//unsigned char test[256];

	  switch((unsigned char)keys)
	  {
			case KEY0:
			{

			}
			break;
			case KEY1:
			{
				// test[0] = 1;
				// test[1] = 0;
				// test[2] = 0;
				// test[3] = 8;
				// test[4] = 6;
				// test[5] = 0xff;

        		// mt_4G_PhoneDial_Ctrl(test);	
			}
			break;			
			case KEY2_UP:
			{
				// mt_4g_Phone_Handup();
			}
			break;	
			case KEY3:
			{

			}
			break;	
			case KEY4_LEFT:
			{			
				// test[0] = 1;
				// test[1] = 8;
				// test[2] = 3;
				// test[3] = 2;
				// test[4] = 0;
				// test[5] = 6;
				// test[6] = 6;
				// test[7] = 9;
				// test[8] = 2;
				// test[9] = 2;
				// test[10] = 7;
				// test[11] = 0xff;

				// test[20] = 'x';
				// test[21] = 'i';
				// test[22] = 'a';
				// test[23] = 'n';
				// test[24] = ' ';
				// test[25] = 'y';
				// test[26] = 'a';
				// test[27] = 'n';
				// test[28] = ' ';
				// test[29] = 'l';
				// test[30] = 'a';
				// test[31] = 'o';
				// test[32] = 'p';
				// test[33] = 'o';
				// test[34] = 0;
				// mt_4G_MesSend_Ctrl(test);
			}
			break;			
			case KEY5:
			{

			}
			break;
			case KEY6_RIGHT:
			{
	
			}
			break;
			case KEY7:
			{
			}
			break;									
			case KEY8_DOWN:
			{
	
			}
			break;
			case KEY9:
			{
			
			}
			break;			
			case KEY_DISARM:
			{
				LCD_ShowString(COOR_ICON_SYSTEMODE_X,COOR_ICON_SYSTEMODE_Y," DISARM ",HUE_LCD_FONT,HUE_LCD_BACK,48,0);
				if((sta != KEY_CLICK_RELEASE) && (sta != KEY_LONG_PRESS_RELEASE))
				{
					hal_Wtn6_PlayVolue(WTN6_DISARM);	
				}	
						
			}
			break;
			
			case KEY_HOMEARM:
			{
				LCD_ShowString(COOR_ICON_SYSTEMODE_X,COOR_ICON_SYSTEMODE_Y,"HOMEARM ",HUE_LCD_FONT,HUE_LCD_BACK,48,0);
				if((sta != KEY_CLICK_RELEASE) && (sta != KEY_LONG_PRESS_RELEASE))
				{
					hal_Wtn6_PlayVolue(WTN6_HOMEARM);	
				}	
								 
			}
			break;
			
			case KEY_AWAYARM:
			{
				LCD_ShowString(COOR_ICON_SYSTEMODE_X,COOR_ICON_SYSTEMODE_Y,"AWAYARM ",HUE_LCD_FONT,HUE_LCD_BACK,48,0);	
				if((sta != KEY_CLICK_RELEASE) && (sta != KEY_LONG_PRESS_RELEASE))
				{
					hal_Wtn6_PlayVolue(WTN6_AWAYARM);			
				}
						
			}
			break;
			
			case KEY_RETURN_DAIL:
			{
				
			}
			break;

			case KEY_SOS_DEL:
			{
				
			}
			break;
			case KEY_MENU:
			{
			
			}
			break;
		}

		if(sta == KEY_CLICK)
		{
			hal_Wtn6_PlayVolue(WTN6_VOLUE_DI);
		} 

}


static unsigned char str_lora_loracommPro(en_lora_eventTypedef event,str_cmdApplyNet pData)
{
	return 1;
}


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


static void showSystemTime(void)
{   
	unsigned char displaytimebuf[16];
	memset(displaytimebuf,0,16);

	displaytimebuf[0] = (stuSystemtime.year/1000) +'0';	
	displaytimebuf[1] = (stuSystemtime.year%1000/100) +'0';	
	displaytimebuf[2] = (stuSystemtime.year%1000%100/10) +'0';
	displaytimebuf[3] = (stuSystemtime.year%1000%100%10) +'0';
	displaytimebuf[4] = '-';	

	displaytimebuf[5] = (stuSystemtime.mon/10) +'0';
	displaytimebuf[6] = (stuSystemtime.mon%10) +'0';
	displaytimebuf[7] = '-';	

	displaytimebuf[8] = (stuSystemtime.day/10) +'0';
	displaytimebuf[9] = (stuSystemtime.day%10) +'0';	
	displaytimebuf[10] = 0;
	LCD_ShowString(20,200,displaytimebuf,HUE_LCD_FONT,HUE_LCD_BACK,24,0);
	
	displaytimebuf[0] = (stuSystemtime.hour/10)+'0';
	displaytimebuf[1] = (stuSystemtime.hour%10)+'0';
	displaytimebuf[2] = ':';	
	displaytimebuf[3] = (stuSystemtime.min/10)+'0';
	displaytimebuf[4] = (stuSystemtime.min%10)+'0';		
	displaytimebuf[5] = 0;
//	displaytimebuf[5] = ':';
//	displaytimebuf[6] = (stuSystemtime.sec/10)+'0';
//	displaytimebuf[7] = (stuSystemtime.sec%10)+'0';
//	displaytimebuf[8] = 0;
	LCD_ShowString(160,200,displaytimebuf,HUE_LCD_FONT,HUE_LCD_BACK,24,0);	
	switch(stuSystemtime.week)
	{
		case 0:	
			LCD_ShowString(260,200,"Sun",HUE_LCD_FONT,HUE_LCD_BACK,24,0);	
		break;

		case 1:	
			LCD_ShowString(260,200,"Mon",HUE_LCD_FONT,HUE_LCD_BACK,24,0);	
		break;

		case 2:	
			LCD_ShowString(260,200,"Tue",HUE_LCD_FONT,HUE_LCD_BACK,24,0);		
		break;

		case 3:	
			LCD_ShowString(260,200,"Wed",HUE_LCD_FONT,HUE_LCD_BACK,24,0);					
		break;
		
		case 4:	
			LCD_ShowString(260,200,"Thu",HUE_LCD_FONT,HUE_LCD_BACK,24,0);			
		break;

		case 5:	
			LCD_ShowString(260,200,"Fir",HUE_LCD_FONT,HUE_LCD_BACK,24,0);	
		break;
		
		case 6:	
			LCD_ShowString(260,200,"Sat",HUE_LCD_FONT,HUE_LCD_BACK,24,0);	
		break;
	}	

	//服务器连接状态图标
	if((GSM_SERVAL_STATUS == TRUE) || (WIFI_SERVAL_STATUS == TRUE))
	{
		LCD_ShowPicture32PixFont(COOR_ICON_SERVER_X,COOR_ICON_SERVER_Y,ICON_32X32_SERVER,HUE_LCD_FONT,HUE_LCD_BACK,0);	
	}else if((GSM_SERVAL_STATUS == FALSE) && (WIFI_SERVAL_STATUS == FALSE)){
		LCD_ShowPicture32PixFont(COOR_ICON_SERVER_X,COOR_ICON_SERVER_Y,ICON_32X32_CLEAR,HUE_LCD_FONT,HUE_LCD_BACK,0);	
	}
	
}



static void Gsm_icon_Display(void)
{
	static unsigned short Timer;
	if(GSM_SIGNAL == 0xff)
	{//没有SIM卡
		LCD_ShowPicture32PixFont(282,0,ICON_32X32_GSM_NOCARD,HUE_LCD_FONT,HUE_LCD_BACK,0);
	}
	else
		{//有SIM卡 需要显示信号的强弱
		Timer ++;
		if(Timer == 100)  //1秒
		{
			LCD_ShowPicture32PixFont(282,0,(GSM_SIGNAL+ICON_32X32_GSM_S1),HUE_LCD_FONT,HUE_LCD_BACK,0);
			Timer = 0;
		}		
	}
	/************************************/
}

/*******************************************************************************************
*@description:屏幕熄灭或者点亮函数
*@param[in]：Screen_Index？0：熄灭，1：点亮
*@return：
*@others：
********************************************************************************************/
static void Screen_On_Off(unsigned char Screen_Index)
{
	if (Screen_Index)
	{
		hal_Oled_Display_on();
		Screen_Display_Flag = TRUE;
	}else
	{
		hal_Oled_Display_off();
		Screen_Display_Flag = FALSE;
	}
	
}
