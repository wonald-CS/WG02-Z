#ifndef ___MT_PROTOCOL_H_
#define ___MT_PROTOCOL_H_

#define SYSTEM_TIME_UTC   0x08 



typedef struct
{
	unsigned char event;
	unsigned char buffer[1];
}str_mqtt_Upevent;





typedef enum
{///系统报警端点数据	
	//MQTT_UPEVENT_GET_TIME0,
	ENDPOINT_UP_QUERY_NEW_FIRMWARE = 0,
	ENDPOINT_UP_TYPE_GET_TIME,   ///获取系统时间
	
	ENDPOINT_UP_TYPE_DETECTOR_ALARM,  ///探测器报警 detector
	ENDPOINT_UP_TYPE_DETECTOR_OFFLINE,  ///探测器报警 detector
	ENDPOINT_UP_TYPE_DETECTOR_ONLINE,  ///探测器报警 detector
	ENDPOINT_UP_TYPE_DETECTOR_BATLOW,///探测器电池低压
	ENDPOINT_UP_TYPE_DETECTOR_ALARM_TEMPER,///探测器防拆报警
	ENDPOINT_UP_TYPE_HOST_ALARM_SOS, ///主机SOS报警
	ENDPOINT_UP_TYPE_HOST_BATLOW,///主机电池低压
	ENDPOINT_UP_TYPE_HOST_ACREMOVE,///主机AC移除
	ENDPOINT_UP_TYPE_HOST_ACRESTORE,///主机AC恢复
	ENDPOINT_UP_TYPE_DOOR_OPEN,     ///探测器门磁打开
	ENDPOINT_UP_TYPE_DOOR_CLOSE,    ///探测器门磁关闭	
 
	ENDPOINT_UP_ARM_TYPE_ARM_BY_HOST,         ///通过主机 操作布撤防
	ENDPOINT_UP_ARM_TYPE_ARM_BY_REMOTE,       ///通过遥控器 操作布撤防
	ENDPOINT_UP_ARM_TYPE_ARM_BY_APP,          ///通过APP 操作布撤防

	ENDPOINT_UP_ARM_TYPE_HOMEARM_BY_HOST,     ///通过主机 操作布撤防
	ENDPOINT_UP_ARM_TYPE_HOMEARM_BY_REMOTE,   ///通过遥控器 操作布撤防
	ENDPOINT_UP_ARM_TYPE_HOMEARM_BY_APP,      ///通过APP 操作布撤防	

	ENDPOINT_UP_ARM_TYPE_DISARM_BY_HOST,     ///通过主机 操作布撤防
	ENDPOINT_UP_ARM_TYPE_DISARM_BY_REMOTE,   ///通过遥控器 操作布撤防
	ENDPOINT_UP_ARM_TYPE_DISARM_BY_APP,      ///通过APP 操作布撤防		
		
	ENDPOINT_UP_TYPE_SUM,
}en_endpoint_up_Type;




//网关协议相关命令
//M 表示主机发送指令
//S 表示服务器下发指令
//R 表示主机回复指令
typedef enum
{
	GLINK_M_CMD_GETDATE = 0x29,//MCU请求获取服务器时间指令
	GLINK_S_CMD_GETDATE	= 0x2A,//服务器回复时间指令		
	
	GLINK_M_CMD_QUERY_UPDATE	= 0x21,			//服务器通知MCU升级
	GLINK_S_CMD_INFORM_UPDATE	= 0x22,			//服务器通知MCU升级

	GLINK_S_CMD_UPDATE_DATA = 0x24,			//向服务器获取固件数据包
    GLINK_R_CMD_UPDATE_DATA = 0x25,		    //服务器回复固件数据包
	
	GLINK_S_CMD_GATEWAY	= 0x50,	//网关协议命令
	GLINK_R_CMD_GATEWAY	= 0x51, //网关协议回复
	
}en_mtprotocol_cmd;

