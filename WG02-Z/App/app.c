#include "app.h"
#include "hal_al6630.h"
#include "hal_key.h"
#include "mt_tftlcd.h"
#include "mt_lora.h"



static unsigned short HumidityValue = 0;
static unsigned short TemperatureValue = 0;

static void App_TempHum_show(unsigned char fuc);
static void KeyEventHandle(EN_KEYNUM keys,KEY_VALUE_TYPEDEF sta);
static str_LoraAppNetState stgMenu_LoraDetectorApplyNetPro(Detector_EVENT event,Data_AppNet_Handle pData);
static unsigned char str_lora_loracommPro(Detector_EVENT event,Data_AppNet_Handle pData);

void app_task_Init(void)
{
    //hal_KeyScanCBSRegister(KeyEventHandle);     //把KeyEventHandle传给KeyScanCBS，把按键值和状态值返回给KeyEventHandle()
    mt_loraRxApplyNet_callback_Register(stgMenu_LoraDetectorApplyNetPro);
	mt_lora_loracomm_callback_Register(str_lora_loracommPro);
}


void app_task(void)
{
    App_TempHum_show(0);
}


void TemperatureValue_Show(void)
{
    unsigned char displayBuf[6],idx = 0;

    if((TemperatureValue > 99) && (TemperatureValue < 1000)) 
    {
        displayBuf[idx ++] = TemperatureValue/100 +'0';  //1 +0x30 =0x31		
    }
    displayBuf[idx ++] = TemperatureValue%100/10 +'0';
    displayBuf[idx ++] = '.';
    displayBuf[idx ++] = TemperatureValue%10 +'0';
    displayBuf[idx ++] = 'C';  
    displayBuf[idx ++] = 0;	
    LCD_ShowString(70,3,displayBuf,HUE_LCD_FONT,HUE_LCD_BACK,24,0);
}

void HumidityValue_Show(void)
{
    unsigned char displayBuf[6],idx = 0;

    if((HumidityValue > 99) && (HumidityValue < 1000)) 
    {
        displayBuf[idx ++] = HumidityValue/100 +'0';
    }
    displayBuf[idx ++] = HumidityValue%100/10 +'0';
    displayBuf[idx ++] = '.';
    displayBuf[idx ++] = HumidityValue%10 +'0';
    displayBuf[idx ++] = '%';
    displayBuf[idx ++] = 0;		
    LCD_ShowString(135,3,displayBuf,HUE_LCD_FONT,HUE_LCD_BACK,24,0);	
}

static void App_TempHum_show(unsigned char fuc)
{
	unsigned char updateFlag = 0;
	
	if(fuc)
	{	
        updateFlag = 1;
	}

	if(TemperatureValue != Get_Temperature_Data())
	{
		TemperatureValue = Get_Temperature_Data();
		updateFlag = 1;
	}

	if(HumidityValue != Get_Humidity_Data())
	{
		HumidityValue = Get_Humidity_Data();
		updateFlag = 1;
	}	
	
	if(updateFlag == 1)
	{
        TemperatureValue_Show();
        HumidityValue_Show();		
	}    
}


static void KeyEventHandle(EN_KEYNUM keys,KEY_VALUE_TYPEDEF sta)
{//00-15
    unsigned char keysta[3];

    keysta[0] = keys/10 + 0x30;
    keysta[1] = keys%10 + 0x30;
    keysta[2] = 0;

    LCD_ShowString(0,40,"-KEY-:",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
	
    switch((unsigned char)keys)
    {
        case KEY0:
        {
            LCD_ShowString(80,40,"KEY0",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
        }
        break;
        case KEY1:
        {
            LCD_ShowString(80,40,"KEY1",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
        }
        break;			
        case KEY2_UP:
        {
            LCD_ShowString(80,40,"KEY2",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
        }
        break;	
        case KEY3:
        {
            LCD_ShowString(80,40,"KEY3",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
        }
        break;	
        case KEY4_LEFT:
        {
            LCD_ShowString(80,40,"KEY4",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
        }
        break;			
        case KEY5:
        {
            LCD_ShowString(80,40,"KEY5",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
        }
        break;
        case KEY6_RIGHT:
        {
            LCD_ShowString(80,40,"KEY6",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
        }
        break;
        case KEY7:
        {
            LCD_ShowString(80,40,"KEY7",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
        }
        break;									
        case KEY8_DOWN:
        {
            LCD_ShowString(80,40,"KEY8",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
        }
        break;
        case KEY9:
        {
            LCD_ShowString(80,40,"KEY9",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
        }
        break;			
        case KEY_DISARM:
        {
            LCD_ShowString(80,40,"DISA",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
        }
        break;
        
        case KEY_HOMEARM:
        {
            LCD_ShowString(80,40,"HOME",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
        }
        break;
        
        case KEY_AWARARM:
        {
            LCD_ShowString(80,40,"AWAY",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
        }
        break;
        
        case KEY_CANCEIL_DAIL:
        {
            LCD_ShowString(80,40,"DAIL",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
        }
        break;

        case KEY_SOS_DEL:
        {
            LCD_ShowString(80,40,"SOS ",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
        }
        break;
        case KEY_MENU:
        {
            LCD_ShowString(80,40,"MENU",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
        }
        break;
    }

    LCD_ShowString(0,65,"KEYDBC:",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
    LCD_ShowString(80,65,keysta,HUE_LCD_FONT,HUE_LCD_BACK,24,0);

    keysta[0] = sta/10 + 0x30;
    keysta[1] = sta%10 + 0x30;
    keysta[2] = 0;
		
    LCD_ShowString(120,65,"KEYSTA:",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
    LCD_ShowString(200,65,keysta,HUE_LCD_FONT,HUE_LCD_BACK,24,0);
}



static unsigned char str_lora_loracommPro(Detector_EVENT event,Data_AppNet_Handle pData)
{
	return 1;
}

static str_LoraAppNetState stgMenu_LoraDetectorApplyNetPro(Detector_EVENT event,Data_AppNet_Handle pData)
{
	str_LoraAppNetState loraApplyNetSta;	
	loraApplyNetSta.state = LORADET_LEARN_FAIL;
	if(event == Detector_AppNet)
	{
        loraApplyNetSta.state = LORADET_LEARN_OK;		
	}
	return loraApplyNetSta;
}
