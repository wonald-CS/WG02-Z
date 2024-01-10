#ifndef _PARA_H
#define _PARA_H



#define DEC_OFFLINE_TIME    240// 86400    //24小时     单位:86400(秒)


typedef enum
{
	DET_HAVED_LEARN,  ///已经学习
	DET_UNLEARN,   ///未学习   学习成功
	DET_LEARN_FAIL, ///学习失败
}en_applyNetSta;


typedef struct 
{
	en_applyNetSta state;
	unsigned char code;  //保存保存的数组的下标
}str_ParaDetectorApplyNetState;





#define TYPE_PRODUCT_UNMBER   0x0001      //产品型号
#define GatewayProtocolVer    0x0064      //协议版本 100  V1.00   655.36
#define MCU_HARDVER_VERSION   0x0064      //硬件版本

//探测器类型
typedef enum
{
	DTC_DOOR = 1,		     //遥控器
	DTC_REMOTE,			     //门磁探测器
	DTC_TYP_SUM,
}DTC_TYPE_TYPEDEF;	//探测器类型设置值

//探测器防区类型
typedef enum
{
	ZONE_TYP_24HOURS,	//24小时警戒	
	ZONE_TYP_1ST,		//布防警戒
	ZONE_TYP_2ND,		//在家布放警戒
	STG_DEV_AT_SUM  //
}ZONE_TYPED_TYPEDEF;

typedef struct Stu_SystemPara
{///系统参数
	unsigned char PhoneNumber[7][20];	//电话号码,共7组
	unsigned char AdminPassword[4];		//管理员密码,必须是4位
	unsigned char firmware[2];        //固件版本号 0-655.36
}SystemPara_InitTypeDef;

#define STU_SYSTEMPARA_SIZE	sizeof(SystemPara_InitTypeDef)	

//探测器属性
typedef struct
{
	unsigned char ID;					  //设备ID
	unsigned char Mark;		 			  //0-未学习 1-已学习
	unsigned char NameNum;				//名称序号
	unsigned char Name[26];				//设备名称 16 10
	DTC_TYPE_TYPEDEF DTCType;			//设备类型
	ZONE_TYPED_TYPEDEF ZoneType;		//防区类型
	unsigned char Code[12];		//MAC ID			        //
	unsigned char node[2];    //节点号
	unsigned int sleepTimes;   //离线时间计时器
}Stu_DTC;
#define STU_DTC_SIZE	sizeof(Stu_DTC)





#define  PARA_DTC_SUM		  20	//支持探测器的总数量

extern SystemPara_InitTypeDef mt_sEPR;  ///系统参数
extern Stu_DTC dERP[PARA_DTC_SUM];	//EEPROM设备参数数据结构
//设置系统参数
#define MT_SET_PARA_SYS_PHONENUMBER(x,i,val)	(mt_sEPR.PhoneNumber[x][i]=val)
#define MT_SET_PARA_SYS_ADMINPASSWORD(i,val)	(mt_sEPR.AdminPassword[i]=val)
#define MT_SET_PARA_SYS_FIRMWARE(i,val)         (mt_sEPR.firmware[i]=val)

//获取系统参数
#define MT_GET_PARA_SYS_PHONENUMBER(x,i)		(mt_sEPR.PhoneNumber[x][i])
#define MT_GET_PARA_SYS_ADMINPASSWORD(i)		(mt_sEPR.AdminPassword[i])
#define MT_GET_PARA_SYS_FIRMWARE(i)             (mt_sEPR.firmware[i])

