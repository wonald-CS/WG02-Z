#include "stm32F10x.h"
#include "string.h"
#include "hal_key.h"


#define KEY_DB0_PORT   GPIOC
#define KEY_DB0_PIN    GPIO_Pin_6

#define KEY_DB1_PORT   GPIOC
#define KEY_DB1_PIN    GPIO_Pin_7

#define KEY_DB2_PORT   GPIOC
#define KEY_DB2_PIN    GPIO_Pin_8

#define KEY_DB3_PORT   GPIOC
#define KEY_DB3_PIN    GPIO_Pin_9

#define KEY_DB4_PORT   GPIOA
#define KEY_DB4_PIN    GPIO_Pin_8

static unsigned char hal_getDB0Sta(void);
static unsigned char hal_getDB1Sta(void);
static unsigned char hal_getDB2Sta(void);
static unsigned char hal_getDB3Sta(void);
static unsigned char hal_getDB4Sta(void);

unsigned char (*getKeysVal[K_BCD_NUM])() =
 { 
	hal_getDB4Sta,
	hal_getDB3Sta,
	hal_getDB2Sta,
	hal_getDB1Sta,
	hal_getDB0Sta
};

unsigned char KeyState[KEYNUM];			//按键状态
unsigned short KeyScanTime[KEYNUM];		//去抖延时
unsigned short KeyPressLongTimer[KEYNUM];	//长按延时
unsigned short KeyContPressTimer[KEYNUM];	//连续长按延时	

KeyEvent_CallBack_t KeyScanCBS;


static void hal_keyConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); 
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = KEY_DB4_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	

	GPIO_InitStructure.GPIO_Pin = KEY_DB0_PIN| KEY_DB1_PIN | KEY_DB2_PIN | KEY_DB3_PIN;;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}


static unsigned char hal_getDB0Sta(void)
{
	return (GPIO_ReadInputDataBit(KEY_DB0_PORT, KEY_DB0_PIN));		
}

static unsigned char hal_getDB1Sta(void)
{
	return (GPIO_ReadInputDataBit(KEY_DB1_PORT, KEY_DB1_PIN));		
}
 
static unsigned char hal_getDB2Sta(void)
{
	return (GPIO_ReadInputDataBit(KEY_DB2_PORT, KEY_DB2_PIN));		
}
 
static unsigned char hal_getDB3Sta(void)
{
	return (GPIO_ReadInputDataBit(KEY_DB3_PORT, KEY_DB3_PIN));		
}
 
static unsigned char hal_getDB4Sta(void)
{
	return (GPIO_ReadInputDataBit(GPIOA, KEY_DB4_PIN));		
}
////


/////////////////////////////
//函数功能: 获取触摸按键DB0-DB4的信号值 并将DCB值转换成按键值
//形参：unsigned char *KeyLineState  16按键的值  和WJ03的代码一致
static void hal_LoadKeyState(unsigned char *KeyLineState)
{
	unsigned char i,KeyVal;
	KeyVal = 0;
    ////获取DB0-DB4的值
	for(i=0; i<K_BCD_NUM; i++)
	{
		KeyVal <<= 1;	
		if((*(getKeysVal[i]))())
		{
			KeyVal |= 0x01;
		}
	}
	////以下代码是将 DB0-DB4读取的值和我们的按键对应起来
	switch(KeyVal)
	{
		case 0x01:			//0
			KeyLineState[KEY_MENU] = 1;
		break;
		case 0x02:			//1
			KeyLineState[KEY_DISARM] = 1;
		break;
		case 0x03:			//2
			KeyLineState[KEY_HOMEARM] = 1;
		break;
		case 0x04:			//3
			KeyLineState[KEY_AWAYARM] = 1;
		break;
		case 0x05:			//4
			KeyLineState[KEY_RETURN_DAIL] = 1;
		break;
		case 0x06:			//5
			KeyLineState[KEY9] = 1;
		break;
		case 0x07:			//6
			KeyLineState[KEY6_RIGHT] = 1;
		break;
		case 0x08:			//7
			KeyLineState[KEY3] = 1;
		break;
		case 0x09:			//8
			KeyLineState[KEY0] = 1;
		break;
		case 0x0A:			//9
			KeyLineState[KEY8_DOWN] = 1;
		break;
		case 0x0B:		
			KeyLineState[KEY5] = 1;
		break;			
		case 0x0C:			// 
			KeyLineState[KEY2_UP] = 1;
		break;	
		case 0x0D:			// 
			KeyLineState[KEY_SOS_DEL] = 1;
		break;
		case 0x0E:			// 
			KeyLineState[KEY7] = 1;
		break;
		case 0x0F:			// 
			KeyLineState[KEY4_LEFT] = 1;
		break;
		case 0x10:			// 
			KeyLineState[KEY1] = 1;
		break;
	}
}
/////