////mt_protocol.h 文件
typedef enum
{
	GNL_ENDPOINT_DEVICE_INFO = 1,	//端点-设备信息
	GNL_ENDPOINT_SYSTEM_MODE,		 //端点-系统设置
	GNL_ENDPOINT_PHONE_NUMBER1,		//端点-接警号码1
	GNL_ENDPOINT_PHONE_NUMBER2,		//端点-接警号码2
	GNL_ENDPOINT_PHONE_NUMBER3,		//端点-接警号码3
	GNL_ENDPOINT_PHONE_NUMBER4,		//端点-接警号码4
	GNL_ENDPOINT_PHONE_NUMBER5,		//端点-接警号码5
	GNL_ENDPOINT_PHONE_NUMBER6,		//端点-接警号码6
	GNL_ENDPOINT_GET_SENSOR_ID,		//端点-获取传感器ID
	GNL_ENDPOINT_SENSOR_ATT1,		//端点-获取传感器属性
	GNL_ENDPOINT_SENSOR_ATT2,		//端点-获取传感器属性	
	GNL_ENDPOINT_SENSOR_ATT3,		//端点-获取传感器属性
	GNL_ENDPOINT_SENSOR_ATT4,		//端点-获取传感器属性	
	GNL_ENDPOINT_SENSOR_ATT5,		//端点-获取传感器属性
	GNL_ENDPOINT_SENSOR_ATT6,		//端点-获取传感器属性	
	GNL_ENDPOINT_SENSOR_ATT7,		//端点-获取传感器属性
	GNL_ENDPOINT_SENSOR_ATT8,		//端点-获取传感器属性	
	GNL_ENDPOINT_SENSOR_ATT9,		//端点-获取传感器属性
	GNL_ENDPOINT_SENSOR_ATT10,		//端点-获取传感器属性	
	GNL_ENDPOINT_SENSOR_ATT11,		//端点-获取传感器属性
	GNL_ENDPOINT_SENSOR_ATT12,		//端点-获取传感器属性	
	GNL_ENDPOINT_SENSOR_ATT13,		//端点-获取传感器属性
	GNL_ENDPOINT_SENSOR_ATT14,		//端点-获取传感器属性	
	GNL_ENDPOINT_SENSOR_ATT15,		//端点-获取传感器属性
	GNL_ENDPOINT_SENSOR_ATT16,		//端点-获取传感器属性	
	GNL_ENDPOINT_SENSOR_ATT17,		//端点-获取传感器属性
	GNL_ENDPOINT_SENSOR_ATT18,		//端点-获取传感器属性	
	GNL_ENDPOINT_SENSOR_ATT19,		//端点-获取传感器属性
	GNL_ENDPOINT_SENSOR_ATT20,		//端点-获取传感器属性	
	GNL_ENDPOINT_SUM
}GENERAL_END_POINT_TYPEDEF;				//普通端点

typedef enum
{
	WNG_ENDPOINT_SYSTEM_MES_UP=100,   	//告警端点-报警
	WNG_ENDPOINT_SYSTEM_ARMDISARM_UP,   //告警端点-系统布撤防上报
}WARNING_END_POINT_TYPEDEF;		         //告警端点 

//网关子协议命令
typedef enum
{
	GLINK_M_GCMD_GET_ENDPOINT = 0x01,	//获取端点数据
	GLINK_M_GCMD_CHANGE_ENDPOINT = 0x02,//修改端点数据
	GLINK_M_GCMD_REPORT_TO_ENDPOINT = 0x03,	//上报端点数据	
}en_mtprotocol_endpoint_oper;


typedef enum
{
	ENDPOINT_TYPE_R,			//只读
	ENDPOINT_TYPE_WR,			//可读写
	ENDPOINT_TYPE_WARNING,		//告警
	ENDPOINT_TYPE_TROUBLE,		//故障
}END_POINT_TYPEDEF;		//端点类型定义



void mt_proto_init(void);
void mt_proto_PutInEvent(unsigned char event,unsigned char dat);
void mt_wifi_Mqtt_DatUpdataTask(unsigned char comType);
void mt_protocol_WIFIMqttRecHandle(unsigned char *pdata,unsigned char len);
void mt_protocol_4GMqttRecHandle(unsigned char *pdata,unsigned char len);
void mt_4G_Mqtt_DatUpdataTask(unsigned char comType);
void mt_protocol_GetFirmwarePack(unsigned char comType,unsigned short packNum,unsigned char *ver);
#endif
