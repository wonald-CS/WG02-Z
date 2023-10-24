#include "hal_key.h"
#include "stm32F10x.h"
#include "string.h"

//触摸芯片


static unsigned char hal_get_DB0Sta(void);
static unsigned char hal_get_DB1Sta(void);
static unsigned char hal_get_DB2Sta(void);
static unsigned char hal_get_DB3Sta(void);
static unsigned char hal_get_DB4Sta(void);


unsigned char KeyState[KEYNUM];			    //按键状态
unsigned short KeyScanTime[KEYNUM];		    //去抖延时
unsigned short KeyPressLongTimer[KEYNUM];	//长按延时
unsigned short KeyContPressTimer[KEYNUM];	//连续长按延时	
unsigned char Key_PressValue[KEYNUM];       //芯片按键读取值标志位



//读取GPIO口值，先读DB4是为了左移
Get_Key_Sta get_key_sta[Key_DB_Num] = {
    hal_get_DB4Sta,
    hal_get_DB3Sta,
    hal_get_DB2Sta,
    hal_get_DB1Sta,
    hal_get_DB0Sta
};

KeyEvent_CallBack_t KeyScanCBS;

void hal_KeyScanCBSRegister(KeyEvent_CallBack_t pCBS)
{
	if(KeyScanCBS == 0)
	{
			KeyScanCBS = pCBS;
	}
}	

static void hal_key_Config(void)
{	 
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA, ENABLE); 	

	GPIO_InitStructure.GPIO_Pin  = KEY_DB0_PIN | KEY_DB1_PIN | KEY_DB2_PIN | KEY_DB3_PIN; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
    GPIO_Init(GPIOC, &GPIO_InitStructure);
 
	GPIO_InitStructure.GPIO_Pin  = KEY_DB4_PIN; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
}



static unsigned char hal_get_DB0Sta(void)
{
	return (GPIO_ReadInputDataBit(KEY_DB0_PORT, KEY_DB0_PIN));		
}

static unsigned char hal_get_DB1Sta(void)
{
	return (GPIO_ReadInputDataBit(KEY_DB1_PORT, KEY_DB1_PIN));		
}
 
static unsigned char hal_get_DB2Sta(void)
{
	return (GPIO_ReadInputDataBit(KEY_DB2_PORT, KEY_DB2_PIN));		
}
 
static unsigned char hal_get_DB3Sta(void)
{
	return (GPIO_ReadInputDataBit(KEY_DB3_PORT, KEY_DB3_PIN));		
}
 
static unsigned char hal_get_DB4Sta(void)
{
	return (GPIO_ReadInputDataBit(KEY_DB4_PORT, KEY_DB4_PIN));		
}



static void hal_Key_DBGet_Sta(void)
{
    unsigned char i,keyValue;
    keyValue = 0;
    for (i = 0; i < Key_DB_Num; i++)
    {
        keyValue <<= 1;
        if (get_key_sta[i]())
        {
            keyValue |= 0x01;
            
        }   
    }

    switch (keyValue)
    {
    case TP0:
        Key_PressValue[KEY_MENU] = 1;
        break;

    case TP1:
        Key_PressValue[KEY_DISARM] = 1;
        break;

    case TP2:
        Key_PressValue[KEY_HOMEARM] = 1; 
        break;
        
    case TP3:
        Key_PressValue[KEY_AWARARM] = 1; 
        break;

    case TP4:
        Key_PressValue[KEY_CANCEIL_DAIL] = 1; 
        break;

    case TP5:
        Key_PressValue[KEY9] = 1; 
        break;

    case TP6:
        Key_PressValue[KEY6_RIGHT] = 1; 
        break;

    case TP7:
        Key_PressValue[KEY3] = 1; 
        break;

    case TP8:
        Key_PressValue[KEY0] = 1; 
        break;

    case TP9:
        Key_PressValue[KEY8_DOWN] = 1; 
        break;

    case TP10:
        Key_PressValue[KEY5] = 1; 
        break;

    case TP11:
        Key_PressValue[KEY2_UP] = 1; 
        break;

    case TP12:
        Key_PressValue[KEY_SOS_DEL] = 1; 
        break;

    case TP13:
        Key_PressValue[KEY7] = 1; 
        break;

    case TP14:
        Key_PressValue[KEY4_LEFT] = 1; 
        break;

    case TP15:
        Key_PressValue[KEY1] = 1; 
        break;

    default : 
        memset(Key_PressValue,0,KEYNUM);
        break;
    }
    
}


void hal_key_Init(void)
{
	unsigned char i;

    KeyScanCBS = 0;
    hal_key_Config();
    for(i=0; i<KEYNUM; i++)
    {
        KeyState[i] = KEY_PRESSWAIT;
        KeyScanTime[i] = KEY_SCANTIME;
        KeyPressLongTimer[i] = KEY_PRESSLONGTIME;
        KeyContPressTimer[i] = KEY_CONTPRESSTIME;
    }
}


void hal_KeyProc(void)
{
	unsigned char i,keys;
	unsigned char state;
	
    hal_Key_DBGet_Sta();
	
	for(i=0; i<KEYNUM; i++)
	{	
		keys = 0xff; 
		state = 0;
		switch(KeyState[i])
		{
			case KEY_PRESSWAIT:
				if(Key_PressValue[i])
				{
					KeyState[i] = KEY_PRESSFIRST;	
				}
			break;
			case KEY_PRESSFIRST:
				if(Key_PressValue[i])
				{
					if(!(--KeyScanTime[i]))
					{
						KeyScanTime[i] = KEY_SCANTIME;
						KeyState[i] = KEY_COUNTTIME;
						keys = i;										//记录按键ID号
				 		state = KEY_CLICK;								//按键单击
						 
					}
				}else
				{
					KeyScanTime[i] = KEY_SCANTIME;
					KeyState[i] = KEY_PRESSWAIT;
				}
			break;
			case KEY_COUNTTIME:
				if(Key_PressValue[i])
				{	
					if(!(--KeyPressLongTimer[i]))
					{
						KeyPressLongTimer[i] = KEY_PRESSLONGTIME;
						KeyState[i] = KEY_RELEASEWAIT;
						
						keys = i;										//记录按键ID号
				  	state = KEY_LONG_PRESS;							//长按确认
							
					}
				}else
				{
					KeyPressLongTimer[i] = KEY_PRESSLONGTIME;
					KeyState[i] = KEY_PELEASE;
					keys = i;										//记录按键ID号
				 	state = KEY_CLICK_RELEASE;						//单击释放
				
				}
			break;
			case KEY_RELEASEWAIT:
				if(!Key_PressValue[i])
				{
					KeyState[i] = KEY_PELEASE;
					KeyContPressTimer[i] = KEY_CONTPRESSTIME;
					keys = i;								//记录按键ID号
			 	state = KEY_LONG_PRESS_RELEASE; 		//长按释放
				}else
				{
					if(!--KeyContPressTimer[i])
					{
						KeyContPressTimer[i] = KEY_CONTPRESSTIME;
						keys = i;							//持续长按
				  	state = KEY_LONG_PRESS_CONTINUE;
						 
					}
				}
			break;
			case KEY_PELEASE:
				if(!Key_PressValue[i])
				{
					KeyState[i] = KEY_PRESSWAIT;
				}
			break;			
		}
		
		if(keys != 0xff)
		{
			if(KeyScanCBS)
			{
				KeyScanCBS((EN_KEYNUM)keys,(KEY_VALUE_TYPEDEF)state);    //把按键值和状态值传给pCBS即KeyEventHandle
			}
		}
	}
}