void hal_KeyScanCBSRegister(KeyEvent_CallBack_t pCBS)
{
	if(KeyScanCBS == 0)
	{
		KeyScanCBS = pCBS;
	}
}

void hal_keyInit(void)
{
	unsigned char i;
	KeyScanCBS = 0;
	hal_keyConfig();
	for(i=0; i<KEYNUM; i++)
	{
		KeyState[i] = KEY_PRESSWAIT;
		KeyScanTime[i] = KEY_SCANTIME;
		KeyPressLongTimer[i] = KEY_PRESSLONGTIME;
		KeyContPressTimer[i] = KEY_CONTPRESSTIME;
	}
}

	
void hal_KeyProc(void)//WJ03
{
	unsigned char i,KeyLineTemp[KEYNUM],keys;
	unsigned char state;
	memset(KeyLineTemp, 0,KEYNUM);
	hal_LoadKeyState(KeyLineTemp);
	
	for(i=0; i<KEYNUM; i++)
	{	
		keys = 0xff; 
		state = 0;
		switch(KeyState[i])
		{
			case KEY_PRESSWAIT:
				if(KeyLineTemp[i])
				{
					KeyState[i] = KEY_PRESSFIRST;	
				}
			break;
			case KEY_PRESSFIRST:
				if(KeyLineTemp[i])
				{
					if(!(--KeyScanTime[i]))
					{
						KeyScanTime[i] = KEY_SCANTIME;
						KeyState[i] = KEY_COUNTTIME;
						
						keys = i;										//记录按键ID号
				 		state = KEY_CLICK;								//按键单击 
					}
				}
				else
				{
					KeyScanTime[i] = KEY_SCANTIME;
					KeyState[i] = KEY_PRESSWAIT;
				}
			break;
			case KEY_COUNTTIME:
				if(KeyLineTemp[i])
				{	
					if(!(--KeyPressLongTimer[i]))
					{
						KeyPressLongTimer[i] = KEY_PRESSLONGTIME;
						KeyState[i] = KEY_RELEASEWAIT;
						
						keys = i;										//记录按键ID号
				  	    state = KEY_LONG_PRESS;							//长按确认
							
					}
				}
				else
				{
					KeyPressLongTimer[i] = KEY_PRESSLONGTIME;
					KeyState[i] = KEY_PELEASE;
					
					keys = i;										//记录按键ID号
				 	state = KEY_CLICK_RELEASE;						//单击释放
				
				}
			break;
			case KEY_RELEASEWAIT:
				if(!KeyLineTemp[i])
				{
					KeyState[i] = KEY_PELEASE;
					KeyContPressTimer[i] = KEY_CONTPRESSTIME;
					
					keys = i;								//记录按键ID号
			 	    state = KEY_LONG_PRESS_RELEASE; 		//长按释放
				}
				else
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
				if(!KeyLineTemp[i])
				{
					KeyState[i] = KEY_PRESSWAIT;
				}
			break;			
		}
		
		if(keys != 0xff)
		{
			if(KeyScanCBS)
			{
				KeyScanCBS((EN_KEYNUM)keys,(KEY_VALUE_TYPEDEF)state);
			}
		}
	}
}






