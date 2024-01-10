#include "app.h"
#include "os_system.h"
#include "hal_al6630.h"
#include "hal_wtn6.h"
#include "hal_key.h"
#include "hal_adc.h"
#include "hal_gpio.h"
#include "hal_eeprom.h"
#include "hal_tftlcd.h"
#include "hal_timer.h"
#include "mt_tftlcd.h"
#include "mt_lora.h"
#include "mt_wifi.h"
#include "mt_4g.h"
#include "mt_api.h"
#include "mt_protocol.h"
#include "mt_update.h"


#include "lcdfont.h"
#include "para.h"
#include "string.h"

stu_mode_menu *pModeMenu;	///当前执行的菜单	
stu_loraDtector loraDtector;
////APP.C
unsigned short SetupMenuTimeOutCnt;//菜单无按键操作返回桌面的时间计数器
unsigned short PutoutScreenTiemr;//桌面无按键操作进入息屏的时间计数器
unsigned char ScreenState;		//熄屏状态 0 表示熄屏   1表示未熄屏

static void server_icon_Display(void);
static void wifi_icon_Display(void);
static void temHum_icon_Display(unsigned char fuc);
static void KeyEventHandle(EN_KEYNUM keys,KEY_VALUE_TYPEDEF sta);
static void PowerState_icon_Display(void);

Queue16 LoraRcvMsg;	//lORA 应用层数据处理丢列


static str_LoraAppNetState stgMenu_LoraDetectorApplyNetPro(en_lora_eventTypedef event,str_cmdApplyNet pData);
static unsigned char str_lora_loracommPro(en_lora_eventTypedef event,str_cmdApplyNet pData);
static void showSystemTime(void);
static void Gsm_icon_Display(void);
static void ScreeControl(unsigned char cmd);
static void gnlMenu_DesktopCBS(void);

static void gnlMenu_DesktopCBS(void);
static void gnlMenu_DialNumberCBS(void);
static void gnlMenu_EnterPinCBS(void);
static void systemAlarmKeyDisArmHandleCBS(void);
static void ganMenu_FirmwareUpdate(void);
 

stu_mode_menu generalModeMenu[GNL_MENU_SUM] =
{
    {GNL_MENU_DESKTOP,DESKTOP_MENU_POS,"Desktop",gnlMenu_DesktopCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0,0},	
	{GNL_MENU_DAIL,STG_SUB_ALARMING_POS,"Dial Numbe",gnlMenu_DialNumberCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0,0},		  // 
	{GNL_MENU_ENTER_PIN,STG_SUB_MENU_POS,"Enter Pin",gnlMenu_EnterPinCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0,0},		// 
	{GNL_MENU_ENTER_DISARM,STG_SUB_MENU_POS,"Key disArm",systemAlarmKeyDisArmHandleCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0,0},		// 	
	{GNL_MENU_FIREWARE_UP,STG_SUB_FIREWARE_UP,"Firmware Update",ganMenu_FirmwareUpdate,SCREEN_CMD_RESET,0,0xFF,0,0,0,0,0},		// 		
};	


///主菜单
static void stgMenu_MainMenuCBS(void);
//子菜单
static void stgMenu_LearnSensorCBS(void);///探测器学习配对
static void stgMenu_DTCListCBS(void);//探测器属性
static void stgMenu_WifiCBS(void);//WIFI 配网
static void stgMenu_PasswordCBS(void);//密码
static void stgMenu_PhoneNumberCBS(void);//报警电话号码
static void stgMenu_MachineInfoCBS(void);//设备信息
static void stgMenu_FactorySettingsCBS(void);//出厂设置
static void stgMenu_AlarmRecordCBS(void);//报警记录




//app.c
stu_mode_menu settingModeMenu[STG_MENU_SUM] = 
{                                   
	{STG_MENU_MAIN_SETTING,STG_SUB_MENU_POS,"     Main Menu      ",stgMenu_MainMenuCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0,0},		//������ҳ��
	{STG_MENU_LEARNING_SENSOR,STG_SUB_1_MENU_POS,"1.Learning Dtc    ",stgMenu_LearnSensorCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0,0},	//̽����ѧϰ
	{STG_MENU_DTC_LIST,STG_SUB_1_MENU_POS,"2.Dtc List        ",stgMenu_DTCListCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0,0},			//����
	{STG_MENU_WIFI,STG_SUB_WIFI_MENU_POS,"3.WiFi            ",stgMenu_WifiCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0,0},
    {STG_MENU_PASSWORD,STG_SUB_1_MENU_POS,"4.Password        ",stgMenu_PasswordCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0,0},
	{STG_MENU_PHONE_NUMBER,STG_SUB_1_MENU_POS,"5.Phone Number    ",stgMenu_PhoneNumberCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0,0},
	{STG_MENU_MACHINE_INFO,STG_SUB_1_MENU_POS,"6.Mac Info        ",stgMenu_MachineInfoCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0,0},
	{STG_MENU_FACTORY_SETTINGS,STG_SUB_1_MENU_POS,"7.Default Setting ",stgMenu_FactorySettingsCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0,0},		
	{STG_MENU_ALARM_RECORD,STG_SUB_1_MENU_POS,"8.Alarm Recording ",stgMenu_AlarmRecordCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0,0},
};


static void stgMenu_dl_ReviewMainCBS(void);
static void stgMenu_dl_ReviewCBS(void);
static void stgMenu_dl_EditCBS(void);
static void stgMenu_dl_DeleteCBS(void);



//3.DTC list->Zone xxx->Review
stu_mode_menu DL_ZX_Review[STG_MENU_DL_ZX_SUM] = 
{
	{STG_MENU_DL_ZX_REVIEW_MAIN,STG_SUB_1_MENU_POS,"View",stgMenu_dl_ReviewMainCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0},	
	{STG_MENU_DL_ZX_REVIEW,STG_SUB_2_MENU_POS,"View",stgMenu_dl_ReviewCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0},		 
	{STG_MENU_DL_ZX_EDIT,STG_SUB_2_MENU_POS,"Edit",stgMenu_dl_EditCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0},
	{STG_MENU_DL_ZX_DELETE,STG_SUB_2_MENU_POS,"Delete",stgMenu_dl_DeleteCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0},
};



//6. Phone number
typedef enum
{
	STG_MENU_PN_NB1=0,			//报警电话1
	STG_MENU_PN_NB2,			//报警电话2
	STG_MENU_PN_NB3,			//报警电话3
	STG_MENU_PN_NB4,			//报警电话4
	STG_MENU_PN_NB5,			//报警电话5
	STG_MENU_PN_NB6,			//报警电话6
	STG_MENU_PN_SUM,
}STG_MENU_PN_LIST;
/////

///6.Phone Number
unsigned char PNModeTypeAry[STG_MENU_PN_SUM][24] = 
{
	{"1. \0"},
	{"2. \0"},
	{"3. \0"},
	{"4. \0"},
	{"5. \0"},
	{"6. \0"},
};
static void stgMenu_PN_EnterNbCBS(void);

stu_mode_menu PN_Submenu[STG_MENU_PN_SUM] = 
{
	{STG_MENU_PN_NB1,STG_SUB_2_MENU_POS,PNModeTypeAry[0],stgMenu_PN_EnterNbCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0},
	{STG_MENU_PN_NB2,STG_SUB_2_MENU_POS,PNModeTypeAry[1],stgMenu_PN_EnterNbCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0},	
	{STG_MENU_PN_NB3,STG_SUB_2_MENU_POS,PNModeTypeAry[2],stgMenu_PN_EnterNbCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0},	
	{STG_MENU_PN_NB4,STG_SUB_2_MENU_POS,PNModeTypeAry[3],stgMenu_PN_EnterNbCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0},
	{STG_MENU_PN_NB5,STG_SUB_2_MENU_POS,PNModeTypeAry[4],stgMenu_PN_EnterNbCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0},	
	{STG_MENU_PN_NB6,STG_SUB_2_MENU_POS,PNModeTypeAry[5],stgMenu_PN_EnterNbCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0},		
};


static void S_ENArmModeProc(void);
static void S_DisArmModeProc(void);
static void S_HomeArmModeProc(void);
static void S_AlarmModeProc(void);

void SystemMode_Change(unsigned char zone,SYSTEMMODE_TYPEDEF sysMode,EN_SYSTEMODE_OPTERTYPEDEF operType);

stu_system_mode stu_Sysmode[SYSTEM_MODE_SUM] =
{
	{SYSTEM_MODE_ENARM_HOST,SCREEN_CMD_RESET,0xFF,S_ENArmModeProc},
	{SYSTEM_MODE_HOMEARM_HOST,SCREEN_CMD_RESET,0xFF,S_HomeArmModeProc},
	{SYSTEM_MODE_DISARM_HOST,SCREEN_CMD_RESET,0xFF,S_DisArmModeProc},
	{SYSTEM_MODE_ALARM,SCREEN_CMD_RESET,0xFF,S_AlarmModeProc},
};

stu_system_mode *pStuSystemMode;
/////



static void systemAlarmHandleCBS(void);

Queue8 DtcTriggerIDMsg;	//触发的探测器ID队列



stu_mode_menu systemAlarmMenu[SYSTEMALARM_MODE_SUM]=
{
	{SYSTEMALARM_MODE_ALARM,STG_SUB_ALARMING_POS,"SYSTEM ALARMING",systemAlarmHandleCBS,SCREEN_CMD_RESET,0,0,0,0,0,0},
	{SYSTEMALARM_MODE_DISARM,STG_SUB_1_MENU_POS,"SYSTEM DISARM",systemAlarmKeyDisArmHandleCBS,SCREEN_CMD_RESET,0,0,0,0,0,0},
};
static void systemAlarmDialSmsHandle(un_AlarmDailSmsSta initflag);
static void detector_off_lineInit(void);

