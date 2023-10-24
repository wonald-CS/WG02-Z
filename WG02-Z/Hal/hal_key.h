#ifndef _HAL_KEY_H_
#define _HAL_KEY_H_

#define  KEY_DB0_PORT GPIOC
#define  KEY_DB0_PIN  GPIO_Pin_6

#define  KEY_DB1_PORT GPIOC
#define  KEY_DB1_PIN  GPIO_Pin_7

#define  KEY_DB2_PORT GPIOC
#define  KEY_DB2_PIN  GPIO_Pin_8

#define  KEY_DB3_PORT GPIOC
#define  KEY_DB3_PIN  GPIO_Pin_9

#define  KEY_DB4_PORT GPIOA
#define  KEY_DB4_PIN  GPIO_Pin_8


//按键消抖时间,以10ms为Tick,3=30ms
#define KEY_SCANTIME	    3		//30ms

//连续长按时间
#define	KEY_PRESSLONGTIME	200	//2s

//持续长按间隔时间
#define KEY_CONTPRESSTIME	150	//1.5秒


enum{
    TP0 = 0x01,
    TP1,
    TP2,
    TP3,
    TP4,
    TP5,
    TP6,
    TP7,
    TP8,
    TP9,
    TP10,
    TP11,
    TP12,
    TP13,
    TP14,
    TP15
};

enum{
    Key_DB4,
    Key_DB3,
    Key_DB2,
    Key_DB1,
    Key_DB0,
    Key_DB_Num
};


typedef enum
{
	KEY_MENU,
	KEY_DISARM,
	KEY_HOMEARM,
	KEY_AWARARM,
	
	KEY_CANCEIL_DAIL,
	KEY9,
	KEY6_RIGHT,
	KEY3,
	
	KEY0,
	KEY8_DOWN,
	KEY5,	
    KEY2_UP,	

    KEY_SOS_DEL,	
	KEY7,
	KEY4_LEFT,
	KEY1,	

	KEYNUM
}EN_KEYNUM;			//触摸按键数量，顺序是根据电路板按键对应面板的数字键 


// 按键检测过程
enum
{
	KEY_PRESSWAIT=0,			//等待按键
	KEY_PRESSFIRST,				//按键按下
	KEY_COUNTTIME,				//按下计时
	KEY_RELEASEWAIT,  			//等待释放
	KEY_PELEASE					//释放消抖
};



typedef enum
{	
	KEY_WAIT,       	 		 					   	//等待按键
	KEY_CLICK,          								//单击确认
	KEY_CLICK_RELEASE,            			//单击释放
	KEY_LONG_PRESS,			   						 	//长按确认
	KEY_LONG_PRESS_CONTINUE,						//长按持续
	KEY_LONG_PRESS_RELEASE							//长按释放
	 
}KEY_VALUE_TYPEDEF;



typedef unsigned char (*Get_Key_Sta)(); 
typedef void (*KeyEvent_CallBack_t)(EN_KEYNUM keys,KEY_VALUE_TYPEDEF sta);


void hal_key_Init(void);
void hal_KeyProc(void);
void hal_KeyScanCBSRegister(KeyEvent_CallBack_t pCBS);

#endif
