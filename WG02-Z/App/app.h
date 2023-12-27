#ifndef ___APP_H_
#define ___APP_H_


#define Srceen_Off_Time 1500 

typedef enum
{
	DESKTOP_MENU_POS,	            //原始桌面
}MENU_POS;


//普通菜单列表
typedef enum
{
	GNL_MENU_DESKTOP,		      	//桌面
	// GNL_MENU_DAIL,			    //电话拨出界面
	// GNL_MENU_ENTER_PIN,		    //输入密码界面
	// GNL_MENU_ENTER_DISARM,		//输入密码界面
	// GNL_MENU_FIREWARE_UP,        //固件升级界面
	GNL_MENU_SUM,
}GENERAL_MENU_LIST;			//普通菜单列表


typedef enum
{
	SCREEN_CMD_NULL,		  //无用命令
	SCREEN_CMD_RESET,		  //重置屏显示
	SCREEN_CMD_RECOVER,		  //恢复原来显示
	SCREEN_CMD_UPDATE,		  //更新原来显示
}SCREEN_CMD;		          //刷新屏显示标志

typedef struct
{
	unsigned char keyVal;				//按键值,0xFF代表无按键触发
	unsigned char state;                //按键状态值。
}Key_Str;


typedef struct MODE_MENU
{
	MENU_POS menuPos;				    //当前菜单的位置信息
//	const unsigned char *pModeType;		//指向当前模式类型“desk”
//	void (*action)(void);				//当前模式下的响应函数
	SCREEN_CMD refreshScreenCmd;		//刷新屏显示命令
	Key_Str Key_Par;
	struct MODE_MENU *pLase;			//指向上一个选项
	struct MODE_MENU *pNext;			//指向下一个选项
	struct MODE_MENU *pParent;		    //指向父级菜单
	struct MODE_MENU *pChild;			//指向子级菜单
}stu_mode_menu;


void app_task_init(void);
void app_task(void);

#endif
