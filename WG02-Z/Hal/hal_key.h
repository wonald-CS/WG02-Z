#ifndef __HAL_KEY_H_
#define __HAL_KEY_H_

enum
{
	K_BCD_DB0,
	K_BCD_DB1,
	K_BCD_DB2,
	K_BCD_DB3,
	K_BCD_DB4,
	K_BCD_NUM,
};		//触摸IC BCD数据数据


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
	KEY_MENU,
	KEY_DISARM,
	KEY_HOMEARM,
	KEY_AWARARM,
	
	KEY_RETURN_DAIL,
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

						
typedef struct
{
	unsigned char key;
	unsigned char state;
}key_msg_hdr_t;

typedef enum
{	
	KEY_WAIT,       	 		 						//等待按键
	KEY_CLICK,          								//单击确认
	KEY_CLICK_RELEASE,            						//单击释放
	KEY_LONG_PRESS,			   						 	//长按确认
	KEY_LONG_PRESS_CONTINUE,							//长按持续
	KEY_LONG_PRESS_RELEASE								//长按释放
	 
}KEY_VALUE_TYPEDEF;

typedef void (*KeyEvent_CallBack_t)(EN_KEYNUM keys,KEY_VALUE_TYPEDEF sta);
         
//按键消抖时间,以10ms为Tick,3=30ms
#define KEY_SCANTIME	    3		//30ms

//连续长按时间
#define	KEY_PRESSLONGTIME	200	//2s

//持续长按间隔时间
#define KEY_CONTPRESSTIME	150	//1.5秒
		
//extern unsigned char registeredKeysTaskID;
		
void hal_KeyScanCBSRegister(KeyEvent_CallBack_t pCBS);
void hal_keyInit(void);
void hal_KeyProc(void);


#endif 