//设置设备参数
#define MT_SET_PARA_DEVICE_ID(x,val)							  (dERP[x].ID=val)
#define MT_SET_PARA_DEVICE_MARK(x,val)							(dERP[x].Mark=val)
#define MT_SET_PARA_DEVICE_NAMENUM(x,val)						(dERP[x].NameNum=val)
#define MT_SET_PARA_DEVICE_NAME_BYTE(x,i,val)				(dERP[x].Name[i]=val)		 
#define MT_SET_PARA_DEVICE_ZONETYPE(x,val)			    (dERP[x].DTCType=val)
#define MT_SET_PARA_DEVICE_ALARMTYPE(x,val)				  (dERP[x].ZoneType=val)		
#define MT_SET_PARA_DEVICE_CODE(x,i,val)						(dERP[x].Code[i]=val)
#define MT_SET_PARA_DEVICE_SLEEPTIME(x,val)				  (dERP[x].sleepTimes = val)

//获取设备参
#define MT_GET_PARA_DEVICE_ID(x)							      (dERP[x].ID)
#define MT_GET_PARA_DEVICE_MARK(x)							    (dERP[x].Mark)
#define MT_GET_PARA_DEVICE_NAMENUM(x)						    (dERP[x].NameNum)
#define MT_GET_PARA_DEVICE_NAME_BYTE(x,i)					  (dERP[x].Name[i])		 
#define MT_GET_PARA_DEVICE_ZONETYPE(x)						  (dERP[x].DTCType)
#define MT_GET_PARA_DEVICE_ALARMTYPE(x)						  (dERP[x].ZoneType)	
#define MT_GET_PARA_DEVICE_CODE(x,i)						    (dERP[x].Code[i])
#define MT_GET_PARA_DEVICE_SLEEPTIME(x)						  (dERP[x].sleepTimes)


//系统参数在EEPROM的地址偏移
#define STU_PRODUCT_OFFSET		0
#define STU_SYSTEMPARA_OFFSET	STU_PRODUCT_OFFSET   //系统参数起始地址
#define STU_DEVICEPARA_OFFSET	STU_SYSTEMPARA_SIZE  //探测器属性 



typedef struct SYSTEM_TIME
{
	unsigned short year;
	unsigned char mon;
	unsigned char day;
	unsigned char week;
	unsigned char hour;
	unsigned char min;
	unsigned char sec;
}stu_system_time;

extern volatile stu_system_time stuSystemtime;

#define MT_SET_DATE_YEAR(x)			(stuSystemtime.year=x)
#define MT_SET_DATE_MON(x)			(stuSystemtime.mon=x)
#define MT_SET_DATE_DAY(x)			(stuSystemtime.day=x)
#define MT_SET_DATE_WEEK(x)			(stuSystemtime.week=x)
#define MT_SET_DATE_HOUR(x)			(stuSystemtime.hour=x)
#define MT_SET_DATE_MIN(x)			(stuSystemtime.min=x)
#define MT_SET_DATE_SEC(x)			(stuSystemtime.sec=x)

#define MT_GET_DATE_YEAR()			(stuSystemtime.year)
#define MT_GET_DATE_MON()			  (stuSystemtime.mon)
#define MT_GET_DATE_DAY()			  (stuSystemtime.day)
#define MT_GET_DATE_WEEK()			(stuSystemtime.week)
#define MT_GET_DATE_HOUR()			(stuSystemtime.hour)
#define MT_GET_DATE_MIN()			  (stuSystemtime.min)
#define MT_GET_DATE_SEC()			  (stuSystemtime.sec)


#define STM32_UID_ADDR		0x1ffff7e8
extern unsigned char STM32_UID[12];///UID[0] =0XFF
#define MT_GET_MCU_UID(i)	(STM32_UID[i])

void ParaInit(void);
void SystemTime_local(void);
str_ParaDetectorApplyNetState AddDtc(Stu_DTC *pDevPara);

void GetDtcStu(Stu_DTC *pdDevPara, unsigned char idx);
unsigned char CheckPresenceofDtc(unsigned char i);
void SetDtcAbt(unsigned char id,Stu_DTC *psDevPara);
void mt_para_FactoryReset(void);
unsigned char DtcMatching(unsigned char *pCode);
#endif
