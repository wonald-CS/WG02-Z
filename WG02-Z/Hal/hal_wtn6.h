#ifndef _HAL_WTN6_H_
#define _HAL_WTN6_H_



#define WTN6_CLK_PORT  GPIOC          //语音芯片WTN6
#define WTN6_CLK_PIN   GPIO_Pin_1 

#define WTN6_DAT_PORT  GPIOC
#define WTN6_DAT_PIN   GPIO_Pin_2 

#define SC8002_SH_PORT  GPIOA         //运放芯片SC8002
#define SC8002_SH_PIN   GPIO_Pin_0 


#define WTN6_CLK_HIGH  GPIO_SetBits(WTN6_CLK_PORT,WTN6_CLK_PIN)
#define WTN6_CLK_LOW   GPIO_ResetBits(WTN6_CLK_PORT,WTN6_CLK_PIN)

#define WTN6_DAT_HIGH  GPIO_SetBits(WTN6_DAT_PORT,WTN6_DAT_PIN)
#define WTN6_DAT_LOW   GPIO_ResetBits(WTN6_DAT_PORT,WTN6_DAT_PIN)

#define SC8002_SH_HIGH  GPIO_SetBits(SC8002_SH_PORT,SC8002_SH_PIN)
#define SC8002_SH_LOW   GPIO_ResetBits(SC8002_SH_PORT,SC8002_SH_PIN)




enum
{
	WTN6_HOMEARM = 1,   ///在家布防
	WTN6_AWAYARM,       ///离家布防
	WTN6_DISARM,        ///撤防
	WTN6_STUDY_START,   ///开始成功，请出发探测器
	WTN6_STUDY_SUC,     ///配对成功 
	WTN6_HAVED_DETEC,   ///探测器已存在 
	WTN6_STUDY_FAIL,    ///配对失败
	WTN6_GET_WIFI_PASSWORD,    ///开始配网，请在APP上输入WIFI密码，点击链接按钮
	WTN6_GET_WIFI_OK,    ///配网成功
	WTN6_GET_WIFI_FAIL,  ///配网失败
	WTN6_WIFI_TIMEOUT,  ///配网超时	

	WTN6_AC_DOWN,    ///主机掉电
	WTN6_AC_RECOVER,  ///外电恢复
	WTN6_TO_FACTORY,  ///恢复出厂设置OK

	WTN6_UPGRADE_NEWFIREWARE,//new fireware
	WTN6_UPGRADE_DOWN_START,//
	WTN6_UPGRADE_DOWN_FAIL,
	WTN6_UPGRADE_DOWN_SUC,  ///升级成功，

	WTN6_VOLUE_SUC,         ///音效 成功音效
	WTN6_VOLUE_DI,        ///音效 DI DI
	WTN6_VOLUE_DINGDONG,    ///音效 叮咚
	WTN6_VOLUE_VOLT_LOW,    ///音效 电池低压	

	WTN6_VOLUE_110,         ///音效 110报警声音
	WTN6_VOLUE_110_12,         ///音效 110报警声音
	WTN6_VOLUE_110_15,         ///音效 110报警声音
	WTN6_VOLUE_110_18,         ///音效 110报警声音
};




void hal_Wtn6Init(void);
void hal_Wtn6_Play(unsigned char VolNum);

#endif
