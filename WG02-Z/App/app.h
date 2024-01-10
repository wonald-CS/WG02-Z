#ifndef ___APP_H_
#define ___APP_H_

////APP.H
#define SETUPMENU_TIMEOUT_PERIOD 2000   //设置菜单没任何操作自动返回桌面时间，20秒
#define PUTOUT_SCREEN_PERIOD	3000	//没任何操作熄屏时间,30秒
//定义菜单的位置，主要用于超时退出判断
typedef enum
{
	DESKTOP_MENU_POS,	//桌面
	STG_SUB_ALARMING_POS,
	STG_SUB_FIREWARE_UP,
	STG_SUB_WIFI_MENU_POS, ///WIFI配网
	STG_SUB_MENU_POS,
	STG_SUB_1_MENU_POS,
	STG_SUB_2_MENU_POS,
}MENU_POS;
//----菜单相关声明区域
typedef enum
{
	SCREEN_CMD_NULL,		  //无用命令
	SCREEN_CMD_RESET,		  //重置屏显示
	SCREEN_CMD_RECOVER,		  //恢复原来显示
	SCREEN_CMD_UPDATE,		  //更新原来显示
}SCREEN_CMD;		          //刷新屏显示标志

typedef struct MODE_MENU
{
	unsigned char ID;				    //菜单唯一ID号
	MENU_POS menuPos;				    //当前菜单的位置信息
	const unsigned char *pModeType;		//指向当前模式类型“desk”
	void (*action)(void);				//当前模式下的响应函数
	SCREEN_CMD refreshScreenCmd;		//刷新屏显示命令
	unsigned char reserved;				//预留，方便参数传递
	unsigned char keyVal;				//按键值,0xFF代表无按键触发
	unsigned char state;                //按键状态值。
	struct MODE_MENU *pLase;			//指向上一个选项
	struct MODE_MENU *pNext;			//指向下一个选项
	struct MODE_MENU *pParent;		    //指向父级菜单
	struct MODE_MENU *pChild;			//指向子级菜单
}stu_mode_menu;
//---------------------------------

//普通菜单列表
typedef enum
{
	GNL_MENU_DESKTOP,		      //桌面
    GNL_MENU_DAIL,					  //电话拨出界面
    GNL_MENU_ENTER_PIN,				//输入密码界面
    GNL_MENU_ENTER_DISARM,				//按键撤防界面
    GNL_MENU_FIREWARE_UP,         //固件升级界面
	GNL_MENU_SUM,
}GENERAL_MENU_LIST;			//普通菜单列表

//设置菜单列表ID   app.h
typedef enum
{
	STG_MENU_MAIN_SETTING = 0,   //主菜单
	STG_MENU_LEARNING_SENSOR,//探测器学习配对
	STG_MENU_DTC_LIST,//探测器属性
	STG_MENU_WIFI,
    STG_MENU_PASSWORD,
	STG_MENU_PHONE_NUMBER,
	STG_MENU_MACHINE_INFO,
	STG_MENU_FACTORY_SETTINGS,
	STG_MENU_ALARM_RECORD,
	STG_MENU_SUM  //
}STG_MENU_LIST;


typedef enum
{
	LORAMODE_APPLEY_NET,     //申请入网
	LORAMODE_APPLEY_NET_OK,  //配网完成OK 
	LORAMODE_NORMAL,///普通模式   正常通讯的模式
}en_loraWorkMode;

typedef struct SYSTEM_LEARN
{
	en_loraWorkMode state;  ///学习状态
	unsigned char detectorType;//探测器类型
    unsigned char LearnSta;  //学习的状态
}stu_loraDtector;

////探测器属性
typedef enum
{
	STG_MENU_DL_ZX_REVIEW_MAIN,
	STG_MENU_DL_ZX_REVIEW,
	STG_MENU_DL_ZX_EDIT,
	STG_MENU_DL_ZX_DELETE,
	STG_MENU_DL_ZX_SUM,
}STG_MENU_DZ_ZX_LIST;



//安防系统模式
typedef enum 
{
	SYSTEM_MODE_ENARM_HOST,	  //离家布放
	SYSTEM_MODE_HOMEARM_HOST,	  //在家布防
	SYSTEM_MODE_DISARM_HOST,	  //撤防
	SYSTEM_MODE_ALARM,		  //报警中
	SYSTEM_MODE_SUM
}SYSTEMMODE_TYPEDEF;

typedef enum
{
  SYSTEM_MODE_OPER_HOSTKEY, ///主机键盘
  SYSTEM_MODE_OPER_REMOTE,  ///遥控器操作
  SYSTEM_MODE_OPER_APP,     ///服务器  APP 远程控制
}EN_SYSTEMODE_OPTERTYPEDEF;


typedef struct SYSTEM_MODE
{
	SYSTEMMODE_TYPEDEF ID;
	SCREEN_CMD refreshScreenCmd;     //刷新屏显示命令
	unsigned char keyVal;		//按键值,0xFF代表无按键触发
	void (*action)(void);		//当前模式下的响应函数
}stu_system_mode;


enum
{
	SYSTEMALARM_MODE_ALARM,
	SYSTEMALARM_MODE_DISARM,
	SYSTEMALARM_MODE_SUM,	
};



typedef enum
{
	ALARM_DAILSMS_STA_INIT,  ////初始化
	ALARM_DAILSMS_STA_ALARMING,////系统正在报警
}un_AlarmDailSmsSta;

typedef enum
{
	ALARMING_IDLE,  ///空闲
	ALARMING_START, ///开始发送短信 和拨打电话
	ALARMING_SMS, //发送短信
	ALARMING_DIAL,//拨打电话
	ALARMING_DIALING,//正在拨打电话中
	ALARMING_DIAL_WAITNEXT,//拨打下一组电话
}un_alarmsms;

#define SYSTEM_ALARM_CYCLETIMS_MAX    3

//initflag  0， 拨打电话 SMS 初始化。
typedef struct
{
	un_alarmsms alarsmsSta;  //报警打电话 发送短信逻辑处理的状态
	unsigned char cycleTims; //循环次数 发送短信循环1次 打电话循环3次
	unsigned char serialNum; //序号 拨打报警电话的序号
	unsigned char smsState;  //发送短信的状态 暂时未用到
}srt_alarmDialSms;

///
typedef enum
{
	SMS_TEXT_DISARM_SUCCESS=0,
	SMS_TEXT_AWAYARM_SUCCESS,	
	SMS_TEXT_HOMEARM_SUCCESS,
	SMS_TEXT_DISARM_MODE,
	SMS_TEXT_AWAYARM_MODE,
	SMS_TEXT_HOMEARM_MODE,
	SMS_TEXT_ALARM_MODE,
	SMS_TEXT_WIFI_DISCONNECT,			//WiFi????
	SMS_TEXT_WIFI_CONNECT,				//WiFi????
	SMS_TEXT_MAX,
}SMS_TEXT_TYPEDEF;		//??????

extern stu_system_mode *pStuSystemMode;
#define GET_SYSTEM_WORK_MODE  pStuSystemMode->ID


void app_task_init(void);
void app_task(void);
void SystemMode_Change(unsigned char zone,SYSTEMMODE_TYPEDEF sysMode,EN_SYSTEMODE_OPTERTYPEDEF operType);
#endif



