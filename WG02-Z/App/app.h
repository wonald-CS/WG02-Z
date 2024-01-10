#ifndef _APP_H_
#define _APP_H_

////=============================================================================
////WIFI工作状态
////=============================================================================
//#define         SMART_CONFIG_STATE              0x00
//#define         AP_STATE                        0x01
//#define         WIFI_NOT_CONNECTED              0x02
//#define         WIFI_CONNECTED                  0x03
//#define         WIFI_CONN_CLOUD                 0x04
//#define         WIFI_LOW_POWER                  0x05
//#define         SMART_AND_AP_STATE              0x06
//#define         WIFI_SATE_UNKNOW                0xff


//#define SETUPMENU_TIMEOUT_PERIOD	2000			//设置菜单没任何操作自动返回桌面时间，20秒
//#define PUTOUT_SCREEN_PERIOD			3000			//没任何操作熄屏时间,30秒
// //探测器码定义
////门磁探测器 2262编码
//#define SENSOR_CODE_DOOR_OPEN		0x0A		 //开门	
//#define SENSOR_CODE_DOOR_CLOSE		0x0E		 //关门
//#define SENSOR_CODE_DOOR_TAMPER		0x07		 //防拆,预留
//#define	SENSOR_CODE_DOOR_LOWPWR		0x06		 //低压


////遥控器 EV1527编码
//#define SENSOR_CODE_REMOTE_ENARM				0x02	 
//#define SENSOR_CODE_REMOTE_DISARM				0x01		 
//#define SENSOR_CODE_REMOTE_HOMEARM				0x04
//#define SENSOR_CODE_REMOTE_SOS					0x08


//#define SENSOR_CODE_PIR						0x03
//#define SENSOR_CODE_PIR_TAMPER				0x0B
//#define SENSOR_CODE_PIR_LOWPWR				0x05

////----菜单相关声明区域
//typedef enum
//{
//	SCREEN_CMD_NULL,		  //无用命令
//	SCREEN_CMD_RESET,		  //重置屏显示
//	SCREEN_CMD_RECOVER,		  //恢复原来显示
//	SCREEN_CMD_UPDATE,		  //更新原来显示
//}SCREEN_CMD;		          //刷新屏显示标志


////普通菜单列表
//typedef enum
//{

//	GNL_MENU_DESKTOP,		      //桌面
//  GNL_MENU_DAIL,					  //电话拨出界面
//	GNL_MENU_ENTER_PIN,				//输入密码界面
//	GNL_MENU_ENTER_DISARM,				//输入密码界面
//	GNL_MENU_SUM,
//}GENERAL_MENU_LIST;			//普通菜单列表

////设置菜单列表ID
//typedef enum
//{
//	STG_MENU_MAIN_SETTING,
//	STG_MENU_LEARNING_SENSOR,
//	STG_MENU_DTC_LIST,
//	STG_MENU_WIFI,
//	STG_MENU_TIME,
//	STG_MENU_MACHINE_INFO,
//	STG_MENU_PASSWORD,
//	STG_MENU_PHONE_NUMBER,
//	STG_MENU_ALARM_RECORD,
//	STG_MENU_FACTORY_SETTINGS,
//	STG_MENU_SUM
//}STG_MENU_LIST;

////3.DTC List->Zone xxx->Review
//typedef enum
//{
//	STG_MENU_DL_ZX_REVIEW_MAIN,
//	STG_MENU_DL_ZX_REVIEW,
//	STG_MENU_DL_ZX_EDIT,
//	STG_MENU_DL_ZX_DELETE,
//	STG_MENU_DL_ZX_SUM,
//}STG_MENU_DZ_ZX_LIST;

////定义菜单的位置，主要用于超时退出判断
//typedef enum
//{
//	DESKTOP_MENU_POS,	//桌面
//	STG_MENU_POS,
//	STG_WIFI_MENU_POS,
//	STG_SUB_2_MENU_POS,
//	STG_SUB_3_MENU_POS,
//	STG_SUB_4_MENU_POS,
//	STG_SUB_ALARMING_POS,
//}MENU_POS;



//typedef struct MODE_MENU
//{
//	unsigned char ID;				//菜单唯一ID号
//	MENU_POS menuPos;				//当前菜单的位置信息
//	const unsigned char *pModeType;		//指向当前模式类型
//	void (*action)(void);				//当前模式下的响应函数
//	SCREEN_CMD refreshScreenCmd;		//刷新屏显示命令
//	unsigned char reserved;				//预留，方便参数传递
//	unsigned char keyVal;				//按键值,0xFF代表无按键触发
//	unsigned char state;
//	struct MODE_MENU *pLase;			//指向上一个选项
//	struct MODE_MENU *pNext;			//指向下一个选项
//	struct MODE_MENU *pParent;		//指向父级菜单
//	struct MODE_MENU *pChild;			//指向子级菜单
//}stu_mode_menu;
////---------------------------------
////安防系统模式
//typedef enum 
//{
//	SYSTEM_MODE_ENARM_HOST,	  //离家布放
////	SYSTEM_MODE_ENARM_REMOTE,//遥控器布防
////	SYSTEM_MODE_ENARM_APP,   //APP布防
//	
//	SYSTEM_MODE_HOMEARM_HOST,	   //在家布防
////	SYSTEM_MODE_HOMEARM_REMOTE,//遥控器在家布防
////	SYSTEM_MODE_HOMEARM_APP,  //APP在家布防
//	
//	SYSTEM_MODE_DISARM_HOST,		        //撤防
////	SYSTEM_MODE_DISARM_REMOTE,		//遥控器撤防
////	SYSTEM_MODE_DISARM_APP,		    //APP撤防

//	SYSTEM_MODE_ALARM,		//报警中
//	SYSTEM_MODE_SUM
//}SYSTEMMODE_TYPEDEF;

//typedef enum
//{
//	SYSTEM_MODE_OPER_HOSTKEY, ///主机键盘
//  SYSTEM_MODE_OPER_REMOTE,  ///遥控器操作
//  SYSTEM_MODE_OPER_APP,     ///服务器  APP 远程控制
//}EN_SYSTEMODE_OPTERTYPEDEF;



//enum
//{
//	SYSTEMALARM_MODE_ALARM,
//	SYSTEMALARM_MODE_DISARM,
//	SYSTEMALARM_MODE_SUM,	
//};

//typedef struct SYSTEM_MODE
//{
//	SYSTEMMODE_TYPEDEF ID;
//	SCREEN_CMD refreshScreenCmd;//刷新屏显示命令
//	unsigned char keyVal;				//按键值,0xFF代表无按键触发
//	void (*action)(void);				//当前模式下的响应函数
//}stu_system_mode;

//extern stu_system_mode *pStuSystemMode;		    //

//#define GET_SYSTEM_WORK_MODE  pStuSystemMode->ID

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



void AppInit(void);
void AppProc(void);
//void mcu_all_dp_update(void);
//void SystemMode_Change(unsigned char zone,SYSTEMMODE_TYPEDEF sysMode,EN_SYSTEMODE_OPTERTYPEDEF operType);
#endif