static void stgMenu_dl_ReviewMainCBS(void)
{
	unsigned char keys = 0xFF;
	unsigned char state;
	unsigned char i,ClrScreenFlag=0;///是否清屏的
	Stu_DTC tStuDtc;
	 
	static stu_mode_menu *MHead;		//这两个结构用来方便显示，页面的头跟尾
	static stu_mode_menu *pMenu,*bpMenu=0;	//用来记录当前选中的菜单
	
	static unsigned char stgMainMenuSelectedPos=0;				 

	if(pModeMenu->refreshScreenCmd == SCREEN_CMD_RESET)
	{	//执行页面切换时屏幕刷新显示 
		pModeMenu->refreshScreenCmd = SCREEN_CMD_NULL;
		hal_Tftlcd_Clear();
		
		if(CheckPresenceofDtc(pModeMenu->reserved))
		{
			GetDtcStu(&tStuDtc,pModeMenu->reserved);	//读取探测器信息
		}
		LCD_ShowString(80,10,tStuDtc.Name,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);

		pMenu = &DL_ZX_Review[STG_MENU_DL_ZX_REVIEW];
		stgMainMenuSelectedPos = 1;
		MHead = pMenu;			//记录当前显示菜单第一项
		ClrScreenFlag = 1;
		bpMenu = 0;
		keys = 0xFF;
	}
	else if(pModeMenu->refreshScreenCmd==SCREEN_CMD_RECOVER)
	{
		pModeMenu->refreshScreenCmd = SCREEN_CMD_NULL;
	///	恢复之前的选择位置显示
		if(CheckPresenceofDtc(pModeMenu->reserved))
		{
			GetDtcStu(&tStuDtc,pModeMenu->reserved);	//读取探测器信息
		}
		hal_Tftlcd_Clear();
		LCD_ShowString(80,10,tStuDtc.Name,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);

		keys = 0xFF;
		ClrScreenFlag = 1;
		bpMenu = 0;
	}
	
	if(pModeMenu->keyVal != 0xff)
	{
		keys = pModeMenu->keyVal;
		pModeMenu->keyVal = 0xFF;	//

		state = pModeMenu->state;
		if(state == KEY_CLICK)
		{
			switch(keys)
			{
				case KEY2_UP:		//上
					if(stgMainMenuSelectedPos ==1)
					{
					///	pMenu = pMenu->pLase;
					////	MHead = MHead->pLase;
					///	ClrScreenFlag = 1;						
					}
					else
					{
						LCD_ShowString(20,(20+32*stgMainMenuSelectedPos),pMenu->pModeType,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);//取消选中本菜单显示	
						pMenu = pMenu->pLase;
						stgMainMenuSelectedPos--;
					}
				break;
				case KEY8_DOWN:		//下
					if(stgMainMenuSelectedPos ==3)
					{
					///	pMenu = pMenu->pNext;
					///	MHead = MHead->pNext;
					///	ClrScreenFlag = 1;
					}
					else
					{
						LCD_ShowString(20,(20+32*stgMainMenuSelectedPos),pMenu->pModeType,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);//取消选中本菜单显示	
						pMenu = pMenu->pNext;																			//切换下一个选项
						stgMainMenuSelectedPos++;
					}
				break;

				case KEY_MENU:
					pMenu->reserved = pModeMenu->reserved;	//继续把探测器id传递下去
					pModeMenu = pMenu;
					pModeMenu->refreshScreenCmd = SCREEN_CMD_RESET;
				break;

				case KEY_RETURN_DAIL:	//返回上一级菜单
					//pModeMenu = pModeMenu->pParent;
				    pModeMenu = &settingModeMenu[STG_MENU_DTC_LIST];
					pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
				break;
			}
		}
	}
	
	if(bpMenu != pMenu)
	{
		bpMenu = pMenu;
		if(ClrScreenFlag)
		{
			ClrScreenFlag = 0;
			pMenu = MHead;
			for(i=0; i<3; i++)
			{
				LCD_ShowString(20,(20+32*(i+1)),pMenu->pModeType,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
				pMenu = pMenu->pNext;
			} 
			pMenu = bpMenu;
			LCD_ShowString(20,(20+32*stgMainMenuSelectedPos),pMenu->pModeType,HUE_LCD_FONT,HUE_FONT_BACK,FORTSIZE_24,0);//取消选中本菜单显示	
		}
		else
		{  
			LCD_ShowString(20,(20+32*stgMainMenuSelectedPos),pMenu->pModeType,HUE_LCD_FONT,HUE_FONT_BACK,FORTSIZE_24,0);//取消选中本菜单显示	
		}		 
	}

}



static void stgMenu_dl_ReviewCBS(void)
{
    unsigned char keys,state;
	Stu_DTC tStuDtc;
	unsigned char temp[25];
	if(pModeMenu->refreshScreenCmd == SCREEN_CMD_RESET)
	{// 界面初始化
		pModeMenu->refreshScreenCmd = SCREEN_CMD_NULL;
		
		if(CheckPresenceofDtc(pModeMenu->reserved))
		{
			GetDtcStu(&tStuDtc,pModeMenu->reserved);
		}
		
		hal_Tftlcd_Clear();
		LCD_ShowString(130,10,pModeMenu->pModeType,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
		
		
		
		LCD_ShowString(10,50,"<Name>: ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
		LCD_ShowString(10,80,"<Type>: ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
		LCD_ShowString(10,110,"<ZoneType>: ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
		LCD_ShowString(10,140,"<Node ID>:  ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
		LCD_ShowString(10,170,"<SlotTime>: ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);

		LCD_ShowString(150,50,tStuDtc.Name,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
		switch((unsigned char)tStuDtc.DTCType)
		{
			case DTC_DOOR:
				LCD_ShowString(150,80,"door dtc",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);	
			break;
			case DTC_REMOTE:
				LCD_ShowString(150,80,"remote",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);		
			break;						
		}
		if(tStuDtc.DTCType != DTC_REMOTE)
		{
			switch((unsigned char)tStuDtc.ZoneType)
			{
				case ZONE_TYP_24HOURS:
					LCD_ShowString(150,110,"24 hrs",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);	
				break;
				case ZONE_TYP_1ST:
					LCD_ShowString(150,110,"1ST",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);		
				break;			
				case ZONE_TYP_2ND:
					LCD_ShowString(150,110,"2ND",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);		
				break;			
			}
		}
		
		
		StringhexToAsciiConversion(&tStuDtc.node[0],temp,2); ///temp[0] temp1  temp2 temp3
		
		temp[4] = 0;
		LCD_ShowString(150,140,temp,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
		
		//HexToAscii(tStuDtc.sleepTimes,temp,12);
		
		temp[0] = tStuDtc.sleepTimes/3600;  ///H 00:00 
		temp[1] = tStuDtc.sleepTimes%3600/60;
		temp[2] = temp[0]/10 + '0';
		temp[3] = temp[0]%10 + '0';	
		temp[4] = ':';
		temp[5] = temp[1]/10 + '0';
		temp[6] = temp[1]%10 + '0';	
		temp[7] = 0;
		temp[8] = 0;			
		LCD_ShowString(150,170,&temp[2],HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);	
		
		StringhexToAsciiConversion(&tStuDtc.Code[0],temp,12); //24 0
		temp[24] = 0;
		LCD_ShowString(20,200,temp,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
	}	

	if(pModeMenu->keyVal != 0xff)
	{////
		keys = pModeMenu->keyVal;
		pModeMenu->keyVal = 0xFF;	//
		state = pModeMenu->state;
		if(state == KEY_CLICK)
		{
			switch(keys)
			{
				case KEY_RETURN_DAIL:	//返回按键
				case KEY_MENU:///确定按键
					pModeMenu = &DL_ZX_Review[STG_MENU_DL_ZX_REVIEW_MAIN];
					pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
				break;				
					////pModeMenu = pModeMenu->pParent;
					// pModeMenu = &DL_ZX_Review[STG_MENU_DL_ZX_REVIEW_MAIN];
					// pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
				// break;
			}		
		}
	}
}






//探测器属性修改UI界面函数

static void stgMenu_dl_EditCBS(void)
{
 	unsigned char keys,state;
	static Stu_DTC tStuDtc;
	unsigned char temp[25];
	static unsigned short timer = 0;      ///跳出当前界面的计时器变量
	static unsigned char editComplete = 0;    ///操作完成了  1
	static unsigned char setValue=0;
	static unsigned char *pDL_ZX_Edit_ZoneType_Val[STG_DEV_AT_SUM] =
	{
		"24 hrs ",
		"1ST    ",
		"2ND    ",
	};	
	if(pModeMenu->refreshScreenCmd == SCREEN_CMD_RESET)
	{//UI界面初始化
		pModeMenu->refreshScreenCmd = SCREEN_CMD_NULL;
		
		if(CheckPresenceofDtc(pModeMenu->reserved))
		{
			GetDtcStu(&tStuDtc,pModeMenu->reserved);
		}
		hal_Tftlcd_Clear();
		LCD_ShowString(130,10,pModeMenu->pModeType,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
		LCD_ShowString(10,50,"<Name>: ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
		LCD_ShowString(10,80,"<Type>: ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
		LCD_ShowString(10,110,"<ZoneType>: ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
		LCD_ShowString(10,140,"<Node ID>:  ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
		LCD_ShowString(10,170,"<Mac ID>:   ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
		
		LCD_ShowString(150,50,tStuDtc.Name,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
		
		switch((unsigned char)tStuDtc.DTCType)
		{
			case DTC_DOOR:
				LCD_ShowString(150,80,"door dtc",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);	
			break;
			case DTC_REMOTE:
				LCD_ShowString(150,80,"remote",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);		
			break;					
		}
		if(tStuDtc.DTCType != DTC_REMOTE)
		{
			switch((unsigned char)tStuDtc.ZoneType)
			{
				case ZONE_TYP_24HOURS:
					LCD_ShowString(150,110,"24 hrs",HUE_LCD_FONT,HUE_FONT_BACK,FORTSIZE_24,0);	
				break;
				case ZONE_TYP_1ST:
					LCD_ShowString(150,110,"1ST   ",HUE_LCD_FONT,HUE_FONT_BACK,FORTSIZE_24,0);		
				break;			
				case ZONE_TYP_2ND:
					LCD_ShowString(150,110,"2ND   ",HUE_LCD_FONT,HUE_FONT_BACK,FORTSIZE_24,0);		
				break;			
			}
		}
		setValue = tStuDtc.ZoneType;
		
		StringhexToAsciiConversion(tStuDtc.node,temp,2);
		temp[4] = 0;
		LCD_ShowString(150,140,temp,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
		
		
		temp[0] = tStuDtc.sleepTimes/3600;  ///H 00:00 
		temp[1] = tStuDtc.sleepTimes%3600/60;
		temp[2] = temp[0]/10 + '0';
		temp[3] = temp[0]%10 + '0';	
		temp[4] = ':';
		temp[5] = temp[1]/10 + '0';
		temp[6] = temp[1]%10 + '0';	
		temp[7] = 0;
		temp[8] = 0;			
		LCD_ShowString(150,170,&temp[2],HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);	

		StringhexToAsciiConversion(tStuDtc.Code,temp,12);
		temp[24] = 0;
		LCD_ShowString(20,200,temp,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);		
		
		editComplete = 0;
		timer = 0;		
	}	

	if(pModeMenu->keyVal != 0xff)
	{
		keys = pModeMenu->keyVal;
		pModeMenu->keyVal = 0xFF;	//
		state = pModeMenu->state;
		if(state == KEY_CLICK)
		{
			switch(keys)
			{
				case KEY6_RIGHT:
					if(tStuDtc.DTCType != DTC_REMOTE)
					{		
						//设置Zone type   右按键  0 1 2
						if(setValue == (STG_DEV_AT_SUM-1))
						{
							setValue = 0;
						}
						else
						{
							setValue++;
						}
						tStuDtc.ZoneType = (ZONE_TYPED_TYPEDEF)setValue;			////更新探测器参数
						LCD_ShowString(150,110,pDL_ZX_Edit_ZoneType_Val[setValue],HUE_LCD_FONT,HUE_FONT_BACK,FORTSIZE_24,0);
					}
				break;
				case KEY4_LEFT:
				{///左按键
					//设置Zone type
					if(tStuDtc.DTCType != DTC_REMOTE)
					{
						if(setValue == ZONE_TYP_24HOURS)
						{
							setValue = STG_DEV_AT_SUM-1;
						}else
						{
							setValue--;
						}
						tStuDtc.ZoneType = (ZONE_TYPED_TYPEDEF)setValue;			////更新探测器参数
						LCD_ShowString(150,110,pDL_ZX_Edit_ZoneType_Val[setValue],HUE_LCD_FONT,HUE_FONT_BACK,FORTSIZE_24,0);
				
					}
				}
				break;
				case KEY_RETURN_DAIL:	//
					pModeMenu = &DL_ZX_Review[STG_MENU_DL_ZX_REVIEW_MAIN];
					pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
				break;
				
				case KEY_MENU:
				if(tStuDtc.DTCType != DTC_REMOTE)
				{
					timer = 0;
					editComplete = 1;
					SetDtcAbt(tStuDtc.ID-1,&tStuDtc);		//更新探测器属性，并写入EEPROM

					hal_Tftlcd_Clear();
					LCD_ShowString(60,100,"Update..",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
				}
				break;
			}		
		}
	}
	if(editComplete)
	{
		timer++;
		if(timer > 150)		//1.5
		{
			timer = 0;
			editComplete = 0;
			pModeMenu = &DL_ZX_Review[STG_MENU_DL_ZX_REVIEW_MAIN];
			pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
		}
	}
}



///探测器属性 删除探测器UI界面函数
static void stgMenu_dl_DeleteCBS(void)
{
  unsigned char keys,state;
	static unsigned short timer = 0;
	static unsigned char Complete = 0;
	static unsigned char stgMainMenuSelectedPos=0; ///光标的位置  1代表YES   0 NO
	static Stu_DTC tStuDtc;
	if(pModeMenu->refreshScreenCmd == SCREEN_CMD_RESET)
	{	//界面初始化
		pModeMenu->refreshScreenCmd = SCREEN_CMD_NULL;
		
		if(CheckPresenceofDtc(pModeMenu->reserved))
		{
			GetDtcStu(&tStuDtc,pModeMenu->reserved);
		}
		
		hal_Tftlcd_Clear();
		LCD_ShowString(110,20,pModeMenu->pModeType,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
		LCD_ShowString(40,80,"Del ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
		LCD_ShowString(80,80,tStuDtc.Name,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
		LCD_ShowString(40,110,"Are you sure?",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
		LCD_ShowString(80,150,"Yes",HUE_LCD_FONT,HUE_FONT_BACK,FORTSIZE_24,0);
		LCD_ShowString(200,150,"No",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
		stgMainMenuSelectedPos = 1;
		timer = 0;
		Complete = 0;
	}	

	if(pModeMenu->keyVal != 0xff)
	{
		keys = pModeMenu->keyVal;
		pModeMenu->keyVal = 0xFF;	//

		state = pModeMenu->state;
		if(!Complete)
		{
			if(state == KEY_CLICK)
			{
				switch(keys)
				{
					case KEY_RETURN_DAIL:	//返回操作
						pModeMenu = pModeMenu->pParent;
						pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
					break;
					case KEY4_LEFT:
					case KEY6_RIGHT:
					{
								if(stgMainMenuSelectedPos == 1)		
								{
									stgMainMenuSelectedPos = 0;
									LCD_ShowString(80,150,"Yes",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
									LCD_ShowString(200,150,"No",HUE_LCD_FONT,HUE_FONT_BACK,FORTSIZE_24,0);
								}
								else if(stgMainMenuSelectedPos == 0)
								{
									stgMainMenuSelectedPos = 1;
									LCD_ShowString(80,150,"Yes",HUE_LCD_FONT,HUE_FONT_BACK,FORTSIZE_24,0);
									LCD_ShowString(200,150,"No",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
								}						
					}
					break;
					case KEY_MENU:
					{
							if(stgMainMenuSelectedPos)
							{
								Complete = 1;
								timer = 0;
								tStuDtc.Mark = 0;
								SetDtcAbt(tStuDtc.ID-1,&tStuDtc);		//更新探测器属性，并写入EEPROM
								hal_Tftlcd_Clear();
								LCD_ShowString(60,100,"Update..",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
							}
							else
							{
								pModeMenu = pModeMenu->pParent;
								pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER; 
							}
					}
				}
			}		
		}
	}
	if(Complete)
	{
		timer++;
		if(timer > 150)		//1.5
		{
			timer = 0;
			Complete = 0;
			pModeMenu = pModeMenu->pParent;
			pModeMenu->refreshScreenCmd = SCREEN_CMD_RESET;
		}
	}
}






#define COOR_MENU_X   24
///主菜单
static void stgMenu_MainMenuCBS(void)
{
	//unsigned char keys,state;
	unsigned char keys;  //按键值
	unsigned char state;  //按键的操作
	unsigned char i;
	unsigned char ClrScreenFlag;//是否清屏   1 表示需要清零
	static stu_mode_menu *pMenu;  //保存当前选择的菜单
	static stu_mode_menu *bpMenu=0;	//备份当前选择的菜单
	static unsigned char stgMainMenuSelectedPos=0;	//目前主菜单选择的行数
	static stu_mode_menu *MHead;  //菜单显示的第一行内容	
	if(pModeMenu->refreshScreenCmd == SCREEN_CMD_RESET)
	{   
		pModeMenu->refreshScreenCmd = SCREEN_CMD_NULL;
		hal_Tftlcd_Clear();
		LCD_ShowString(0,0,settingModeMenu[STG_MENU_MAIN_SETTING].pModeType,HUE_LCD_FONT,HUE_FONT_BACK,32,0);

		pMenu = &settingModeMenu[1];//指向像一个菜单		
		MHead = pMenu;			//第一行显示
		bpMenu = 0;
		
		ClrScreenFlag = 1;
		stgMainMenuSelectedPos = 1;
		keys = 0xFF;
	
	}
	else if(pModeMenu->refreshScreenCmd == SCREEN_CMD_RECOVER)
	{
		pModeMenu->refreshScreenCmd = SCREEN_CMD_NULL;
		hal_Tftlcd_Clear();
		LCD_ShowString(0,0,settingModeMenu[STG_MENU_MAIN_SETTING].pModeType,HUE_LCD_FONT,HUE_FONT_BACK,32,0);
		keys = 0xFF;
		ClrScreenFlag = 1;
		bpMenu = 0;
  }
		
	if(pModeMenu->keyVal != 0xff)
	{
		keys = pModeMenu->keyVal;
		pModeMenu->keyVal = 0xFF;	//

		state = pModeMenu->state;
		if(state == KEY_CLICK)
		{
			switch(keys)
			{
				case KEY2_UP:		//
				if(stgMainMenuSelectedPos ==1)
				{
					MHead = MHead->pLase;
					pMenu = pMenu->pLase;
					stgMainMenuSelectedPos = 1;
					ClrScreenFlag = 1;
				}
				else
				{
					LCD_ShowString(3,32*stgMainMenuSelectedPos," ",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
					pMenu = pMenu->pLase;
					stgMainMenuSelectedPos--;
				}
				break;
				
				case KEY8_DOWN:		//
				if(stgMainMenuSelectedPos ==6)
				{
					MHead = MHead->pNext;	
					pMenu = pMenu->pNext;
					stgMainMenuSelectedPos = 6;
					ClrScreenFlag = 1;
				}
				else
				{
					LCD_ShowString(3,32*stgMainMenuSelectedPos," ",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
					pMenu = pMenu->pNext;																		 
					stgMainMenuSelectedPos++;
				}
				break;
				
				case KEY_RETURN_DAIL:	//
					pModeMenu = &generalModeMenu[GNL_MENU_DESKTOP];;
					pModeMenu->refreshScreenCmd = SCREEN_CMD_RESET;
					mt_4g_DialHandup();
				break;
				case KEY_MENU:	//
					pModeMenu = pMenu;
					pModeMenu->refreshScreenCmd = SCREEN_CMD_RESET; 
				break;
			}
		}
	}
	
	
	if(bpMenu != pMenu)
	{///当前的界面需要更新
		bpMenu = pMenu;
		if(ClrScreenFlag)
		{///需要清屏操作
			ClrScreenFlag = 0;  //复位
			pMenu = MHead;

			for(i=1; i<7; i++)
			{
				LCD_ShowString(COOR_MENU_X,32*i,pMenu->pModeType,HUE_LCD_FONT,HUE_LCD_BACK,32,0);
				pMenu = pMenu->pNext;
			} 
			pMenu = bpMenu;   ///恢复当前选择的指针内容
			LCD_ShowString(3,32*stgMainMenuSelectedPos,">",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
		}
		else
		{ 
			LCD_ShowString(3,32*stgMainMenuSelectedPos,">",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
		}	 
	}	
}


//子菜单
static void stgMenu_LearnSensorCBS(void)///探测器学习配对
{
	unsigned char keys;  //按键值
	unsigned char state;  //按键的操作
	
	static unsigned short Timer = 0;
	static unsigned char PairingComplete = 0;	 ///= 1 表示界面操作完成 	
	if(pModeMenu->refreshScreenCmd == SCREEN_CMD_RESET)
	{   
		pModeMenu->refreshScreenCmd = SCREEN_CMD_NULL;
		hal_Tftlcd_Clear();
		LCD_ShowString(0,0,"   Learning DTC     ",HUE_LCD_FONT,HUE_FONT_BACK,32,0);
		LCD_ShowString(60,120,"Pairing...",HUE_LCD_FONT,HUE_LCD_BACK,32,0);		
		loraDtector.state = LORAMODE_APPLEY_NET;   ///修改探测器学习的模式   探测器申请入网
	    Timer = 0;
		PairingComplete = 0;///界面操作为完成		
		hal_Wtn6_PlayVolue(WTN6_STUDY_START); ///语言提示： 探测器可是配对 ，请触发探测器	
		keys = 0xFF;	
	}

	if(pModeMenu->keyVal != 0xff)
	{
		keys = pModeMenu->keyVal;
		pModeMenu->keyVal = 0xFF;	//

		state = pModeMenu->state;
		if(state == KEY_CLICK)
		{
			switch(keys)
			{
				case KEY_RETURN_DAIL:	// 按返回按键  返回到主菜单
					pModeMenu = &settingModeMenu[STG_MENU_MAIN_SETTING];;
					pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
					loraDtector.state = LORAMODE_NORMAL;					
				break;
			}
		}
	}	
	
	if((loraDtector.state == LORAMODE_APPLEY_NET_OK) && (!PairingComplete))
	{////探测器配对有响应操作
		loraDtector.state = LORAMODE_NORMAL;//模式修改成普通模式
		Timer = 0;
        PairingComplete = 1;
		
		switch(loraDtector.LearnSta)
		{
			case DET_LEARN_FAIL:
			{//探测器学习失败
				LCD_ShowString(60,70,"Apply net fail",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
				hal_Wtn6_PlayVolue(WTN6_STUDY_FAIL);  //语言提示
			}
			break;
			case DET_UNLEARN:
			{////探测器之前未学习  表示探测器学习成功
				LCD_ShowString(60,70,"Success!",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
				switch(loraDtector.detectorType)
				{
					case DTC_DOOR:
					{//门磁探测器
						LCD_ShowString(20,120,"Added door dtc..",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
						hal_Wtn6_PlayVolue(WTN6_STUDY_SUC);
					}
					break;
					case DTC_REMOTE:
					{//遥控器
						LCD_ShowString(20,120,"Added Remote dtc..",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
						hal_Wtn6_PlayVolue(WTN6_STUDY_SUC);
					}
					break;			
				}
			}
			break;
			case DET_HAVED_LEARN:
			{///探测器已经学习
				LCD_ShowString(60,70,"Apply net fail",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
				LCD_ShowString(8,120,"Detectors learned",HUE_LCD_FONT,HUE_LCD_BACK,32,0);	
				hal_Wtn6_PlayVolue(WTN6_HAVED_DETEC);		//语言提示  探测器已存在		
			}
			break;
			default:
			{///探测器已学习。
			}
			break;
		}
	}	
	
	
	
	
	
	if(PairingComplete)
	{///界面操作完成
		Timer++;   //延时计数
		if(Timer > 200)
		{///大于2秒
			PairingComplete = 0;
			Timer = 0;
			pModeMenu = pModeMenu->pParent;			// 
			pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
			loraDtector.state = LORAMODE_NORMAL;
		}
	}		
}



////////////////////////////////////////////////////////////////
//2. 探测器列表
static void stgMenu_DTCListCBS(void)
{
	unsigned char keys,state;
	unsigned char ClrScreenFlag;//是否清屏   1 表示需要清零
	unsigned char slotTimH,slotTimL;
	unsigned char i,j;
	Stu_DTC tStuDtc;
    static Stu_DTC StuDTCtemp[PARA_DTC_SUM];
	static stu_mode_menu settingMode_DTCList_Sub_Menu[PARA_DTC_SUM];
	static stu_mode_menu *pMenu;  ///当前选择的探测器ID
	static stu_mode_menu *bpMenu=0;		//用来备份上一次菜单选项，主要用于刷屏判断
	static unsigned char stgMainMenuSelectedPos=0;	//用来记录当前选中菜单的位置
	//static stu_mode_menu *MHead,*MTail;		//这两个结构是为了上下切换菜单时做翻页处理
	static stu_mode_menu *MHead;		////菜单显示的第一行内容
	static unsigned char pMenuIdx=0;	//用来动态指示菜单下标,最终这个就是已学习探测器的总数量	

	if(pModeMenu->refreshScreenCmd == SCREEN_CMD_RESET)
	{///UI界面的初始化操作
		pModeMenu->refreshScreenCmd = SCREEN_CMD_NULL;
		keys = 0xFF;
		hal_Tftlcd_Clear();
		LCD_ShowString(0,0,"       Dtc List     ",HUE_LCD_FONT,HUE_FONT_BACK,FORTSIZE_32,0);
		
		
		
		ClrScreenFlag = 1;
		pMenuIdx = 0;   ////O
 		pMenu = settingMode_DTCList_Sub_Menu;///O
		bpMenu = 0;
		stgMainMenuSelectedPos = 1;	
		MHead = pMenu;			//记录当前显示菜单第一项
		
		
		
	  //逐个判断，把已经学习的探测器找出来
		for(i=0; i<PARA_DTC_SUM; i++)
		{
		    if(CheckPresenceofDtc(i))
			{///dER0[i]P探测器已经学习  
				GetDtcStu(&tStuDtc,i);
				//settingMode_DTCList_Sub_Menu[1]
				(pMenu+pMenuIdx)->ID = pMenuIdx;
				(pMenu+pMenuIdx)->menuPos = STG_SUB_2_MENU_POS;
				(pMenu+pMenuIdx)->reserved = tStuDtc.ID-1;///表示的是探测器属性保存数组dER0 的下标值
				
				StuDTCtemp[pMenuIdx].ID = tStuDtc.ID-1;
				
				for(j=0; j<16; j++)
				{
					if(tStuDtc.Name[j] == 0)
					{
						StuDTCtemp[pMenuIdx].Name[j] = ' '; 
					}					
					else
					{
						StuDTCtemp[pMenuIdx].Name[j] = tStuDtc.Name[j];
					}
				}
				//on line
				//off-line			

				if(tStuDtc.DTCType != DTC_REMOTE)
				{
					
						slotTimH = tStuDtc.sleepTimes/3600;
						slotTimL = tStuDtc.sleepTimes%3600/60;
						
					 
						StuDTCtemp[pMenuIdx].Name[10] = slotTimH/10 + '0';
						StuDTCtemp[pMenuIdx].Name[11] = slotTimH%10 + '0';	
						StuDTCtemp[pMenuIdx].Name[12] = ':';
						StuDTCtemp[pMenuIdx].Name[13] = slotTimL/10 + '0';
						StuDTCtemp[pMenuIdx].Name[14] = slotTimL%10 + '0';	
						StuDTCtemp[pMenuIdx].Name[24]	= 0;					
						if(tStuDtc.sleepTimes > DEC_OFFLINE_TIME)
						{
								StuDTCtemp[pMenuIdx].Name[16]	= ' ';
								StuDTCtemp[pMenuIdx].Name[17]	= 'o';	
								StuDTCtemp[pMenuIdx].Name[18]	= 'f';
								StuDTCtemp[pMenuIdx].Name[19]	= 'f';
								StuDTCtemp[pMenuIdx].Name[20]	= 'l';	
								StuDTCtemp[pMenuIdx].Name[21]	= 'i';	
								StuDTCtemp[pMenuIdx].Name[22]	= 'n';	
								StuDTCtemp[pMenuIdx].Name[23]	= 'e';
								StuDTCtemp[pMenuIdx].Name[24]	= 0;
						}
						else
						{
								StuDTCtemp[pMenuIdx].Name[16]	= ' ';
								StuDTCtemp[pMenuIdx].Name[17]	= 'o';	
								StuDTCtemp[pMenuIdx].Name[18]	= 'n';	
								StuDTCtemp[pMenuIdx].Name[19]	= 'l';	
								StuDTCtemp[pMenuIdx].Name[20]	= 'i';	
								StuDTCtemp[pMenuIdx].Name[21]	= 'n';	
								StuDTCtemp[pMenuIdx].Name[22]	= 'e';	
							  StuDTCtemp[pMenuIdx].Name[23]	= ' ';
								StuDTCtemp[pMenuIdx].Name[24]	= 0;	
						}
				}	
				else
				{
								StuDTCtemp[pMenuIdx].Name[16]	= ' ';
								StuDTCtemp[pMenuIdx].Name[17]	= ' ';	
								StuDTCtemp[pMenuIdx].Name[18]	= ' ';	
								StuDTCtemp[pMenuIdx].Name[19]	= ' ';	
								StuDTCtemp[pMenuIdx].Name[20]	= ' ';	
								StuDTCtemp[pMenuIdx].Name[21]	= ' ';	
								StuDTCtemp[pMenuIdx].Name[22]	= ' ';	
							  StuDTCtemp[pMenuIdx].Name[23]	= ' ';
								StuDTCtemp[pMenuIdx].Name[24]	= 0;			
				}
				(pMenu+pMenuIdx)->pModeType = StuDTCtemp[pMenuIdx].Name;
				 pMenuIdx++;				
			}
		}
		
		
		
		if(pMenuIdx != 0)
		{
			//有探测器存在的情况
			if(pMenuIdx > 1)
			{
				//settingMode_DTCList_Sub_Menu[2];  0 1 
				//
				pMenu->pLase =  pMenu+(pMenuIdx-1);
				pMenu->pNext =  pMenu+1;
				pMenu->pParent = &settingModeMenu[STG_MENU_MAIN_SETTING];
				
				for(i=1; i<pMenuIdx-1; i++)
				{
					(pMenu+i)->pLase =  pMenu+(i-1);
					(pMenu+i)->pNext = pMenu+(i+1);
					(pMenu+i)->pParent = &settingModeMenu[STG_MENU_MAIN_SETTING];
				}
				(pMenu+(pMenuIdx-1))->pLase =  pMenu+(i-1);
				(pMenu+(pMenuIdx-1))->pNext = pMenu;
				(pMenu+(pMenuIdx-1))->pParent = &settingModeMenu[STG_MENU_MAIN_SETTING];
			}
			else if(pMenuIdx == 1)
			{
				pMenu->pLase = pMenu;
				pMenu->pNext = pMenu;
				pMenu->pParent = &settingModeMenu[STG_MENU_MAIN_SETTING];
			}
		}
		else
		{
			//没有探测器
			bpMenu = pMenu;
			LCD_ShowString(20,120,"No detector",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
		}
		MHead = pMenu;			//记录当前显示菜单第一项
	}
    else if(pModeMenu->refreshScreenCmd==SCREEN_CMD_RECOVER)
	{	///恢复当前菜单
		pModeMenu->refreshScreenCmd = SCREEN_CMD_NULL;
		hal_Tftlcd_Clear();
		LCD_ShowString(0,0,"       Dtc List     ",HUE_LCD_FONT,HUE_FONT_BACK,FORTSIZE_32,0);
		keys = 0xFF;
		ClrScreenFlag = 1;
		bpMenu = 0;
	}
	
	if(pModeMenu->keyVal != 0xff)
	{
		keys = pModeMenu->keyVal;
		pModeMenu->keyVal = 0xFF;	//

		state = pModeMenu->state;
		if(state == KEY_CLICK)
		{
			switch(keys)
			{
				case KEY_MENU:
				{///确定按键
					if(pMenuIdx>0)
					{
						pModeMenu = &DL_ZX_Review[STG_MENU_DL_ZX_REVIEW_MAIN]; 
						pModeMenu->reserved = pMenu->reserved;	//这里用于传递后面要查看、修改、删除探测器的ID号
						pModeMenu->refreshScreenCmd = SCREEN_CMD_RESET;
						//pModeMenu->pParent = &settingModeMenu[STG_MENU_DTC_LIST];;	
					}
				}
				break;
				case KEY_RETURN_DAIL:	//
					pModeMenu = pModeMenu->pParent;
					pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
				break;
				case KEY2_UP:		
						if(pMenuIdx < 2)
						{////只有1个探测器不做处理
						}
						else if(pMenuIdx < 7)
						{
							//只有一页，也就是只有4个探测器的时候
							if(stgMainMenuSelectedPos ==1)	//判断是否选中的是第一行
							{
							}
							else 
							{
								//不清屏，直接刷新局部显示
								LCD_ShowString(3,(32*stgMainMenuSelectedPos)," ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);//取消选中本菜单显示	
								pMenu = pMenu->pLase;
								stgMainMenuSelectedPos--;
							}
						}
						else
						{
							if(stgMainMenuSelectedPos ==1)
							{
								MHead = MHead->pLase;
								pMenu = pMenu->pLase;
								stgMainMenuSelectedPos = 1;
								ClrScreenFlag = 1;
							}
							else
							{
								LCD_ShowString(3,(32*stgMainMenuSelectedPos)," ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);//取消选中本菜单显示	
								pMenu = pMenu->pLase;
								stgMainMenuSelectedPos--;
							}
						}
					break;
					case KEY8_DOWN:		//
						if(pMenuIdx < 2)
						{////只有一个探测器不做处理
						
						}
						else if(pMenuIdx < 7)
						{//只有一页，也就是只有6个探测器的时候  1-6   1-6
							if(stgMainMenuSelectedPos ==pMenuIdx)	//判断是否选中的是第4行
							{
								//头尾指针不变，只把当前菜单指向下个
								LCD_ShowString(3,(32*stgMainMenuSelectedPos)," ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);//取消选中本菜单显示	
								pMenu = pMenu->pNext;
								stgMainMenuSelectedPos = 1;
								ClrScreenFlag = 1;
							}
							else 
							{
								//不清屏，直接刷新局部显示
								LCD_ShowString(3,(32*stgMainMenuSelectedPos)," ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);//取消选中本菜单显示	
								pMenu = pMenu->pNext;																			//切换下一个选项
								stgMainMenuSelectedPos++;
							}						
						}
						else
						{
							if(stgMainMenuSelectedPos ==6)
							{
								MHead = MHead->pNext;	
								pMenu = pMenu->pNext;
								stgMainMenuSelectedPos = 6;
								ClrScreenFlag = 1;
							}
							else
							{
								LCD_ShowString(3,(32*stgMainMenuSelectedPos)," ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);//取消选中本菜单显示	
								pMenu = pMenu->pNext;																			// 
								stgMainMenuSelectedPos++;
							}						
						}
						break;
			}
		}
	}

    if(bpMenu != pMenu)
	{
		bpMenu = pMenu;
		if(ClrScreenFlag)
		{
			ClrScreenFlag = 0;
			pMenu = MHead;
			if(pMenuIdx <7)
			{
				for(i=0; i<pMenuIdx; i++)
				{
					LCD_ShowString(COOR_MENU_X,(32*(i+1)),pMenu->pModeType,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
					pMenu = pMenu->pNext;
				}
			}
			else
			{///
				for(i=1; i<7; i++)
				{
					LCD_ShowString(COOR_MENU_X,(32*i),pMenu->pModeType,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
					pMenu = pMenu->pNext;
				} 
			}
			pMenu = bpMenu;
			
			LCD_ShowString(3,(32*stgMainMenuSelectedPos),">",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);	 
		}
		else
		{ 
			LCD_ShowString(3,(32*stgMainMenuSelectedPos),">",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
		}	
	} 
}








static void stgMenu_WifiCBS(void)//WIFI 配网
{
	static unsigned char APStep = 0;  ///WIFI配网操作 0 选择是否配网   1开始配网  2 获取WIFI密码 3 退出配网状态
    unsigned char keys,state;
	unsigned char wifiWorkState = 0;
	static unsigned char stgMainMenuSelectedPos = 0;   //表示选择的是YES   NO      0-yes     1-NO
	static unsigned int Time_DelayT = 0;
	
	if(pModeMenu->refreshScreenCmd == SCREEN_CMD_RESET)
	{///初始化界面 选择是否配网
		pModeMenu->refreshScreenCmd = SCREEN_CMD_NULL;
		hal_Tftlcd_Clear();
		LCD_ShowString(0,0,"        Wifi        ",HUE_LCD_FONT,HUE_FONT_BACK,32,0);	
		LCD_ShowString(0,60,"Are you sure to ",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
		LCD_ShowString(0,100,"get wifi password ?",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
		//yes   no
		LCD_ShowString(43,150,"Yes",HUE_LCD_FONT,HUE_FONT_BACK,32,0);
		LCD_ShowString(160,150,"No",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
		stgMainMenuSelectedPos = 0;
		
		keys = 0xFF;
		APStep = 0;
		Time_DelayT = 0;
	}
	
	
	
	if(pModeMenu->keyVal != 0xff)
	{
		keys = pModeMenu->keyVal;
		pModeMenu->keyVal = 0xFF;	// 
		state = pModeMenu->state;
		if(state == KEY_CLICK) 
		{
			switch(keys)
			{
				case KEY_RETURN_DAIL:	//返回操作按键
					pModeMenu = pModeMenu->pParent;
					pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
				    if(APStep)////
						mt_wifi_exit_SmartConfig();
				break;
				case KEY4_LEFT:
				case KEY6_RIGHT:
				{////左右按键选择是否进入配网
					  if(!APStep)
						{
							if(stgMainMenuSelectedPos == 0)		
							{///如果当前选择的是YES
								stgMainMenuSelectedPos = 1;
								LCD_ShowString(43,150,"Yes",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
								LCD_ShowString(160,150,"No",HUE_LCD_FONT,HUE_FONT_BACK,32,0);
							}
							else if(stgMainMenuSelectedPos == 1)
							{
								stgMainMenuSelectedPos = 0;
								LCD_ShowString(43,150,"Yes",HUE_LCD_FONT,HUE_FONT_BACK,32,0);
								LCD_ShowString(160,150,"No",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
							}						
						}
				}
				break;
				case KEY_MENU:
				{
					if(!APStep)
					{
						if(stgMainMenuSelectedPos == 0)		
						{  ////	
							LCD_ShowString(0,60,"                  ",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
							LCD_ShowString(0,100,"Get Wifi PassWord... ",HUE_LCD_FONT,HUE_LCD_BACK,32,0);	

							LCD_ShowString(40,150,"  Waiting...      ",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
							if(mt_wifi_get_wifi_work_state() >= ESP12_STA_DETEC_STA)
							{////进入WIFI配网模式
								mt_wifi_changState(ESP12_STA_GET_PASSWORD);  ///
							}
							APStep = 1;
							Time_DelayT = 0;

							hal_Wtn6_PlayVolue(WTN6_GET_WIFI_PASSWORD);////
						}
						else
						{
							pModeMenu = pModeMenu->pParent;
							pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
						}
					}
				}
				break;
			}
		}
	}
	
	
	Time_DelayT++;
	wifiWorkState = 0;
	if(APStep)
	{
	    wifiWorkState = mt_wifi_get_wifi_work_state();    
	}
	
	
	switch(APStep)
	{
		case 1:
		{
			if(wifiWorkState == ESP12_STA_GET_SMART_WIFINFO)
			{
				LCD_ShowString(0,150,"Smart Get WIFI info     ",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
				APStep = 2;
				Time_DelayT = 0;
			}
			else if(wifiWorkState == ESP12_STA_DETEC_STA)
			{
				mt_wifi_changState(ESP12_STA_GET_PASSWORD);
			}
			if(Time_DelayT > 6000)
			{///
				APStep = 3;
				Time_DelayT = 0;
				LCD_ShowString(0,100,"Wait for a timeout       ",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
				LCD_ShowString(0,150,"The operation failure    ",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
				hal_Wtn6_PlayVolue(WTN6_WIFI_TIMEOUT); //
			}			
		}
		break;
		case 2:
		{
			if(wifiWorkState == ESP12_STA_GETING_SUC)
			{
				hal_Wtn6_PlayVolue(WTN6_GET_WIFI_OK);
				LCD_ShowString(0,150,"Connect to router ok.    ",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
				APStep = 3;
				Time_DelayT = 0;
			}
			if(Time_DelayT > 6000)
			{///
				APStep = 3;
				Time_DelayT = 0;
				LCD_ShowString(0,100,"Wifi password error      ",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
				LCD_ShowString(0,150,"The operation failure    ",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
				hal_Wtn6_PlayVolue(WTN6_GET_WIFI_FAIL);
			}				
		}
		break;
		case 3:
		{
			if(Time_DelayT > 300)
			{/// 
					pModeMenu = pModeMenu->pParent;
					pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
					mt_wifi_exit_SmartConfig();
			}	
		}
		break;
	}
};



//系统密码修改界面
static void stgMenu_PasswordCBS(void)
{
	unsigned char pdVal = 0;  ///按键值
	unsigned char keyPressFlag = 0;///是否有有效数字按键按下
	unsigned char i;
	static unsigned char xCursorPos = 0;			//按键输入的密码个数
	static unsigned char yCursorPos = 0;			//Y坐标  70表示第一次输入密码  110表示第二次输入密码

	static unsigned short bLinkTimer = 0;      //光标闪烁间隔时间计时 0
	static unsigned char bLinkFlag = 0;        //光标闪烁状态 0
	
	static unsigned short timer=0;   //延时
	static unsigned char updateParaFlag=0;    //0正在输入密码  1 表示2次输入的密码一样 修改密码成功  2 表示两次输入的密码不一致，密码修改失败
	static unsigned char password1[4];  //保存输入的按键密码 
	static unsigned char password2[4];  //备份第1次输入的密码
    unsigned char keys,state;//按键相关的
	
	if(pModeMenu->refreshScreenCmd == SCREEN_CMD_RESET)
	{///初始化界面 部分代码
		pModeMenu->refreshScreenCmd = SCREEN_CMD_NULL;

		hal_Tftlcd_Clear();	
		
		LCD_ShowString(0,0,"      Password      ",HUE_LCD_FONT,HUE_FONT_BACK,32,0);		
		LCD_ShowString(10,70, "New    :",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
		LCD_ShowString(10,110,"Confirm:",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
		bLinkTimer = 0;
		bLinkFlag = 0;		
		keys = 0xff;
		
		xCursorPos = 0;
		yCursorPos = 70;
		

		updateParaFlag = 0;

		password1[0] = 0;
		password1[1] = 0;
		password1[2] = 0;
		password1[3] = 0;
		
		password2[0] = 0;
		password2[1] = 0;
		password2[2] = 0;
		password2[3] = 0;			
		timer = 0;	
	} 
	if(!updateParaFlag)   
	{
		
		//////
		bLinkTimer++;
		if(bLinkTimer > 30)
		{//310ms
			bLinkTimer = 0;
			bLinkFlag = !bLinkFlag;
			
			if(bLinkFlag)
			{
				LCD_ShowString(150+32*xCursorPos,yCursorPos," ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0); 
			}
			else
			{
				LCD_ShowString(150+32*xCursorPos,yCursorPos,"_",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0); 
			}			
		}
		

		////// 
		
		if(pModeMenu->keyVal != 0xff)
		{
			keys = pModeMenu->keyVal;
			pModeMenu->keyVal = 0xFF;	// 

			state = pModeMenu->state;
			if(state == KEY_CLICK)
			{
				switch(keys)
				{
					case KEY_RETURN_DAIL:	// 返回主菜单操作
						pModeMenu = pModeMenu->pParent;
						pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
					break;
 
					case KEY0:
							pdVal = '0';
							keyPressFlag = 1;
					break;
					case KEY1:
							pdVal = '1';
							keyPressFlag = 1;
					break;	
					case KEY2_UP:
							pdVal = '2';
							keyPressFlag = 1;
					break;
					case KEY3:
							pdVal = '3';
							keyPressFlag = 1;
					break;		
					case KEY4_LEFT:
							pdVal = '4';
							keyPressFlag = 1;
					break;
					case KEY5:
							pdVal = '5';
							keyPressFlag = 1;
					break;				
					case KEY6_RIGHT:
							pdVal = '6';
							keyPressFlag = 1;
					break;
					case KEY7:
							pdVal = '7';
							keyPressFlag = 1;
					break;	
					case KEY8_DOWN:
							pdVal = '8';
							keyPressFlag = 1;
					break;
					case KEY9:
							pdVal = '9';
							keyPressFlag = 1;
					break;	
					case KEY_MENU:
					{
						if(yCursorPos==70)		//????????????
						{
							if(xCursorPos == 4)		//????????4?
							{
								LCD_ShowString(150+32*xCursorPos,yCursorPos," ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0); 	
								yCursorPos = 110;
								xCursorPos = 0;
								
								
								password2[0] = password1[0];
								password2[1] = password1[1];
								password2[2] = password1[2];
								password2[3] = password1[3];
								
								password1[0] = ' ';
								password1[1] = ' ';
								password1[2] = ' ';
								password1[3] = ' ';
							}
						}
						else					//????????
						{
							if((password2[0]==password1[0])
								 && (password2[1]==password1[1])
								 && (password2[2]==password1[2])
								 && (password2[3]==password1[3]))		//?????????????
							{
								timer = 0;
								updateParaFlag = 1;
								LCD_ShowString(10,70, "     Save          ",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
								LCD_ShowString(10,110,"     Update        ",HUE_LCD_FONT,HUE_LCD_BACK,32,0);

							}
							else
							{
								timer = 0;
								updateParaFlag = 2;
							    LCD_ShowString(10,70, "                      ",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
								LCD_ShowString(10,110,"     Invalid Pin      ",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
							}
						}
					}
					break;
					case KEY_SOS_DEL:
					{
						if(xCursorPos>0)
						{///已经输入了密码才支持删除操作
							LCD_ShowString(150+32*xCursorPos,yCursorPos," ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0); 
							password1[xCursorPos] = ' ';
							xCursorPos--;
						}
					}
					break;	
				}
			}
		}
	}
	else
	{
		if(timer<121)
		{//1.2
			timer++;
		}
		else
		{
			timer = 0;
			if(updateParaFlag == 1)
			{////密码正确
				updateParaFlag = 0;

				for(i=0; i<4; i++)
				{
					MT_SET_PARA_SYS_ADMINPASSWORD(i,password1[i]-'0');
				}
				I2C_PageWrite(STU_SYSTEMPARA_OFFSET,(unsigned char*)(&mt_sEPR),sizeof(mt_sEPR));	
				pModeMenu = pModeMenu->pParent;	 
				pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
			}
			else if(updateParaFlag == 2)
			{///密码错误 重新输入
				updateParaFlag = 0;
				pModeMenu->refreshScreenCmd = SCREEN_CMD_RESET;
			}
		}
	}
 
	
	if(xCursorPos < 4)
	{///输入的密码个数不够4位
		if(keyPressFlag) 
		{
			keyPressFlag = 0;
			
			LCD_ShowString(150+32*xCursorPos,yCursorPos,"*",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0); 
			password1[xCursorPos] = pdVal;
			xCursorPos++;
		
			bLinkTimer = 0; 
		}
	}
	else
	{
		keyPressFlag = 0;
	}
}
//报警电话号码设置
///////////////////////
void stgMenu_PhoneNumberCBS(void)
{
  unsigned char keys,state;
	unsigned char i,j,ClrScreenFlag=0;
	static stu_mode_menu *MHead;//,*MTail;		// 
	static stu_mode_menu *pMenu;  ///当前选择的菜单
	static stu_mode_menu *bpMenu=0;	//备份当前选择的菜单
	static unsigned char stgMainMenuSelectedPos=0;	
		
	if(pModeMenu->refreshScreenCmd == SCREEN_CMD_RESET)
	{////界面初始化操作
		pModeMenu->refreshScreenCmd = SCREEN_CMD_NULL;

		pMenu = &PN_Submenu[0];
		MHead = pMenu;
		bpMenu = 0;
		ClrScreenFlag = 1;  //需要清屏
		stgMainMenuSelectedPos = 1;
		
		for(i=0; i<STG_MENU_PN_SUM; i++)
		{
			for(j=0; j<19; j++)
			{   
				if(MT_GET_PARA_SYS_PHONENUMBER(i,j)<10)		//
				{
					PNModeTypeAry[i][2+j] = MT_GET_PARA_SYS_PHONENUMBER(i,j)+'0';	//
				}
				else
				{
					PNModeTypeAry[i][2+j] =  '\0';
					break;
				}
			} 
			PNModeTypeAry[i][20] = '\0';		//
		}
		
		hal_Tftlcd_Clear();
		LCD_ShowString(0,0,"    Phone Number    ",HUE_LCD_FONT,HUE_FONT_BACK,32,0);			
	}
	else if(pModeMenu->refreshScreenCmd == SCREEN_CMD_RECOVER)
	{
		pModeMenu->refreshScreenCmd = SCREEN_CMD_NULL;
		
		hal_Tftlcd_Clear();
		LCD_ShowString(0,0,"    Phone Number    ",HUE_LCD_FONT,HUE_FONT_BACK,32,0);	
		//LCD_ShowString(100,0,"Phone Number",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
		
		bpMenu = 0;			//
		ClrScreenFlag = 1;		//
	}	
	if(pModeMenu->keyVal != 0xff)
	{
		keys = pModeMenu->keyVal;
		pModeMenu->keyVal = 0xFF;	//

		state = pModeMenu->state;
		if(state == KEY_CLICK)
		{
			switch(keys)
			{
				case KEY_RETURN_DAIL:	//按键返回 返回主菜单
					pModeMenu = pModeMenu->pParent;
					pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
				break;
				
					case KEY2_UP:
						if(stgMainMenuSelectedPos ==1)
						{
						}
						else
						{
							LCD_ShowString(3,10+32*stgMainMenuSelectedPos," ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
							pMenu = pMenu->pLase;
							stgMainMenuSelectedPos--;
						}  
				break;
				case KEY8_DOWN:
						if(stgMainMenuSelectedPos ==6)
						{
							
						}
						else
						{
							LCD_ShowString(3,10+32*stgMainMenuSelectedPos," ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
							pMenu = pMenu->pNext;																			//
							stgMainMenuSelectedPos++;
						}
				break;	
				case KEY_MENU:
				    pModeMenu = pMenu;
			      pModeMenu->refreshScreenCmd = SCREEN_CMD_RESET;	
				break;
				
				
			}
		}
	}
		
	if(bpMenu != pMenu)
	{
		bpMenu = pMenu;
		if(ClrScreenFlag)		//????????
		{
			ClrScreenFlag = 0;
			pMenu = MHead;
			for(i=1; i<7; i++)
			{
				LCD_ShowString(24,10+32*i,pMenu->pModeType,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
				pMenu = pMenu->pNext;
			} 
			pMenu = bpMenu;	
			LCD_ShowString(3,10+32*stgMainMenuSelectedPos,">",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
		}
		else
		{
			LCD_ShowString(3,10+32*stgMainMenuSelectedPos,">",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);		
		}
	}	
};

////修改报警电话号码操作界面
static void stgMenu_PN_EnterNbCBS(void)
{
	static unsigned short timer=0;
	static unsigned char updateParaFlag=0;  ///输入电话号码的状态 0号码输入中  1表示按了确定按键 停止号码输入
	unsigned char i,pdVal = 0;   ///
	unsigned char keyPressFlag = 0;
	
	static unsigned char CursorPos = 0;	///表示电话号码的长度
	
	static unsigned short bLinkTimer = 0;  ///光标的显示
	static unsigned char bLinkFlag = 0;
	
	static unsigned char PNModeTypeAryTemp[24];///用来保存编辑的电话号码的
	unsigned char keys,state;
	if(pModeMenu->refreshScreenCmd == SCREEN_CMD_RESET)
	{////界面的初始化
		pModeMenu->refreshScreenCmd = SCREEN_CMD_NULL;	
		hal_Tftlcd_Clear();
		LCD_ShowString(70,20,"Phone Number",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
		
		memset(PNModeTypeAryTemp,0,sizeof(PNModeTypeAryTemp));	//对数组PNModeTypeAryTemp 的值清0  
		memcpy(PNModeTypeAryTemp,&PNModeTypeAry[pModeMenu->ID],sizeof(PNModeTypeAryTemp));

		
		CursorPos = 0;	
		bLinkTimer = 0;
		bLinkFlag = 0;
		keyPressFlag = 0;
		updateParaFlag = 0;
		timer = 0; 
		LCD_ShowString(5,108,&PNModeTypeAryTemp[2],HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
		////计算电话号码的长度
		for(i=0; i<20;i++)
		{
			if((PNModeTypeAryTemp[i+2]==' ') || (PNModeTypeAryTemp[i+2]=='\0'))
			{
				CursorPos = i;				// 
				break;
			}
		}			
	}
	
	if(!updateParaFlag)
	{////正在输入
        ///光标闪烁
		bLinkTimer++;
		if(bLinkTimer > 30)
		{
			bLinkTimer = 0;
			bLinkFlag = !bLinkFlag;
			if(bLinkFlag)
			{
				LCD_ShowString(5+16*CursorPos,108," ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
			}
			else
			{
				LCD_ShowString(5+16*CursorPos,108,"_",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
			}			
		}
		if(pModeMenu->keyVal != 0xff)
		{
			keys = pModeMenu->keyVal;
			pModeMenu->keyVal = 0xFF;	//
			state = pModeMenu->state;
			if(state == KEY_CLICK)
			{
				switch(keys)
				{
					case KEY_RETURN_DAIL:	//
						pModeMenu = &settingModeMenu[STG_MENU_PHONE_NUMBER];
						pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
					break;
 
					case KEY0:
							pdVal = '0';
							keyPressFlag = 1;
					break;
					case KEY1:
							pdVal = '1';
							keyPressFlag = 1;
					break;	
					case KEY2_UP:
							pdVal = '2';
							keyPressFlag = 1;
					break;
					case KEY3:
							pdVal = '3';
							keyPressFlag = 1;
					break;		
					case KEY4_LEFT:
							pdVal = '4';
							keyPressFlag = 1;
					break;
					case KEY5:
							pdVal = '5';
							keyPressFlag = 1;
					break;				
					case KEY6_RIGHT:
							pdVal = '6';
							keyPressFlag = 1;
					break;
					case KEY7:
							pdVal = '7';
							keyPressFlag = 1;
					break;	
					case KEY8_DOWN:
							pdVal = '8';
							keyPressFlag = 1;
					break;
					case KEY9:
							pdVal = '9';
							keyPressFlag = 1;
					break;	
					case KEY_SOS_DEL:
					{
						if(CursorPos>0)
						{
							LCD_ShowString(5+16*CursorPos,108," ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
							CursorPos--;
							PNModeTypeAryTemp[2+CursorPos] = 0;
						}
					}
					break;
					case KEY_MENU:		//??
						timer = 0;
						updateParaFlag = 1;
					  LCD_ShowString(5,108,"Save,Update...    ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
					break;					
				}	
			}
		}
	}
	else
	{////按键确定按键 保存设置的电话号码
		if(timer<100)
		{
			timer++;
		}
		else
		{
			timer = 0;
			updateParaFlag = 0;
			
			for(i=0; i<20; i++)
			{
				if((PNModeTypeAryTemp[2+i]>='0')&&(PNModeTypeAryTemp[2+i]<='9'))
				{
					MT_SET_PARA_SYS_PHONENUMBER(pModeMenu->ID,i,PNModeTypeAryTemp[2+i]-'0');	//??????
				}
				else
				{
					MT_SET_PARA_SYS_PHONENUMBER(pModeMenu->ID,i,0xFF);	//电话号码的号码是无效的
				}
			}
		    memcpy(&PNModeTypeAry[pModeMenu->ID],PNModeTypeAryTemp,sizeof(PNModeTypeAryTemp));	//
			I2C_PageWrite(STU_SYSTEMPARA_OFFSET,(unsigned char*)(&mt_sEPR),sizeof(mt_sEPR));	
			
			pModeMenu = &settingModeMenu[STG_MENU_PHONE_NUMBER];
			pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
		}
	}
	
	
	if(CursorPos < 16)
	{///输入电话号码处理
		if(keyPressFlag) 
		{
			keyPressFlag = 0;

			LCD_ShowString(5+16*CursorPos,108,&pdVal,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
			PNModeTypeAryTemp[2+CursorPos] = pdVal;
			CursorPos++;
			bLinkTimer = 0; 
		}
	}
	else
	{
		keyPressFlag = 0; 
	}
}
////设备信息 UI界面
static void stgMenu_MachineInfoCBS(void)
{
  unsigned char keys,state;
	unsigned char madid[13],i,dat;
	unsigned short versionDat;
	if(pModeMenu->refreshScreenCmd == SCREEN_CMD_RESET)
	{
		pModeMenu->refreshScreenCmd = SCREEN_CMD_NULL;
		hal_Tftlcd_Clear();
		LCD_ShowString(0,0,"      Mac infog     ",HUE_LCD_FONT,HUE_FONT_BACK,32,0);
		
		LCD_ShowString(10,50, "<firmware ver>:V   ",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
		versionDat =  (MT_GET_PARA_SYS_FIRMWARE(0) << 8) + MT_GET_PARA_SYS_FIRMWARE(1);
		i = 0;
		if(versionDat > 1000)		
			madid[i++] = (versionDat % 10000)/1000 + '0';
		madid[i++] = (versionDat % 1000)/100 + '0';
		madid[i++] = '.'; 
		madid[i++] = (versionDat % 100)/10 + '0';
		madid[i++] = versionDat % 10 + '0';
		madid[i++] = 0;
		LCD_ShowString(205,50,madid,HUE_LCD_FONT,HUE_LCD_BACK,24,0);
		LCD_ShowString(10,90, "<protocol ID>:V",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
		i = 0;
		if(GatewayProtocolVer > 1000)		
			madid[i++] = (GatewayProtocolVer % 10000)/1000 + '0';
		madid[i++] = (GatewayProtocolVer % 1000)/100 + '0';
		madid[i++] = '.'; 
		madid[i++] = (GatewayProtocolVer % 100)/10 + '0';
		madid[i++] = GatewayProtocolVer % 10 + '0';
		madid[i++] = 0;	
		LCD_ShowString(193,90,madid,HUE_LCD_FONT,HUE_LCD_BACK,24,0);
		
		
		
		
		
		LCD_ShowString(10,130, "<hard ver>:V",HUE_LCD_FONT,HUE_LCD_BACK,24,0);	
        i = 0;		
		if(GatewayProtocolVer > 1000)		
			madid[i++] = (MCU_HARDVER_VERSION % 10000)/1000 + '0';
		madid[i++] = (MCU_HARDVER_VERSION % 1000)/100 + '0';
		madid[i++] = '.'; 
		madid[i++] = (MCU_HARDVER_VERSION % 100)/10 + '0';
		madid[i++] = MCU_HARDVER_VERSION % 10 + '0';
		madid[i++] = 0;		
		LCD_ShowString(160,130,madid,HUE_LCD_FONT,HUE_LCD_BACK,24,0);
		
		
		
    LCD_ShowString(10,170, "<mac ID>:",HUE_LCD_FONT,HUE_LCD_BACK,24,0);	
    for(i = 0;i<6;i++)
		{
			dat = ((MT_GET_MCU_UID(i) >> 4) & 0x0f);
		//	dat >>= 4;
			if(dat < 10)
			{//0-9  ///0X30 - 0X39 
				madid[2*i] = dat + '0';
			}
			else
			{///A-F 
				madid[2*i] = dat + 'A' -10;   //0X41
			}			
			dat = (MT_GET_MCU_UID(i) & 0x0f);
			if(dat < 10)
			{
				madid[2*i + 1] = dat + '0';
			}
			else
			{
				madid[2*i + 1] = dat + 'A' -10;
			}
		}
		madid[12] = 0;		
		LCD_ShowString(118,170, madid,HUE_LCD_FONT,HUE_LCD_BACK,24,0);	
    for(i = 0;i<12;i++)
		{
			dat = ((MT_GET_MCU_UID(i+6) >> 4) & 0x0f); //0X31  >>4  0X03
		//	dat >>= 4;
			if(dat < 10)
			{
				madid[2*i] = dat + '0';
			}
			else
			{
				madid[2*i] = dat + 'A' -10;
			}			
			dat = (MT_GET_MCU_UID(i+6) & 0x0f);
			if(dat < 10)
			{
				madid[2*i + 1] = dat + '0';
			}
			else
			{
				madid[2*i + 1] = dat + 'A' -10;
			}
		}
		madid[12] = 0;
        LCD_ShowString(118,200, madid,HUE_LCD_FONT,HUE_LCD_BACK,24,0);			
		keys = 0xFF; 
	}
	if(pModeMenu->keyVal != 0xff)
	{
		keys = pModeMenu->keyVal;
		pModeMenu->keyVal = 0xFF;	//

		state = pModeMenu->state;
		if(state == KEY_CLICK)
		{
			switch(keys)
			{
				case KEY_RETURN_DAIL:	//返回按键
					pModeMenu = pModeMenu->pParent;
					pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
				break;
			}
		}
	}
};





//////出厂设置
static void stgMenu_FactorySettingsCBS(void)
{
  unsigned char keys,state;
	static unsigned short timer = 0;
	static unsigned char Complete = 0;   //是否恢复出厂设置
	static unsigned char stgMainMenuSelectedPos=0;  //1表示YES
	
	if(pModeMenu->refreshScreenCmd == SCREEN_CMD_RESET)
	{	//ִ  界面初始化
		pModeMenu->refreshScreenCmd = SCREEN_CMD_NULL;
		
		hal_Tftlcd_Clear();
		LCD_ShowString(0,0,"  Default setting   ",HUE_LCD_FONT,HUE_FONT_BACK,32,0);
		
		LCD_ShowString(40,80,"Are you sure?",HUE_LCD_FONT,HUE_LCD_BACK,24,0);

		LCD_ShowString(80,150,"Yes",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
		LCD_ShowString(200,150,"No",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
		LCD_ShowString(60,150,">",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
		stgMainMenuSelectedPos = 1;  //表示YES
		timer = 0;
		Complete = 0;
	}	

	if(pModeMenu->keyVal != 0xff)
	{
		keys = pModeMenu->keyVal;
		pModeMenu->keyVal = 0xFF;	//�ָ��˵�����ֵ

		state = pModeMenu->state;
		if(!Complete)
		{
			if(state == KEY_CLICK)
			{
				switch(keys)
				{
					case KEY_RETURN_DAIL:	//返回按键
						pModeMenu = pModeMenu->pParent;
						pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
					break;
					case KEY4_LEFT:
					case KEY6_RIGHT:
					{
								if(stgMainMenuSelectedPos == 1)		
								{
									stgMainMenuSelectedPos = 0;
									LCD_ShowString(60,150," ",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
									LCD_ShowString(180,150,">",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
								}
								else if(stgMainMenuSelectedPos == 0)
								{
									stgMainMenuSelectedPos = 1;
									LCD_ShowString(60,150,">",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
									LCD_ShowString(180,150," ",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
								}						
					}
					break;
					case KEY_MENU:
					{
							if(stgMainMenuSelectedPos)
							{///yes
								Complete = 1;
								timer = 0;
								mt_para_FactoryReset();		//
								hal_Tftlcd_Clear();
								hal_Wtn6_PlayVolue(WTN6_TO_FACTORY);  ///恢复出厂设置的语言提示
								LCD_ShowString(60,100,"Update..",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
							}
							else
							{///NO
								pModeMenu = pModeMenu->pParent;
								pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER; 
							}
					}
				}
			}		
		}
	}
	if(Complete)
	{
		timer++;
		if(timer > 150)		//
		{///1.5
			timer = 0;
			Complete = 0;
			pModeMenu = pModeMenu->pParent;
			pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
		}
	}
}




static void stgMenu_AlarmRecordCBS(void)//报警记录
{
	unsigned char keys;  //按键值
	unsigned char state;  //按键的操作
	
	if(pModeMenu->refreshScreenCmd == SCREEN_CMD_RESET)
	{   
		pModeMenu->refreshScreenCmd = SCREEN_CMD_NULL;
		hal_Tftlcd_Clear();
	
		
		LCD_ShowString(0,0,"  Alarm Recording   ",HUE_LCD_FONT,HUE_FONT_BACK,32,0);
		LCD_ShowString(40,110,"Awaiting processing",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
		keys = 0xFF;	
	}

	if(pModeMenu->keyVal != 0xff)
	{
		keys = pModeMenu->keyVal;
		pModeMenu->keyVal = 0xFF;	//

		state = pModeMenu->state;
		if(state == KEY_CLICK)
		{
			switch(keys)
			{
				case KEY_RETURN_DAIL:	//
					pModeMenu = &settingModeMenu[STG_MENU_MAIN_SETTING];;
					pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
				break;
			}
		}
	}	
}
////////////////////////////








////拨打电话UI任务函数

static void gnlMenu_DialNumberCBS(void)
{
    unsigned char keys,state;
	static unsigned char dialNumb[20];
    static unsigned char CursorPos = 0;
	static unsigned short bLinkTimer = 0;
	static unsigned char bLinkFlag = 0;	
	unsigned char pdVal = 0;
	unsigned char keyPressFlag = 0;
	static unsigned char dialSta = 0;// = 0 表示输入电话号码  = 1 表示正在拨号  = 2 表示正在通话
	if(pModeMenu->refreshScreenCmd == SCREEN_CMD_RESET)
	{
		pModeMenu->refreshScreenCmd = SCREEN_CMD_NULL;
		hal_Tftlcd_Clear();
		LCD_ShowString(30,30,"Enter DialNumber",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
		CursorPos = 0;
		bLinkTimer = 0;
		bLinkFlag = 0;
		dialSta = 0;
		memset(&dialNumb[0],' ',16);
	}
	
	if(pModeMenu->keyVal != 0xff)
	{
		keys = pModeMenu->keyVal;
		pModeMenu->keyVal = 0xFF;	//

		state = pModeMenu->state;
		if(state == KEY_CLICK)
		{
			switch(keys)
			{
				case KEY_RETURN_DAIL:	//
					pModeMenu = pModeMenu->pParent;
					pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
					mt_4g_DialHandup();
				break;
				case KEY0:
					pdVal = '0';
					keyPressFlag = 1;
				break;
				case KEY1:
					pdVal = '1';
					keyPressFlag = 1;
				break;	
				case KEY2_UP:
					pdVal = '2';
					keyPressFlag = 1;
				break;
				case KEY3:
					pdVal = '3';
					keyPressFlag = 1;
				break;		
				case KEY4_LEFT:
					pdVal = '4';
					keyPressFlag = 1;
				break;
				case KEY5:
					pdVal = '5';
					keyPressFlag = 1;
				break;				
				case KEY6_RIGHT:
					pdVal = '6';
					keyPressFlag = 1;
				break;
				case KEY7:
					pdVal = '7';
					keyPressFlag = 1;
				break;	
				case KEY8_DOWN:
					pdVal = '8';
					keyPressFlag = 1;
				break;
				case KEY9:
					pdVal = '9';
					keyPressFlag = 1;
				break;	
				case KEY_MENU:
				{/////
					if(CursorPos > 2)
					{
						dialNumb[CursorPos]  = 0xff;
						LCD_ShowString(20+16*CursorPos,100," ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
						switch(mt_4G_AtaOper(dialNumb))
						//if(mt_4G_AtaOper(dialNumb) == DIALSTA_RING)
						{
							case DIALSTA_RING:
							{
								LCD_ShowString(20,150,"   Dialing ...   ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);		
								dialSta = 1;								
							}
							break;
							case DIALSTA_NNERROR:
							{
								LCD_ShowString(20,150,"   busy   ...   ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);		
								dialSta = 0;		
							}
							break;
							case DIALSTA_NOCARD:
							{
								LCD_ShowString(20,160,"  No Card    ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);		
							}							
						  break;
						}
					}		
				}	
				break;
				case KEY_SOS_DEL:
				{
					if(CursorPos>0)
					{
						if(CursorPos<16)
						{
							dialNumb[CursorPos] = ' ';
						}else
						{
							dialNumb[3] = ' ';
						}
						LCD_ShowString(20+16*CursorPos,100," ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0); 
						CursorPos--;
					}
				}
				break;
			}
		}
	}
	if(dialSta == 0)
	{
			if(CursorPos < 16)
			{
				if(keyPressFlag) 
				{
					keyPressFlag = 0;
					
					dialNumb[CursorPos] =  pdVal;
					if(CursorPos==15)
					{
						LCD_ShowString(20+16*CursorPos,100,&pdVal,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0); 
						CursorPos = 16;
					}
					else
					{
						LCD_ShowString(20+16*CursorPos,100,&pdVal,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0); 
						CursorPos++;
					} 
					bLinkTimer = 0; 
				}
			}
			else
			{
				CursorPos = 16; 
			}
			
			bLinkTimer++;
			if(bLinkTimer > 30)
			{
				bLinkTimer = 0;
				bLinkFlag = !bLinkFlag;	
			}
			if(bLinkFlag)
			{
				LCD_ShowString(20+16*CursorPos,100," ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);	
			}
			else
			{
				LCD_ShowString(20+16*CursorPos,100,"_",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
			}	
	}
	else
	{
		switch(mt_4G_GetDialSta())
		{
			case 	DIALSTA_RING:
				LCD_ShowString(20,150,"   Ringing...   ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);	
			break;
			case DIALSTA_CALLING:
				LCD_ShowString(20,150," On the phone   ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
			break;
			case DIALSTA_OVER:
			{
				LCD_ShowString(20,150,"   Hang up     ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
				pModeMenu = pModeMenu->pParent;
				pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
			  if(dialSta == 1)
				{///挂电话
					mt_4g_DialHandup();
				}
			}
			break;
		}
	}
}

////进入菜单密码保护UI任务函数
static void gnlMenu_EnterPinCBS(void) 
{
    unsigned char keys,state;  //按键值和按键状态
	static unsigned char passWord[6];  //保存输入系统密码
	static unsigned char CursorPos = 0; //输入密码的个数
	static unsigned short bLinkTimer = 0;  //光标闪烁的间隔时间
	static unsigned char bLinkFlag = 0;	  //光标的标志位
	static unsigned char KeyPutInComplete = 0;  ///密码输入的状态  =1表示密码输入完成
	static unsigned short Timer = 0;  ///延时
	unsigned char pdVal = 0;    //按键值
	unsigned char keyPressFlag = 0;//输入的按键值是否有效   =1表示有效
	if(pModeMenu->refreshScreenCmd == SCREEN_CMD_RESET)
	{/////输入密码界面初始化
		pModeMenu->refreshScreenCmd = SCREEN_CMD_NULL;
		
		hal_Tftlcd_Clear(); ///清除屏幕
		LCD_ShowString(90,20,"Enter Pin",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
		CursorPos = 0;
	    bLinkTimer = 0;
		bLinkFlag = 0;
		KeyPutInComplete = 0;
		Timer = 0;
		passWord[0] = ' ';
		passWord[1] = ' ';
		passWord[2] = ' '; 
		passWord[3] = ' ';
		passWord[4] = ' '; 
		passWord[5] = ' ';		
	}
	

	if(!KeyPutInComplete)
	{////如果按键输入没有完成   执行光标闪烁
		bLinkTimer++;
		if(bLinkTimer > 30)
		{///间隔300毫秒 更改光标的图标  周期600毫秒
			bLinkTimer = 0;
			bLinkFlag = !bLinkFlag;	
		}		
		if(bLinkFlag)
		{////
			LCD_ShowString(75+48*CursorPos,100," ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);	
		}
		else
		{
			LCD_ShowString(75+48*CursorPos,100,"_",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
		}	
	}

	if((pModeMenu->keyVal != 0xff) && (!KeyPutInComplete))
	{
		keys = pModeMenu->keyVal;
		pModeMenu->keyVal = 0xFF;	//

		state = pModeMenu->state;
		if(state == KEY_CLICK)
		{
			switch(keys)
			{
				case KEY_RETURN_DAIL:	// 
					pModeMenu = pModeMenu->pParent;
					pModeMenu->refreshScreenCmd = SCREEN_CMD_RESET;
				break;
				case KEY0:
						pdVal = '0';
						keyPressFlag = 1;
				break;
				case KEY1:
						pdVal = '1';
						keyPressFlag = 1;
				break;	
				case KEY2_UP:
						pdVal = '2';
						keyPressFlag = 1;
				break;
				case KEY3:
						pdVal = '3';
						keyPressFlag = 1;
				break;		
				case KEY4_LEFT:
						pdVal = '4';
						keyPressFlag = 1;
				break;
				case KEY5:
						pdVal = '5';
						keyPressFlag = 1;
				break;				
				case KEY6_RIGHT:
						pdVal = '6';
						keyPressFlag = 1;
				break;
				case KEY7:
						pdVal = '7';
						keyPressFlag = 1;
				break;	
				case KEY8_DOWN:
						pdVal = '8';
						keyPressFlag = 1;
				break;
				case KEY9:
						pdVal = '9';
						keyPressFlag = 1;
				break;	
				case KEY_MENU:
				{///确定按键
					if(passWord[0]==MT_GET_PARA_SYS_ADMINPASSWORD(0) 
						&& passWord[1]==MT_GET_PARA_SYS_ADMINPASSWORD(1)
						&& passWord[2]==MT_GET_PARA_SYS_ADMINPASSWORD(2)
						&& passWord[3]==MT_GET_PARA_SYS_ADMINPASSWORD(3))
					{///输入的密码正确
						//LCD_ShowString(20,100,"Wai add Menu",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
						pModeMenu = &settingModeMenu[STG_MENU_MAIN_SETTING];
						pModeMenu->refreshScreenCmd = SCREEN_CMD_RESET;
					}
					else
					{//输入密码错误
						KeyPutInComplete = 1;
						Timer = 0;
						LCD_ShowString(20,100,"Password mistake",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
					}
				}	
				break;
				case KEY_SOS_DEL:
				{///删除按键
					if(CursorPos>0)
					{
						LCD_ShowString(75+48*CursorPos,100," ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0); 
						CursorPos--;
					}
				}
				break;
			}
		}
	}
	if(CursorPos < 4)
	{////按键显示部分代码
		if(keyPressFlag) 
		{
			keyPressFlag = 0;	
			passWord[CursorPos] =  pdVal-'0';
			if(CursorPos==3)
			{
				LCD_ShowString(75+48*CursorPos,100,"*",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0); 
				CursorPos = 4;
			}
			else
			{
				LCD_ShowString(75+48*CursorPos,100,"*",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0); 
				CursorPos++;
			} 
			bLinkTimer = 0; 
		}
	}
	else
	{
		CursorPos = 4; 
	}
	
	if(KeyPutInComplete)
	{/// 密码错误 退出密码输入界面
		Timer++;
		if(Timer > 200)
		{
			KeyPutInComplete = 0;
			Timer = 0;
			pModeMenu = pModeMenu->pParent;			// 
			pModeMenu->refreshScreenCmd = SCREEN_CMD_RESET;
		}
	}
	
}
////系统撤防UI任务函数
static void systemAlarmKeyDisArmHandleCBS(void)
{
    unsigned char keys,state;
	static unsigned char passWord[6];  //保存键盘输入的密码
	static unsigned char CursorPos = 0;
	static unsigned short bLinkTimer = 0;///光标闪烁的时间
	static unsigned char bLinkFlag = 0;	 ///光标闪烁的标志位
	static unsigned char KeyPutInComplete = 0;  //输入密码操作是否完成的标志位  0 表示为输入完成 1输入完成了
	static unsigned short Timer = 0;  ///延时作用

	unsigned char pdVal = 0;
	unsigned char keyPressFlag = 0;
	
	if(pModeMenu->refreshScreenCmd == SCREEN_CMD_RESET)
	{
		pModeMenu->refreshScreenCmd = SCREEN_CMD_NULL;
		hal_Tftlcd_Clear();
	    LCD_ShowString(60,30,"System Disarm",HUE_LCD_FONT,HUE_LCD_BACK,32,0);
		CursorPos = 0;
	    bLinkTimer = 0;
		bLinkFlag = 0;
		Timer = 0;
	    KeyPutInComplete = 0;
		
		passWord[0] = ' ';
		passWord[1] = ' ';
		passWord[2] = ' '; 
		passWord[3] = ' ';
		passWord[4] = ' '; 
		passWord[5] = ' ';		
	}
	
	
	if(!KeyPutInComplete)
	{
		bLinkTimer++;
		if(bLinkTimer > 30)
		{
			bLinkTimer = 0;
			bLinkFlag = !bLinkFlag;	
			//0  
			
			if(bLinkFlag)
			{
				LCD_ShowString(75+48*CursorPos,100," ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);	
			}
			else
			{
				LCD_ShowString(75+48*CursorPos,100,"_",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
			}	
		}
			if(pModeMenu->keyVal != 0xff)
			{
				keys = pModeMenu->keyVal;
				pModeMenu->keyVal = 0xFF;	//

				state = pModeMenu->state;
				if(state == KEY_CLICK)
				{
					switch(keys)
					{
						case KEY_RETURN_DAIL:	// 
							pModeMenu = pModeMenu->pParent;
							pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
						break;
						case KEY0:
								pdVal = '0';
								keyPressFlag = 1;
						break;
						case KEY1:
								pdVal = '1';
								keyPressFlag = 1;
						break;	
						case KEY2_UP:
								pdVal = '2';
								keyPressFlag = 1;
						break;
						case KEY3:
								pdVal = '3';
								keyPressFlag = 1;
						break;		
						case KEY4_LEFT:
								pdVal = '4';
								keyPressFlag = 1;
						break;
						case KEY5:
								pdVal = '5';
								keyPressFlag = 1;
						break;				
						case KEY6_RIGHT:
								pdVal = '6';
								keyPressFlag = 1;
						break;
						case KEY7:
								pdVal = '7';
								keyPressFlag = 1;
						break;	
						case KEY8_DOWN:
								pdVal = '8';
								keyPressFlag = 1;
						break;
						case KEY9:
								pdVal = '9';
								keyPressFlag = 1;
						break;	
						case KEY_MENU:
						{
									if(passWord[0]==  MT_GET_PARA_SYS_ADMINPASSWORD(0) 
									&& passWord[1]== MT_GET_PARA_SYS_ADMINPASSWORD(1) 
									&& passWord[2]== MT_GET_PARA_SYS_ADMINPASSWORD(2) 
									&& passWord[3]== MT_GET_PARA_SYS_ADMINPASSWORD(3)) 
									{
										KeyPutInComplete = 1;
	                                    Timer = 0;
										
										SystemMode_Change(0XFF,SYSTEM_MODE_DISARM_HOST,SYSTEM_MODE_OPER_HOSTKEY);
										//LCD_ShowString(COOR_ICON_SYSTEMODE_X,COOR_ICON_SYSTEMODE_Y,"DISARM SUC",HUE_LCD_FONT,HUE_LCD_BACK,48,0); 
										//SystemMode_Change(0xff,SYSTEM_MODE_DISARM_HOST,SYSTEM_MODE_OPER_HOSTKEY);
									}
									else
									{
										KeyPutInComplete = 1;
										Timer = 0;
										LCD_ShowString(20,100,"Password mistake",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
										
									 //	pModeMenu = pModeMenu->pParent;	 
									 //	pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
									}
						}	
						break;
						case KEY_SOS_DEL:
						{
							if(CursorPos>0)
							{
								if(CursorPos<4)
								{
									passWord[CursorPos] = ' ';
								}else
								{
									passWord[3] = ' ';
								}
								LCD_ShowString(75+48*CursorPos,100," ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0); 
								CursorPos--;
							}
						}
						break;
					}
				}
			}
			if(CursorPos < 4)
			{
				if(keyPressFlag) 
				{
					keyPressFlag = 0;
					
					passWord[CursorPos] =  pdVal-'0';
					if(CursorPos==3)
					{
						LCD_ShowString(75+48*CursorPos,100,"*",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0); 
						CursorPos = 4;
					}
					else
					{
						LCD_ShowString(75+48*CursorPos,100,"*",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0); 
						CursorPos++;
					} 
					bLinkTimer = 0; 
				}
			}
			else
			{
				CursorPos = 4; 
			}	
	}
    
	
	if(KeyPutInComplete)
	{
		Timer++;
		if(Timer > 200)
		{///10ms *200  
			KeyPutInComplete = 0;
			Timer = 0;
			pModeMenu = pModeMenu->pParent;			// 
			pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
		}
	}
	pStuSystemMode->action();	// 
}

////固件升级任务函数 UI界面函数
static void ganMenu_FirmwareUpdate(void)
{
  unsigned char keys,state;
	static unsigned char stgMainMenuSelectedPos=0;
	static Stu_DTC tStuDtc;
	static unsigned char updatestate;
	static unsigned short percentage = 0;
	unsigned short percentageT = 0;
	unsigned char percent[7];
	static unsigned char fail = 0;
	static unsigned short timer=0;
	unsigned char updatstate;
	unsigned char comTyp;	
	if(pModeMenu->refreshScreenCmd == SCREEN_CMD_RESET)
	{	//
		pModeMenu->refreshScreenCmd = SCREEN_CMD_NULL;
		
		if(CheckPresenceofDtc(pModeMenu->reserved))
		{
			GetDtcStu(&tStuDtc,pModeMenu->reserved);
		}
		hal_Tftlcd_Clear();
		LCD_ShowString(35,20,pModeMenu->pModeType,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
		LCD_ShowString(30,70,"have New FireWare ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
		LCD_ShowString(40,110,"Are you update?",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
		LCD_ShowString(80,150,"Yes",HUE_LCD_FONT,HUE_FONT_BACK,FORTSIZE_24,0);
		LCD_ShowString(200,150,"No",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
		stgMainMenuSelectedPos = 1;
 		timer = 0;
 		fail = 0;
		updatestate = 0;
	}	

	if(pModeMenu->keyVal != 0xff)
	{
		keys = pModeMenu->keyVal;
		pModeMenu->keyVal = 0xFF;	//

		state = pModeMenu->state;
		if(!updatestate)
		{
			if(state == KEY_CLICK)
			{
				switch(keys)
				{
					case KEY_RETURN_DAIL:	//
						pModeMenu = pModeMenu->pParent;
						pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
					break;
					case KEY4_LEFT:
					case KEY6_RIGHT:
					{
								if(stgMainMenuSelectedPos == 1)		
								{
									stgMainMenuSelectedPos = 0;
									LCD_ShowString(80,150,"Yes",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
									LCD_ShowString(200,150,"No",HUE_LCD_FONT,HUE_FONT_BACK,FORTSIZE_24,0);
								}
								else if(stgMainMenuSelectedPos == 0)
								{
									stgMainMenuSelectedPos = 1;
									LCD_ShowString(80,150,"Yes",HUE_LCD_FONT,HUE_FONT_BACK,FORTSIZE_24,0);
									LCD_ShowString(200,150,"No",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
								}						
					}
					break;
					case KEY_MENU:
					{
							if(stgMainMenuSelectedPos)
							{
									tStuDtc.Mark = 0;
									SetDtcAbt(tStuDtc.ID-1,&tStuDtc);		//
									hal_Tftlcd_Clear();
									LCD_ShowString(60,160,"downloading......",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
								  LCD_ShowString(80,60,"000.0%",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_48,0);
								  mt_update_upingStart();  ///不升级固件
								  updatestate = 1;
								  percentage = 0;
								  hal_Wtn6_PlayVolue(WTN6_UPGRADE_DOWN_START); 
							}
							else
							{
									pModeMenu = pModeMenu->pParent;
									pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER; 
									mt_update_SetUpState(MT_UPDATE_STA_DOWN_N0);  ///不升级固件
							}
					}
					break;
				}
			}		
		}
	}
	
	if(updatestate == 1)
	{///固件正在升级
		updatstate = mt_update_GetUpState();
		if(updatstate == MT_UPDATE_STA_DOWN_FIRMWARE)
		{
				if(mt_wifi_get_wifi_Mqtt_state() == STA_MQTT_READY)
				{
					comTyp = 0;	
				}	
				else
				{
					if(mt_getgsmMqttState() == GSM_MQTT_READY)
					{
						comTyp = 1;	
					}
				}
				if(comTyp < 2)
				{
					percentageT = mt_update_pro(comTyp,&mt_protocol_GetFirmwarePack);
				}
				if(percentage != percentageT)
				{
					  percentage = percentageT;
						if(percentageT == 0xffff)
						{/////固件升级失败
								hal_Tftlcd_Clear();
								LCD_ShowString(60,160,"download failed",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
							  LCD_ShowString(40,200,"Wait for a timeout",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
								mt_update_SetUpState(MT_UPDATE_STA_DOWN_FIRMWARE);  ///升级失败
							  hal_Wtn6_PlayVolue(WTN6_UPGRADE_DOWN_FAIL); 
								timer = 0;
								fail = 1;
						}
						else
						{
							percentage = percentageT;
							percent[0] = percentage/1000 + '0';
							percent[1] = percentage%1000/100 + '0';
							percent[2] = percentage%100/10 + '0';
							percent[3] = '.';
							percent[4] = percentage%10 + '0';
							percent[5] = '%';
							percent[6] = 0;
							LCD_ShowString(80,60,percent,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_48,0);	
							if(percentage == 1000)	
							{///固件下载成功  系统重启，进入固件升级界面
									hal_Wtn6_PlayVolue(WTN6_UPGRADE_DOWN_SUC); 
							}						
						}
				 }		
		}
		else if(updatstate == MT_UPDATE_STA_DOWN_FAIL)
		{/////固件升级失败
				hal_Tftlcd_Clear();
				LCD_ShowString(60,160,"download failed",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
				mt_update_SetUpState(MT_UPDATE_STA_DOWN_FIRMWARE);  ///升级失败
		  	hal_Wtn6_PlayVolue(WTN6_UPGRADE_DOWN_FAIL); 
			
		}
		else if(updatstate == MT_UPDATE_STA_DOWN_OVER)
		{///// 
				hal_Tftlcd_Clear();
				percent[0] = '1';
				percent[1] = '0';
				percent[2] = '0';
				percent[3] = '.';
				percent[4] = '0';
				percent[5] = '%';
				percent[6] = 0;
				LCD_ShowString(80,60,percent,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_48,0);	
				LCD_ShowString(60,120,"download fnish",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
				LCD_ShowString(20,160,"Restart to complete the",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
				LCD_ShowString(20,190,"program upgrade",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);

				hal_Wtn6_PlayVolue(WTN6_UPGRADE_DOWN_SUC); 
				MT_SET_PARA_SYS_FIRMWARE(0,Firmware_Update.upVer.version[0]);
				MT_SET_PARA_SYS_FIRMWARE(1,Firmware_Update.upVer.version[1]);
				I2C_PageWrite(STU_SYSTEMPARA_OFFSET,(unsigned char*)(&mt_sEPR),sizeof(mt_sEPR));
				timer = 0;
				fail = 1;
		  //	NVIC_SystemReset();//// 增加系统重启
		}		
	}	
	if(fail)
	{
		timer++;
		if(timer > 350)		//1.5
		{
			timer = 0;
			fail = 0;
			pModeMenu = pModeMenu->pParent;
			pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;
		}
	}
}




static void menuInit(void)
{
	unsigned char i;
	
	loraDtector.state = LORAMODE_NORMAL;
	
	settingModeMenu[1].pLase = &settingModeMenu[STG_MENU_SUM-1];
	settingModeMenu[1].pNext = &settingModeMenu[2];
	settingModeMenu[1].pParent = &settingModeMenu[STG_MENU_MAIN_SETTING];

	for(i=2; i<STG_MENU_SUM-1; i++)
	{
		settingModeMenu[i].pLase = &settingModeMenu[i-1];
		settingModeMenu[i].pNext = &settingModeMenu[i+1];
		settingModeMenu[i].pParent = &settingModeMenu[STG_MENU_MAIN_SETTING];
	}
	settingModeMenu[STG_MENU_SUM-1].pLase = &settingModeMenu[i-1];
	settingModeMenu[STG_MENU_SUM-1].pNext = &settingModeMenu[1];
	settingModeMenu[STG_MENU_SUM-1].pParent = &settingModeMenu[STG_MENU_MAIN_SETTING];

	
	
//	stu_mode_menu DL_ZX_Review[STG_MENU_DL_ZX_SUM] = 
//{
//	{STG_MENU_DL_ZX_REVIEW_MAIN,STG_SUB_1_MENU_POS,"View",stgMenu_dl_ReviewMainCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0},	
//	{STG_MENU_DL_ZX_REVIEW,STG_SUB_2_MENU_POS,"View",stgMenu_dl_ReviewCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0},		 
//	{STG_MENU_DL_ZX_EDIT,STG_SUB_2_MENU_POS,"Edit",stgMenu_dl_EditCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0},
//	{STG_MENU_DL_ZX_DELETE,STG_SUB_2_MENU_POS,"Delete",stgMenu_dl_DeleteCBS,SCREEN_CMD_RESET,0,0xFF,0,0,0,0},
//};
	
	//2.ZONE list
	DL_ZX_Review[1].pLase = &DL_ZX_Review[STG_MENU_DL_ZX_SUM-1];
	DL_ZX_Review[1].pNext = &DL_ZX_Review[2];
	DL_ZX_Review[1].pParent = &settingModeMenu[STG_MENU_DTC_LIST];
	

	for(i=2;i<STG_MENU_DL_ZX_SUM-1;i++)
	{
		DL_ZX_Review[i].pLase = &DL_ZX_Review[i-1];
		DL_ZX_Review[i].pNext = &DL_ZX_Review[i+1];
		DL_ZX_Review[i].pParent = &settingModeMenu[STG_MENU_DTC_LIST];
	}
	DL_ZX_Review[STG_MENU_DL_ZX_SUM-1].pLase = &DL_ZX_Review[i-1];
	DL_ZX_Review[STG_MENU_DL_ZX_SUM-1].pNext = &DL_ZX_Review[1];
	DL_ZX_Review[STG_MENU_DL_ZX_SUM-1].pParent = &settingModeMenu[STG_MENU_DTC_LIST];	
	
	
	
	pModeMenu = &generalModeMenu[GNL_MENU_DESKTOP];
	
	
	//6.Phone Number
	//PN_Submenu[0].pLase = &PN_Submenu[STG_MENU_PN_SUM-1];
	PN_Submenu[0].pNext = &PN_Submenu[1];
	PN_Submenu[0].pParent = &settingModeMenu[STG_MENU_PHONE_NUMBER];
	 
	for(i=1; i<STG_MENU_PN_SUM-1; i++)
	{
		PN_Submenu[i].pLase = &PN_Submenu[i-1];
		PN_Submenu[i].pNext = &PN_Submenu[i+1];
		PN_Submenu[i].pParent = &settingModeMenu[STG_MENU_PHONE_NUMBER];
	}
	 
	PN_Submenu[STG_MENU_PN_SUM-1].pLase = &PN_Submenu[i-1];
	//PN_Submenu[STG_MENU_PN_SUM-1].pNext = &PN_Submenu[0];
	PN_Submenu[STG_MENU_PN_SUM-1].pParent = &settingModeMenu[STG_MENU_PHONE_NUMBER];

	systemAlarmMenu[SYSTEMALARM_MODE_ALARM].pChild = &systemAlarmMenu[SYSTEMALARM_MODE_DISARM];
	systemAlarmMenu[SYSTEMALARM_MODE_DISARM].pParent = &systemAlarmMenu[SYSTEMALARM_MODE_ALARM];

	//generalModeMenu[GNL_MENU_ENTER_DISARM].pParent = &generalModeMenu[GNL_MENU_DESKTOP];
}




void app_task_init(void)
{
	hal_KeyScanCBSRegister(KeyEventHandle); 
	mt_loraRxApplyNet_callback_Register(stgMenu_LoraDetectorApplyNetPro);
	mt_lora_loracomm_callback_Register(str_lora_loracommPro);
    QueueEmpty(LoraRcvMsg);
	
	
	//void App_task_Init(void)  初始化
	SetupMenuTimeOutCnt = 0;  //
	PutoutScreenTiemr = 0;    //
	ScreenState = 1;	
	ScreeControl(1);

	///函数:app_task_init  中添加
    pStuSystemMode = &stu_Sysmode[SYSTEM_MODE_ENARM_HOST];
	menuInit();// 函数中添加菜单初始化函数
	
	///void App_task_Init(void)
	QueueEmpty(DtcTriggerIDMsg);
    detector_off_lineInit();	
}


void app_task(void)
{
	  if((pModeMenu->menuPos!=DESKTOP_MENU_POS) &&(pModeMenu->menuPos!=STG_SUB_WIFI_MENU_POS)&&(pModeMenu->menuPos != STG_SUB_ALARMING_POS)&&(pModeMenu->menuPos != STG_SUB_FIREWARE_UP))
		{
			SetupMenuTimeOutCnt++;
			if(SetupMenuTimeOutCnt > SETUPMENU_TIMEOUT_PERIOD)
			{
				SetupMenuTimeOutCnt = 0;
//				if(pModeMenu == &systemAlarmMenu[SYSTEMALARM_MODE_DISARM])
//				{
//					pModeMenu = &systemAlarmMenu[SYSTEMALARM_MODE_ALARM];	//
//					pModeMenu->refreshScreenCmd = SCREEN_CMD_RECOVER;	//				
//				}
//				else
//				{
					pModeMenu = &generalModeMenu[GNL_MENU_DESKTOP];	//0000
					pModeMenu->refreshScreenCmd = SCREEN_CMD_RESET;	//
				//}
			}
		}
	  
	  if((pModeMenu->menuPos != STG_SUB_ALARMING_POS)&&(pModeMenu->menuPos!=STG_SUB_WIFI_MENU_POS)&&(pModeMenu->menuPos!=STG_SUB_FIREWARE_UP))
		{//3000
			PutoutScreenTiemr++;
			if(PutoutScreenTiemr > PUTOUT_SCREEN_PERIOD)
			{
				PutoutScreenTiemr = 0;
				ScreeControl(0);
			}
		}
        pModeMenu->action();		
}



static void gnlMenu_DesktopCBS(void)
{
	static unsigned char timer;
	EN_KEYNUM keys;
	KEY_VALUE_TYPEDEF state;
	unsigned char zoneNum;
	
	if((pModeMenu->refreshScreenCmd == SCREEN_CMD_RESET)||(pModeMenu->refreshScreenCmd == SCREEN_CMD_RECOVER))
	{
		pModeMenu->refreshScreenCmd = SCREEN_CMD_NULL;
		pModeMenu->keyVal = 0xFF;
		hal_Tftlcd_Clear();
		temHum_icon_Display(0);///温湿度显示函数
		PowerState_icon_Display();//外电  电池电量显示函数
		Gsm_icon_Display();   ///4G SIM状态   信号强度显示函数	
        showSystemTime();  //系统时间显示函数
		wifi_icon_Display();  
		server_icon_Display();
		LCD_ShowPicture32PixFont(COOR_ICON_WIFI_X,COOR_ICON_WIFI_Y,ICON_32X32_WIFI_S4,HUE_LCD_FONT,HUE_LCD_BACK,0);
		LCD_ShowPicture32PixFont(COOR_ICON_SERVER_X,COOR_ICON_SERVER_Y,ICON_32X32_SERVER,HUE_LCD_FONT,HUE_LCD_BACK,0);
		//LCD_ShowString(COOR_ICON_SYSTEMODE_X,COOR_ICON_SYSTEMODE_Y," DISARM ",HUE_LCD_FONT,HUE_LCD_BACK,48,0); 
		pStuSystemMode->refreshScreenCmd = SCREEN_CMD_RESET;
	}
	server_icon_Display();
	wifi_icon_Display();  ///WIFI 状态
	temHum_icon_Display(0);///温湿度显示函数
	PowerState_icon_Display();//外电  电池电量显示函数
	Gsm_icon_Display();   ///4G SIM状态   信号强度显示函数
	timer ++;
	if(timer == 100)
	{
		showSystemTime();  //系统时间显示函数
		timer = 0;
	}
	                             
	if(mt_update_GetUpState() == MT_UPDATE_STA_NEW_VER)
	{///有新的固件，进入有新的固件提示界面
		hal_Wtn6_PlayVolue(WTN6_UPGRADE_NEWFIREWARE);
		pModeMenu = &generalModeMenu[GNL_MENU_FIREWARE_UP];
		pModeMenu->pParent = &generalModeMenu[GNL_MENU_DESKTOP];
		pModeMenu->refreshScreenCmd = SCREEN_CMD_RESET;		
	}
	
	
	///static void gnlMenu_DesktopCBS(void)
	if(pModeMenu->keyVal != 0xff)
	{///
		keys =(EN_KEYNUM)pModeMenu->keyVal;
		state =(KEY_VALUE_TYPEDEF) pModeMenu->state;
		pModeMenu->keyVal = 0xFF;	//
		if(state == KEY_CLICK)
		{
			switch((unsigned char)keys)
			{
				case KEY_MENU:
					pModeMenu = &generalModeMenu[GNL_MENU_ENTER_PIN];///菜单密码保护界面
					pModeMenu->pParent = &generalModeMenu[GNL_MENU_DESKTOP];
					pModeMenu->refreshScreenCmd = SCREEN_CMD_RESET;		
				break;
				case KEY_DISARM:		
					pModeMenu = &generalModeMenu[GNL_MENU_ENTER_DISARM];
					pModeMenu->pParent = &generalModeMenu[GNL_MENU_DESKTOP];
					pModeMenu->refreshScreenCmd = SCREEN_CMD_RESET;	
					//LCD_ShowString(COOR_ICON_SYSTEMODE_X,COOR_ICON_SYSTEMODE_Y," DISARM ",HUE_LCD_FONT,HUE_LCD_BACK,48,0); 
				break;
				case KEY_HOMEARM:
					//LCD_ShowString(COOR_ICON_SYSTEMODE_X,COOR_ICON_SYSTEMODE_Y,"Home Arm",HUE_LCD_FONT,HUE_LCD_BACK,48,0); 
				   SystemMode_Change(0XFF,SYSTEM_MODE_HOMEARM_HOST,SYSTEM_MODE_OPER_HOSTKEY);
				break;
				case KEY_AWARARM:
					SystemMode_Change(0XFF,SYSTEM_MODE_ENARM_HOST,SYSTEM_MODE_OPER_HOSTKEY);
				    //LCD_ShowString(COOR_ICON_SYSTEMODE_X,COOR_ICON_SYSTEMODE_Y,"Away Arm",HUE_LCD_FONT,HUE_LCD_BACK,48,0); 
				break;
				case KEY_RETURN_DAIL:
					pModeMenu = &generalModeMenu[GNL_MENU_DAIL];
					pModeMenu->pParent = &generalModeMenu[GNL_MENU_DESKTOP];
					pModeMenu->refreshScreenCmd = SCREEN_CMD_RESET;
				break;
				
			}		
		}
		else if(state == KEY_LONG_PRESS)
		{
			if(keys == KEY_SOS_DEL)
			{
				zoneNum = 0xff;
				QueueDataIn(DtcTriggerIDMsg, &zoneNum,1); 
				SystemMode_Change(0xff,SYSTEM_MODE_ALARM,SYSTEM_MODE_OPER_HOSTKEY);
			}
		}	
	}
	pStuSystemMode->action();

}

static void server_icon_Display(void)
{
	static unsigned short BlinkIntervalTime = 0;
	
	if((mt_getgsmMqttState() == GSM_MQTT_READY)||(mt_wifi_get_wifi_Mqtt_state() == STA_MQTT_READY))
	{
		BlinkIntervalTime ++;
		if(BlinkIntervalTime == 200)
		{//
			BlinkIntervalTime = 0;
			LCD_ShowPicture32PixFont(210,0,ICON_32X32_SERVER,HUE_LCD_FONT,HUE_LCD_BACK,0);
		}		
	}
	else
	{
		BlinkIntervalTime ++;
		if(BlinkIntervalTime == 100)
			{///不显示服务器连接OK 图标
			LCD_ShowPicture32PixFont(210,0,ICON_32X32_CLEAR,HUE_LCD_BACK,HUE_LCD_BACK,0);	
		}
		else if(BlinkIntervalTime == 200)
		{//显示连接OK的图标
			BlinkIntervalTime = 0;
			LCD_ShowPicture32PixFont(210,0,ICON_32X32_SERVER,HUE_LCD_FONT,HUE_LCD_BACK,0);
		}		
	}
}


static void wifi_icon_Display(void)
{
  static unsigned char wifiSingal =0xff;  ///连接异常
	static unsigned char wifiSingalTim;
	/**************WIFI �ź�**********************/
	if(wifiSingal != mt_wifi_LinkState_rssi())
	{
		wifiSingal = mt_wifi_LinkState_rssi();
		wifiSingalTim = 0;
	}

	if(wifiSingal == 0xff)
	{////10ms
		wifiSingalTim ++;
		if(wifiSingalTim == 100)
			{//1秒的时候   不显示WIFI S4图标
			LCD_ShowPicture32PixFont(COOR_ICON_WIFI_X,COOR_ICON_WIFI_Y,ICON_32X32_WIFI_S4,HUE_LCD_BACK,HUE_LCD_BACK,0);
		}
		else if(wifiSingalTim > 200)
		{/// 2秒的时候   显示WIFI S4的图标
			wifiSingalTim = 0;
			LCD_ShowPicture32PixFont(COOR_ICON_WIFI_X,COOR_ICON_WIFI_Y,ICON_32X32_WIFI_S4,HUE_LCD_FONT,HUE_LCD_BACK,0);
		}
	}
	else
	{
		wifiSingalTim ++;
		if(wifiSingalTim == 100)
		{///显示WIFI的信号量
			LCD_ShowPicture32PixFont(COOR_ICON_WIFI_X,COOR_ICON_WIFI_Y,wifiSingal,HUE_LCD_FONT,HUE_LCD_BACK,0);
			wifiSingalTim = 0;
		}		
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
	if(!ScreenState)
	{
		ScreeControl(1);
	}
	else
	{
		if(pModeMenu)
		{
			pModeMenu->keyVal = keys;
			pModeMenu->state = sta;
//			if((pModeMenu->menuPos!=DESKTOP_MENU_POS)&&(pModeMenu->menuPos!=STG_SUB_WIFI_MENU_POS))
//			{
   			SetupMenuTimeOutCnt = 0;
//			}
			PutoutScreenTiemr = 0;
		}
	}
	if(sta==KEY_CLICK)
	{
		hal_Wtn6_PlayVolue(WTN6_VOLUE_DI);
	} 
}

/*
static void KeyEventHandle(EN_KEYNUM keys,KEY_VALUE_TYPEDEF sta)
{//00-15
		unsigned char keysta[3];
	  unsigned char test[50];

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
				LCD_ShowString(200,40,"KEY0",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
				test[0] = '1';  ///测试请修改成自己的电话号码
				test[1] = '1';
				test[2] = '1';
				test[3] = '1';
				test[4] = '1';
				test[5] = '1';
				test[6] = '1';
				test[7] = '1';
				test[8] = '1';
				test[9] = '1';
				test[10] = '1';
				test[11] = 0;
								
				test[20] = 'W';
				test[20+1] = 'U';
				test[20+2] = 'J';
				test[20+3] = 'I';
				test[20+4] = '-';
				test[20+5] = 'M';
				test[20+6] = 'C';
				test[20+7] = 'U';
				test[20+8] = 0;
				mt_gsmSendMessage(test);
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
				test[0] = 1;
				test[1] = 0;
				test[2] = 0;
				test[3] = 8;
				test[4] = 6;
				test[5] = 0xff;
        mt_gsmDialHandle(DIALTYPE_CALL,test);	
				LCD_ShowString(200,40,"KEY6",HUE_LCD_FONT,HUE_LCD_BACK,24,0);		
			}
			break;
			case KEY7:
			{
				LCD_ShowString(200,40,"KEY7",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
				mt_4g_DialHandup();
			}
			break;									
			case KEY8_DOWN:
			{
				LCD_ShowString(200,40,"KEY8",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
				mt_wifi_changState(ESP12_STA_GET_PASSWORD);
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
}*/
/////////////////

static void LoraRcvGedDatHandle(unsigned char *pBuff)
{
	unsigned char temp;
	temp = '#';
	QueueDataIn(LoraRcvMsg, &temp, 1);
	QueueDataIn(LoraRcvMsg, &pBuff[0], 2);   //#01#2456
}
//
static unsigned char str_lora_loracommPro(en_lora_eventTypedef event,str_cmdApplyNet pData)
{   
	Stu_DTC stuTempDevice;  //用于探测器参数时 初始化探测器信息
	unsigned char events;
	unsigned char node;
	unsigned char RecDatbuf[2];

	stuTempDevice.node[0] = pData.node[0];
	stuTempDevice.node[1] = pData.node[1];
	events = (unsigned char)pData.cmd;
	
	node = DtcMatching(&stuTempDevice.node[0]);
	
	if(node == 0xff)
	{//
		return 0;
	}
    else
	{
		RecDatbuf[0] = node-1; //探测器防区hao  0 
		RecDatbuf[1] = events; //探测器操作指令
		LoraRcvGedDatHandle(RecDatbuf);	
	}
	return 1;
}


//typedef enum
//{
//	LORAMODE_APPLEY_NET,     //申请入网
//	LORAMODE_APPLEY_NET_OK,  //配网完成OK 
//	LORAMODE_NORMAL,

//}en_loraWorkMode;

//typedef struct SYSTEM_LEARN
//{
//	en_loraWorkMode state;  ///学习状态
//	unsigned char detectorType;//探测器类型
//  unsigned char LearnSta;  //学习的状态
//}stu_loraDtector;

//stu_loraDtector loraDtector;



/////Lora探测器申请入网处理函数
static str_LoraAppNetState stgMenu_LoraDetectorApplyNetPro(en_lora_eventTypedef event,str_cmdApplyNet pData)
{
	str_ParaDetectorApplyNetState para_applyNetSta;
	str_LoraAppNetState loraApplyNetSta;
	Stu_DTC stuTempDevice; 		//用于探测器参数时 初始化探测器信息
	
	loraApplyNetSta.state = LORADET_LEARN_FAIL;

	if(event == LORA_COM_APPLY_NET)////接收到了探测器申请入网的申请
	{
		if(loraDtector.state == LORAMODE_APPLEY_NET)
		{///报警主机的状态是允许探测器申请入网
			loraDtector.state = LORAMODE_APPLEY_NET_OK;	
			
			
			loraDtector.detectorType = pData.detectorType;
			stuTempDevice.DTCType = (DTC_TYPE_TYPEDEF) pData.detectorType;
			stuTempDevice.ZoneType = ZONE_TYP_1ST;  ///探测器类型  第一防线
			stuTempDevice.Code[0] = pData.addr[0];
			stuTempDevice.Code[1] = pData.addr[1];
			stuTempDevice.Code[2] = pData.addr[2];
			stuTempDevice.Code[3] = pData.addr[3];
			stuTempDevice.Code[4] = pData.addr[4];
			stuTempDevice.Code[5] = pData.addr[5];
			stuTempDevice.Code[6] = pData.addr[6];
			stuTempDevice.Code[7] = pData.addr[7];
			stuTempDevice.Code[8] = pData.addr[8];
			stuTempDevice.Code[9] = pData.addr[9];
			stuTempDevice.Code[10] = pData.addr[10];
			stuTempDevice.Code[11] = pData.addr[11];	
			
			stuTempDevice.node[0] = pData.node[0];
			stuTempDevice.node[1] = pData.node[1];		
			stuTempDevice.sleepTimes = 0;
			
		 	para_applyNetSta = AddDtc(&stuTempDevice);
			
			loraDtector.LearnSta =	para_applyNetSta.state; 
			if(loraDtector.LearnSta == DET_UNLEARN)
			{
				loraApplyNetSta.state = LORADET_LEARN_OK;
				loraApplyNetSta.code = para_applyNetSta.code;
			}
			else if(loraDtector.LearnSta == DET_HAVED_LEARN)
			{
				loraApplyNetSta.state = LORADET_LEARN_FAIL;
				loraApplyNetSta.code = para_applyNetSta.code;
			}
		}
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

static void Gsm_icon_Display(void)
{
    static unsigned char gsmSingal=0xff;
	static unsigned char gsmSingalTim = 0;
	/**************WIFI �ź�**********************/
	if(gsmSingal != mt_4G_LinkState_rssi())
	{
		gsmSingal = mt_4G_LinkState_rssi();
		gsmSingalTim = 0;
	}
///gsmSIMStatus
	if(gsmSingal == 0xff)
	{///没有SIM卡
		LCD_ShowPicture32PixFont(282,0,ICON_32X32_GSM_NOCARD,HUE_LCD_FONT,HUE_LCD_BACK,0);
	}
	else
		{///有SIM卡 需要显示信号的强弱
		gsmSingalTim ++;
		if(gsmSingalTim == 100)  //1秒
		{
			LCD_ShowPicture32PixFont(282,0,(gsmSingal+ICON_32X32_GSM_S1),HUE_LCD_FONT,HUE_LCD_BACK,0);
			gsmSingalTim = 0;
		}		
	}
	/************************************/
}


static void showSystemTime(void)
{
//2021-04-09 18:10 Sun
	unsigned char displaytimebuf[16];
	memset(displaytimebuf,0,16);
	
	displaytimebuf[0]=0x32;
	displaytimebuf[1]=0x30;	
	displaytimebuf[2]=((stuSystemtime.year%1000%100)/10)+'0';
	displaytimebuf[3]=(stuSystemtime.year%1000%100%10)+'0';
	displaytimebuf[4]='-';	
	displaytimebuf[5]=(stuSystemtime.mon/10)+'0';
	displaytimebuf[6]=(stuSystemtime.mon%10)+'0';
	displaytimebuf[7]='-';	
	displaytimebuf[8]=(stuSystemtime.day/10)+'0';
	displaytimebuf[9]=(stuSystemtime.day%10)+'0';	
	displaytimebuf[10]=0;
	LCD_ShowString(20,200,displaytimebuf,HUE_LCD_FONT,HUE_LCD_BACK,24,0);
	displaytimebuf[0]=(stuSystemtime.hour/10)+'0';
	displaytimebuf[1]=(stuSystemtime.hour%10)+'0';
	displaytimebuf[2]=':';	
	displaytimebuf[3]=(stuSystemtime.min/10)+'0';
	displaytimebuf[4]=(stuSystemtime.min%10)+'0';		
	displaytimebuf[5]=0;	
	LCD_ShowString(160,200,displaytimebuf,HUE_LCD_FONT,HUE_LCD_BACK,24,0);	
	switch(stuSystemtime.week)
	{
		case 1:	//	        LCD_ShowString(260,200,"Mon",HUE_LCD_FONT,HUE_LCD_BACK,24,0);	
		break;
		case 2:	//
		  LCD_ShowString(260,200,"Tue",HUE_LCD_FONT,HUE_LCD_BACK,24,0);		
		break;
		case 3:	//
		  LCD_ShowString(260,200,"Wed",HUE_LCD_FONT,HUE_LCD_BACK,24,0);					
		break;
		
		case 4:	// 
		  LCD_ShowString(260,200,"Thu",HUE_LCD_FONT,HUE_LCD_BACK,24,0);			
		break;
		case 5:	//����5
			LCD_ShowString(260,200,"Fir",HUE_LCD_FONT,HUE_LCD_BACK,24,0);	
		break;
		case 6:	//����6
			LCD_ShowString(260,200,"Sat",HUE_LCD_FONT,HUE_LCD_BACK,24,0);	
		break;
		case 0:	//����7
			LCD_ShowString(260,200,"Sun",HUE_LCD_FONT,HUE_LCD_BACK,24,0);	
		break;
	}	
}
////

//cmd =1 打开背光     =0  关闭背光    
static void ScreeControl(unsigned char cmd)
{
	if(cmd)
	{//cmd =1 打开背光 
		if(!ScreenState)
		{///lcd 的状态是熄屏
			ScreenState = 1;   //修改状态  LCD屏幕是亮
		  hal_Oled_Display_on();//打开背光 
			PutoutScreenTiemr = 0; //
		}
	}
	else
	{
		if(ScreenState)
		{
			ScreenState = 0;
			hal_Oled_Display_off();
			PutoutScreenTiemr = 0;
		}
	}
}



static void S_DisArmModeProc(void) 
{
	unsigned char dat,zoneNum,zonefuc;
	
	static unsigned short InfoDisplayTim;
	Stu_DTC tStuDtc;
	
	if(pStuSystemMode->refreshScreenCmd == SCREEN_CMD_RESET)
	{
		pStuSystemMode->refreshScreenCmd = SCREEN_CMD_NULL;
		pStuSystemMode->keyVal = 0xFF;
		LCD_ShowString(70,100," DISARM ",HUE_LCD_FONT,HUE_LCD_BACK,48,0); 
		InfoDisplayTim = 1; 
	}		
	if(QueueDataLen(LoraRcvMsg))
	{	
		QueueDataOut(LoraRcvMsg,&dat);
		if(dat == '#')
		{
			if(QueueDataLen(LoraRcvMsg) >= 2)	
	                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         		{
				QueueDataOut(LoraRcvMsg,&zoneNum);
				QueueDataOut(LoraRcvMsg,&zonefuc);
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																		
				GetDtcStu(&tStuDtc,zoneNum);	
				if(tStuDtc.DTCType == DTC_REMOTE)
				{///探测器的类型  如果是遥控器
						if(zonefuc == LORA_COM_DISARM)
						{
								SystemMode_Change(tStuDtc.ID,SYSTEM_MODE_DISARM_HOST,SYSTEM_MODE_OPER_REMOTE);
						}
						else if(zonefuc == LORA_COM_AWAYARM)
						{
								SystemMode_Change(tStuDtc.ID,SYSTEM_MODE_ENARM_HOST,SYSTEM_MODE_OPER_REMOTE);
						}
						else if(zonefuc == LORA_COM_HOMEARM)
						{
								SystemMode_Change(tStuDtc.ID,SYSTEM_MODE_HOMEARM_HOST,SYSTEM_MODE_OPER_REMOTE);
						}	
						else if(zonefuc == LORA_COM_ALARM)
						{
 								SystemMode_Change(tStuDtc.ID,SYSTEM_MODE_ALARM,SYSTEM_MODE_OPER_REMOTE);	//紧急报警
 								zoneNum ++;
 								QueueDataIn(DtcTriggerIDMsg, &zoneNum,1);			
						}
                        else if(zonefuc == LORA_COM_BAT_LOW)
						{
							LCD_ShowString(60,160,tStuDtc.Name,HUE_LCD_FONT,HUE_LCD_BACK,24,0);
							LCD_ShowString(170,160,"Volt Low  ",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
							InfoDisplayTim = 1;
							mt_proto_PutInEvent(ENDPOINT_UP_TYPE_DETECTOR_BATLOW,zoneNum);

						}							
				}	
				else if(tStuDtc.DTCType == DTC_DOOR)   
				{///探测器属性是门磁
					if(zonefuc == LORA_COM_ALARM)
					{///门磁探测器开门
						if(tStuDtc.ZoneType==ZONE_TYP_24HOURS)
						{
 							SystemMode_Change(tStuDtc.ID,SYSTEM_MODE_ALARM,SYSTEM_MODE_OPER_REMOTE);	//紧急报警
 							zoneNum ++;
 							QueueDataIn(DtcTriggerIDMsg, &zoneNum, 1);		
						}
                        else 
						{
							LCD_ShowString(60,160,tStuDtc.Name,HUE_LCD_FONT,HUE_LCD_BACK,24,0);
							LCD_ShowString(170,160,"Door Open ",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
							InfoDisplayTim = 1;
							mt_proto_PutInEvent(ENDPOINT_UP_TYPE_DOOR_OPEN,zoneNum);

						}						
					}
					else
					{
                        if(zonefuc == LORA_COM_CLOSE)
						{///门磁探测器 关门动作
							LCD_ShowString(60,160,tStuDtc.Name,HUE_LCD_FONT,HUE_LCD_BACK,24,0);
							LCD_ShowString(170,160,"Door Close",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
							InfoDisplayTim = 1;
							mt_proto_PutInEvent(ENDPOINT_UP_TYPE_DOOR_CLOSE,zoneNum);
						}
						else if(zonefuc == LORA_COM_BAT_LOW)
						{
							LCD_ShowString(60,160,tStuDtc.Name,HUE_LCD_FONT,HUE_LCD_BACK,24,0);
							LCD_ShowString(170,160,"Volt Low  ",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
							InfoDisplayTim = 1;
							mt_proto_PutInEvent(ENDPOINT_UP_TYPE_DETECTOR_BATLOW,zoneNum);
						}
					}
				}
			}
		}	
	}
    if(InfoDisplayTim)
	{
		InfoDisplayTim++;
		if(InfoDisplayTim > 200)
		{
			InfoDisplayTim = 0;
			LCD_ShowString(0,160,"                         ",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
		}
	}
	
}

static void S_HomeArmModeProc(void) 
{
	unsigned char dat,zoneNum,zonefuc;
	static unsigned short InfoDisplayTim;
	Stu_DTC tStuDtc;	

	if(pStuSystemMode->refreshScreenCmd == SCREEN_CMD_RESET)
	{
		pStuSystemMode->refreshScreenCmd = SCREEN_CMD_NULL;
		pStuSystemMode->keyVal = 0xFF;
		LCD_ShowString(70,100,"HOME ARM",HUE_LCD_FONT,HUE_LCD_BACK,48,0);
		InfoDisplayTim = 1; 		
	}
	
	if(QueueDataLen(LoraRcvMsg))
	{	
		QueueDataOut(LoraRcvMsg,&dat);
		if(dat == '#')
		{
			if(QueueDataLen(LoraRcvMsg) >= 2)	
			{
				QueueDataOut(LoraRcvMsg,&zoneNum);
				QueueDataOut(LoraRcvMsg,&zonefuc);
				GetDtcStu(&tStuDtc,zoneNum);	
				if(tStuDtc.DTCType == DTC_REMOTE)
				{
						if(zonefuc == LORA_COM_DISARM)
						{
								SystemMode_Change(tStuDtc.ID,SYSTEM_MODE_DISARM_HOST,SYSTEM_MODE_OPER_REMOTE);
						}
						else if(zonefuc == LORA_COM_AWAYARM)
						{
								SystemMode_Change(tStuDtc.ID,SYSTEM_MODE_ENARM_HOST,SYSTEM_MODE_OPER_REMOTE);
						}
						else if(zonefuc == LORA_COM_HOMEARM)
						{
								SystemMode_Change(tStuDtc.ID,SYSTEM_MODE_HOMEARM_HOST,SYSTEM_MODE_OPER_REMOTE);
						}	
						else if(zonefuc == LORA_COM_ALARM)
						{
 								SystemMode_Change(tStuDtc.ID,SYSTEM_MODE_ALARM,SYSTEM_MODE_OPER_REMOTE);	//紧急报警
 								zoneNum ++;
 								QueueDataIn(DtcTriggerIDMsg, &zoneNum,1);			
						}	
						else if(zonefuc == LORA_COM_BAT_LOW)
						{
							LCD_ShowString(60,160,tStuDtc.Name,HUE_LCD_FONT,HUE_LCD_BACK,24,0);
							LCD_ShowString(170,160,"Volt Low  ",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
							InfoDisplayTim = 1;
							mt_proto_PutInEvent(ENDPOINT_UP_TYPE_DETECTOR_BATLOW,zoneNum);
						}							
				}	
				else if(tStuDtc.DTCType == DTC_DOOR) 
				{///探测器是门磁
					if(zonefuc == LORA_COM_ALARM)
					{///
						if(tStuDtc.ZoneType != ZONE_TYP_2ND)
						{
 							SystemMode_Change(tStuDtc.ID,SYSTEM_MODE_ALARM,SYSTEM_MODE_OPER_REMOTE);	//紧急报警
 							zoneNum ++;
 							QueueDataIn(DtcTriggerIDMsg, &zoneNum, 1);		
						}		
						else
						{
								LCD_ShowString(60,160,tStuDtc.Name,HUE_LCD_FONT,HUE_LCD_BACK,24,0);
								LCD_ShowString(170,160,"Door Open ",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
								InfoDisplayTim = 1;
							    mt_proto_PutInEvent(ENDPOINT_UP_TYPE_DOOR_OPEN,zoneNum);
						}							
					}
					else
					{
						if(zonefuc == LORA_COM_CLOSE)
						{
							LCD_ShowString(60,160,tStuDtc.Name,HUE_LCD_FONT,HUE_LCD_BACK,24,0);
							LCD_ShowString(170,160,"Door Close",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
							InfoDisplayTim = 1;
							mt_proto_PutInEvent(ENDPOINT_UP_TYPE_DOOR_CLOSE,zoneNum);

						}
						else if(zonefuc == LORA_COM_BAT_LOW)
						{
							LCD_ShowString(60,160,tStuDtc.Name,HUE_LCD_FONT,HUE_LCD_BACK,24,0);
							LCD_ShowString(170,160,"Volt Low  ",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
							InfoDisplayTim = 1;
							mt_proto_PutInEvent(ENDPOINT_UP_TYPE_DETECTOR_BATLOW,zoneNum);
						}
					}
				}
			}
		}	
	}
	if(InfoDisplayTim)
	{
		InfoDisplayTim++;
		if(InfoDisplayTim > 200)
		{
			InfoDisplayTim = 0;
			LCD_ShowString(0,160,"                         ",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
		}
	}
}






static void S_ENArmModeProc(void) 
{
	unsigned char dat;
	unsigned char zoneNum,zonefuc;
	static unsigned short InfoDisplayTim;
	Stu_DTC tStuDtc;
	if(pStuSystemMode->refreshScreenCmd == SCREEN_CMD_RESET)
	{
		pStuSystemMode->refreshScreenCmd = SCREEN_CMD_NULL;
		pStuSystemMode->keyVal = 0xFF;
		LCD_ShowString(70,100,"AWAY ARM",HUE_LCD_FONT,HUE_LCD_BACK,48,0);	
		InfoDisplayTim = 1;
	}
	
	if(QueueDataLen(LoraRcvMsg))
	{	
		QueueDataOut(LoraRcvMsg,&dat);
		if(dat == '#')
		{
			if(QueueDataLen(LoraRcvMsg) >= 2)	
			{
				QueueDataOut(LoraRcvMsg,&zoneNum);
				QueueDataOut(LoraRcvMsg,&zonefuc);
				GetDtcStu(&tStuDtc,zoneNum);	
				if(tStuDtc.DTCType == DTC_REMOTE)
				{
						if(zonefuc == LORA_COM_DISARM)
						{
								SystemMode_Change(tStuDtc.ID,SYSTEM_MODE_DISARM_HOST,SYSTEM_MODE_OPER_REMOTE);
						}
				        else if(zonefuc == LORA_COM_AWAYARM)
						{
								SystemMode_Change(tStuDtc.ID,SYSTEM_MODE_ENARM_HOST,SYSTEM_MODE_OPER_REMOTE);
						}
						else if(zonefuc == LORA_COM_HOMEARM)
						{
								SystemMode_Change(tStuDtc.ID,SYSTEM_MODE_HOMEARM_HOST,SYSTEM_MODE_OPER_REMOTE);
						}	
						else if(zonefuc == LORA_COM_ALARM)
						{
							 SystemMode_Change(tStuDtc.ID,SYSTEM_MODE_ALARM,SYSTEM_MODE_OPER_REMOTE);	//紧急报警
							 zoneNum ++;
							 QueueDataIn(DtcTriggerIDMsg, &zoneNum,1);			
						}	//
						else if(zonefuc == LORA_COM_BAT_LOW)
						{
							LCD_ShowString(60,160,tStuDtc.Name,HUE_LCD_FONT,HUE_LCD_BACK,24,0);
							LCD_ShowString(170,160,"Volt Low  ",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
							InfoDisplayTim = 1;
							mt_proto_PutInEvent(ENDPOINT_UP_TYPE_DETECTOR_BATLOW,zoneNum);
						}	
				}	
				else if(tStuDtc.DTCType == DTC_DOOR)    
				{
					if(zonefuc == LORA_COM_ALARM)
					{////门磁探测器开门
						//Name
						 SystemMode_Change(tStuDtc.ID,SYSTEM_MODE_ALARM,SYSTEM_MODE_OPER_REMOTE);	//紧急报警
						 zoneNum ++;
						 QueueDataIn(DtcTriggerIDMsg, &zoneNum, 1);						
					}
					else
					{
						if(zonefuc == LORA_COM_CLOSE)
						{
							LCD_ShowString(60,160,tStuDtc.Name,HUE_LCD_FONT,HUE_LCD_BACK,24,0);
							LCD_ShowString(170,160,"Door Close",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
							InfoDisplayTim = 1;
							mt_proto_PutInEvent(ENDPOINT_UP_TYPE_DOOR_CLOSE,zoneNum);

						}
						else if(zonefuc == LORA_COM_BAT_LOW)
						{
							LCD_ShowString(60,160,tStuDtc.Name,HUE_LCD_FONT,HUE_LCD_BACK,24,0);
							LCD_ShowString(170,160,"Volt Low  ",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
							InfoDisplayTim = 1;
							mt_proto_PutInEvent(ENDPOINT_UP_TYPE_DETECTOR_BATLOW,zoneNum);
						}	
					}
				}
			}
		}	
	}
	if(InfoDisplayTim)
	{
		InfoDisplayTim++;
		if(InfoDisplayTim > 200)
		{
			InfoDisplayTim = 0;
			LCD_ShowString(0,160,"                         ",HUE_LCD_FONT,HUE_LCD_BACK,24,0);
		}
	}
}



static void S_AlarmModeProc(void)
{
	unsigned char dat,zoneNum,zonefuc;
	Stu_DTC tStuDtc;
	if(QueueDataLen(LoraRcvMsg))
	{	
		QueueDataOut(LoraRcvMsg,&dat);
		if(dat == '#')
		{
			if(QueueDataLen(LoraRcvMsg) >= 2)	
			{
				QueueDataOut(LoraRcvMsg,&zoneNum);
				QueueDataOut(LoraRcvMsg,&zonefuc);
				GetDtcStu(&tStuDtc,zoneNum);	
				if(tStuDtc.DTCType == DTC_REMOTE)
				{///
						if(zonefuc == LORA_COM_DISARM)
						{///遥控器撤防
								SystemMode_Change(tStuDtc.ID,SYSTEM_MODE_DISARM_HOST,SYSTEM_MODE_OPER_REMOTE);
						}
						else if(zonefuc == LORA_COM_ALARM)
						{//紧急报警的按键
								 SystemMode_Change(tStuDtc.ID,SYSTEM_MODE_ALARM,SYSTEM_MODE_OPER_REMOTE);	//紧急报警
								 zoneNum ++;
								 QueueDataIn(DtcTriggerIDMsg, &zoneNum,1);			
						}	
				}	
				else
				{
					if(zonefuc == LORA_COM_ALARM)
					{///
						 if((tStuDtc.ZoneType==ZONE_TYP_24HOURS) || (tStuDtc.ZoneType != ZONE_TYP_2ND))
						 {
							 SystemMode_Change(tStuDtc.ID,SYSTEM_MODE_ALARM,SYSTEM_MODE_OPER_REMOTE);	//紧急报警
							 zoneNum ++;
							 QueueDataIn(DtcTriggerIDMsg, &zoneNum, 1);		
						 }							
					}
					
				}
			}
		}	
	}	
}



///zone     配件防区   遥控器1  遥控器2 遥控器3
///sysMode  系统模式
///operType 操作方式
void SystemMode_Change(unsigned char zone,SYSTEMMODE_TYPEDEF sysMode,EN_SYSTEMODE_OPTERTYPEDEF operType)
{

	///// 完善函数:void SystemMode_Change 
	if(sysMode == SYSTEM_MODE_ALARM)
	{
		if(pModeMenu != &systemAlarmMenu[SYSTEMALARM_MODE_ALARM])
		{///判断当前的界面是否是报警界面  如果不是
			pModeMenu = &systemAlarmMenu[SYSTEMALARM_MODE_ALARM];
			pModeMenu ->refreshScreenCmd = SCREEN_CMD_RESET;///
			pStuSystemMode = &stu_Sysmode[sysMode];
			ScreeControl(1);  ///如果系统报警了，让屏幕亮起来
		}
		if(zone == 0xff)
		{///主机键盘报警
			mt_proto_PutInEvent(ENDPOINT_UP_TYPE_HOST_ALARM_SOS,0xff);
		}
		else
		{
			mt_proto_PutInEvent(ENDPOINT_UP_TYPE_DETECTOR_ALARM,zone);
		}
		
		
	}
 	else if(sysMode < SYSTEM_MODE_SUM)
	{
		if(sysMode == SYSTEM_MODE_DISARM_HOST)
		{
			hal_Wtn6_PlayVolue(WTN6_DISARM);///
			mt_4g_systemDisarmOper();
			//if(pModeMenu == &generalModeMenu[GNL_MENU_ENTER_DISARM])
			if((pModeMenu == &systemAlarmMenu[SYSTEMALARM_MODE_ALARM]) || (pModeMenu == &systemAlarmMenu[SYSTEMALARM_MODE_DISARM]) || (pModeMenu == &generalModeMenu[GNL_MENU_ENTER_DISARM]))			
			{
				pModeMenu = &generalModeMenu[DESKTOP_MENU_POS];
				pModeMenu->refreshScreenCmd = SCREEN_CMD_RESET;
			}

			switch(operType)
			{
				case 	SYSTEM_MODE_OPER_HOSTKEY:///主机键盘
				mt_proto_PutInEvent(ENDPOINT_UP_ARM_TYPE_DISARM_BY_HOST,zone);		
				break;
				case 	SYSTEM_MODE_OPER_REMOTE:  ///遥控器操作
				  mt_proto_PutInEvent(ENDPOINT_UP_ARM_TYPE_DISARM_BY_REMOTE,zone);
				break;
				case 	SYSTEM_MODE_OPER_APP:    ///服务器  APP 远程控制
				  mt_proto_PutInEvent(ENDPOINT_UP_ARM_TYPE_DISARM_BY_APP,zone);
				break;
			}	
		}
		else if(sysMode == SYSTEM_MODE_HOMEARM_HOST)
		{
			hal_Wtn6_PlayVolue(WTN6_HOMEARM);
		    switch(operType)
		 	{
				case 	SYSTEM_MODE_OPER_HOSTKEY:///主机键盘
			  	mt_proto_PutInEvent(ENDPOINT_UP_ARM_TYPE_DISARM_BY_HOST,zone);		
				break;
				case 	SYSTEM_MODE_OPER_REMOTE:  ///遥控器操作
				  mt_proto_PutInEvent(ENDPOINT_UP_ARM_TYPE_DISARM_BY_REMOTE,zone);
				break;
				case 	SYSTEM_MODE_OPER_APP:    ///服务器  APP 远程控制
				  mt_proto_PutInEvent(ENDPOINT_UP_ARM_TYPE_DISARM_BY_APP,zone);
				break;
			}			
		}
		else if(sysMode == SYSTEM_MODE_ENARM_HOST)
		{
			hal_Wtn6_PlayVolue(WTN6_AWAYARM);
			switch(operType)
			{
				case 	SYSTEM_MODE_OPER_HOSTKEY:///主机键盘
					 mt_proto_PutInEvent(ENDPOINT_UP_ARM_TYPE_ARM_BY_HOST,0xff);
				break;
				case 	SYSTEM_MODE_OPER_REMOTE:  ///遥控器操作
					 mt_proto_PutInEvent(ENDPOINT_UP_ARM_TYPE_ARM_BY_REMOTE,zone);				
				break;
				case 	SYSTEM_MODE_OPER_APP:    ///服务器  APP 远程控制
					 mt_proto_PutInEvent(ENDPOINT_UP_ARM_TYPE_ARM_BY_APP,0xff);		
				break;
			}
			
			
		}

		ScreeControl(1);
		PutoutScreenTiemr = 0;
		
		if(pStuSystemMode != &stu_Sysmode[sysMode])
		{
			pStuSystemMode = &stu_Sysmode[sysMode];	
			pStuSystemMode->refreshScreenCmd = SCREEN_CMD_RESET;
		}		
	}
}





///////
static void systemAlarmHandleCBS(void)
{
	unsigned char keys,state,i;
	static unsigned short timer = 0;  ///更新报警信息
	static unsigned short timer2 = 0; ///ALARMING 闪烁
	static unsigned char displayAlarmFlag = 1;	////ALARMING 闪烁
	static unsigned char alarmNum,aralmFull,alarmInfo[3];
	//aralmFull 当前的报警信息是否等于或超过3
	//alarmNum  当前报警信息的个数
	unsigned char id;
	static unsigned char alarmVolueFlag = 0;	

	Stu_DTC tStuDtc;

	if(pModeMenu->refreshScreenCmd == SCREEN_CMD_RESET)
	{
		hal_Tftlcd_Clear();	
		pModeMenu->refreshScreenCmd = SCREEN_CMD_NULL; 
		pModeMenu->keyVal = 0xFF;
		LCD_ShowString(70,50,"ALARMING",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_48,0);
		alarmNum = 0;
		timer = 0;
		aralmFull = 0;
	    timer2 = 160;
		displayAlarmFlag = 0;
		alarmVolueFlag = 3;
		systemAlarmDialSmsHandle(ALARM_DAILSMS_STA_INIT);
	}
	else if(pModeMenu->refreshScreenCmd == SCREEN_CMD_RECOVER)
	{
		hal_Tftlcd_Clear();	
		pModeMenu->refreshScreenCmd = SCREEN_CMD_NULL;
		pModeMenu->keyVal = 0xFF;	       LCD_ShowString(70,50,"ALARMING",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_48,0);
		timer = 1;
		alarmVolueFlag = 3;
	}	
	
	systemAlarmDialSmsHandle(ALARM_DAILSMS_STA_ALARMING);
	
	if(pModeMenu->keyVal != 0xff)
	{
		keys = pModeMenu->keyVal;
		pModeMenu->keyVal = 0xFF;	//
		state = pModeMenu->state;
		if(state == KEY_CLICK)
		{
			switch(keys)
			{
				case KEY_DISARM:
					pModeMenu =  pModeMenu->pChild;
				    pModeMenu->refreshScreenCmd = SCREEN_CMD_RESET;
				break;
			}
		}
	}
	
	if(QueueDataLen(DtcTriggerIDMsg))
	{////           DtcTriggerIDMsg
		QueueDataOut(DtcTriggerIDMsg,&id);
		if(id > 0)
		{//1-20  =》0-19   
			timer = 1;   /// 0 1 2
			alarmInfo[alarmNum ++] = id;
			if(alarmNum == 3)
			{
				alarmNum = 0;
				aralmFull = 1;
			}
		}
	}
	
    if(timer)
	{
		timer = 0;
		if(aralmFull == 1)
		{
				for(i=0;i<3;i++)
				{
					  if(alarmInfo[i] == 0xff)
						{                                  
									LCD_ShowString(30,(120+30*i),"Host KeyBoard Sos  ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
						}
						else
						{
							if((alarmInfo[i]-1) < PARA_DTC_SUM)
							{
								GetDtcStu(&tStuDtc,(alarmInfo[i]-1)); 	
								switch(tStuDtc.DTCType)
								{
									case DTC_REMOTE:                 
										LCD_ShowString(30,(120+30*i),"Remote: sos        ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
									break;
									case DTC_DOOR:                   
										LCD_ShowString(30,(120+30*i),"Door:              ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
										LCD_ShowString(110,(120+30*i),tStuDtc.Name,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
									break;
								}	
							}								
						}
				}
		}
		else 
		{
			for(i=0;i<alarmNum;i++)
			{
					if(alarmInfo[i] == 0xff)
					{                                  
						LCD_ShowString(30,(120+30*i),"Host KeyBoard Sos  ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
					}
					else
					{////配件   遥控器   门磁探测器
						if((alarmInfo[i]-1) < PARA_DTC_SUM)
						{
							GetDtcStu(&tStuDtc,(alarmInfo[i]-1)); 	
							switch(tStuDtc.DTCType)
							{
								case DTC_REMOTE:                 
									LCD_ShowString(30,(120+30*i),"Remote: sos        ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
								break;
								case DTC_DOOR:                   
									LCD_ShowString(30,(120+30*i),"Door:              ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
									LCD_ShowString(110,(120+30*i),tStuDtc.Name,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
								break;
							}							
						}
					}
			}
		}
	}	
	
	timer2++;
	if(timer2 > 80)
	{///10*160    1.6
		timer2 = 0;
		
		displayAlarmFlag = !displayAlarmFlag;
		
		alarmVolueFlag ++;
		if(alarmVolueFlag == 4) 
		{
			alarmVolueFlag = 0;
			hal_Wtn6_PlayVolue(WTN6_VOLUE_110_18);
		}///800*4 =3.2
		
		
		if(displayAlarmFlag)
		{
			//hal_Wtn6_PlayVolue(WTN6_VOLUE_110_18);
			LCD_ShowString(70,50,"ALARMING",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_48,0);							 
		}
		else
		{
			LCD_ShowString(70,50,"         ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_48,0);	 
		}
	}
	pStuSystemMode->action();
}


static unsigned char SystemGetPhoneNo(unsigned char serial,unsigned char *phoneNo)
{
	unsigned char i;
	for(i=0; i<19; i++)
	{ 
		if(mt_sEPR.PhoneNumber[serial][i]<10)		//0-9
		{
			phoneNo[i] =  mt_sEPR.PhoneNumber[serial][i] + '0';	//
		}
		else
		{
			phoneNo[i] = '\0';
			return i;
		//	break;
		}		
	}		
	phoneNo[i] = '\0';//
	return 0;
}


//SMS
const unsigned char SMS_Text[SMS_TEXT_MAX][30]=
{
	"Disarm success\0",
	"Away arm success\0",
	"Home arm success\0",
	"Alarm panel status:Disarm\0",
	"Alarm panel status:Away arm\0",
	"Alarm panel status:Home arm\0",
	"Alarm panel status:Alarming\0",
	"WiFi network is off\0",
	"WiFi network reconnection\0"
};

/////
static void systemAlarmDialSmsHandle(un_AlarmDailSmsSta initflag)
{
//	static un_AlarmDailSmsSta sta;
	static srt_alarmDialSms alarmDailSms;
	
	
	static unsigned int timer = 0;
	unsigned char SentSmsDailBuffer[140];
	unsigned char i,idx;
	if(initflag == ALARM_DAILSMS_STA_INIT)
	{               // ALARM_DAILSMS_STA_INIT
		alarmDailSms.alarsmsSta = ALARMING_START;
		alarmDailSms.cycleTims = 0;
		alarmDailSms.serialNum = 0;
		timer = 0;
	}
	switch(alarmDailSms.alarsmsSta)
	{
		case ALARMING_IDLE:
		{
			alarmDailSms.alarsmsSta = ALARMING_IDLE;
			alarmDailSms.cycleTims = 0;
			alarmDailSms.serialNum = 0;
		}
		break;
		case ALARMING_START:
		{
			alarmDailSms.alarsmsSta = ALARMING_SMS;	
		}	
		break;	
		case ALARMING_SMS:
		{
			memset(SentSmsDailBuffer,0,140);
			if(SystemGetPhoneNo(alarmDailSms.serialNum,SentSmsDailBuffer) == 11)
			{
				idx = 20;
				for(i=0;i<140;i++)
				{
					if(SMS_Text[SMS_TEXT_ALARM_MODE][i] !=0x00)
					{
						SentSmsDailBuffer[idx ++] = SMS_Text[SMS_TEXT_ALARM_MODE][i];
					}
					else
						break;
				}
				SentSmsDailBuffer[idx ++] = '\0';
				mt_gsmSendMessage(SentSmsDailBuffer);					
			}
			alarmDailSms.serialNum ++;
			if(alarmDailSms.serialNum == 6)
			{
				alarmDailSms.serialNum = 0;
				alarmDailSms.alarsmsSta = ALARMING_DIAL;
			}
		}	
		break;
		case ALARMING_DIAL:
		{
				if(QueueDataLen(GSMTxMessageIdMsg) == 0)
				{///检测发送短信的队列是否为空		
					switch(mt_getSMSDialState())
					{
						case GSM_STATE_SMSDIAL_READY:
							memset(SentSmsDailBuffer,0,20);
							////0-5
							if(SystemGetPhoneNo(alarmDailSms.serialNum,SentSmsDailBuffer) >2)
							{////获取电话号码
								if(mt_4G_AlarmingAtaOper(SentSmsDailBuffer) == DIALSTA_RING)
								{////正在拨打电话中
									alarmDailSms.alarsmsSta = ALARMING_DIALING;
								}
							}
							else
							{
								alarmDailSms.alarsmsSta = ALARMING_DIAL_WAITNEXT;
								timer = 10;
							}
						break;	
					}	
				}					
		}	
		break;
	     case ALARMING_DIALING:
		{///正在拨打电话中
			if(mt_getSMSDialState() == GSM_STATE_DIAL_ALARM_END)
			{
				alarmDailSms.alarsmsSta = ALARMING_DIAL_WAITNEXT;
				timer = 150;
			}
			else if(mt_getSMSDialState() == GSM_STATE_SMSDIAL_READY)
			{
				alarmDailSms.cycleTims = 0;
				alarmDailSms.alarsmsSta = ALARMING_IDLE;
			}
		}
		break;						
		case ALARMING_DIAL_WAITNEXT:
		{
			if(timer)
				timer --;
			if(timer == 0)
			{
					alarmDailSms.serialNum ++;
					if(alarmDailSms.serialNum > 5)
					{
							alarmDailSms.serialNum = 0;
							alarmDailSms.cycleTims ++;
							if(alarmDailSms.cycleTims >= SYSTEM_ALARM_CYCLETIMS_MAX)
							{
									alarmDailSms.cycleTims = 0;
									alarmDailSms.alarsmsSta = ALARMING_IDLE;
									return;
							}
					}
					alarmDailSms.alarsmsSta = ALARMING_DIAL;			
			}
		}
		break; 
	}
}


/////函数调用 按照来技术
void detector_off_lineHandle(void)
{
	unsigned char i;
	for (i=0;i<PARA_DTC_SUM;i++)
	{
		if(MT_GET_PARA_DEVICE_MARK(i))
		{
			if(MT_GET_PARA_DEVICE_SLEEPTIME(i) < DEC_OFFLINE_TIME)
			{
				MT_SET_PARA_DEVICE_SLEEPTIME(i,(MT_GET_PARA_DEVICE_SLEEPTIME(i) + 1));
			}	
			else if(MT_GET_PARA_DEVICE_SLEEPTIME(i) == DEC_OFFLINE_TIME)
		    { 
				MT_SET_PARA_DEVICE_SLEEPTIME(i,(DEC_OFFLINE_TIME + 1));
				if(MT_GET_PARA_DEVICE_ZONETYPE(i) != DTC_REMOTE)
				{//// 增加离线推送代码 和离线提示功能
					mt_proto_PutInEvent(ENDPOINT_UP_TYPE_DETECTOR_OFFLINE,i);
				}
			}				
		}
	}
	hal_ResetTimer(T_DEC_OFFLINE,T_STA_START);		
}

 
///app.c 中新建
static void detector_off_lineInit(void)
{////1秒
	hal_CreatTimer(T_DEC_OFFLINE,detector_off_lineHandle,20000,T_STA_START);
}

//void App_task_Init(void) 中添加
 
   
 


