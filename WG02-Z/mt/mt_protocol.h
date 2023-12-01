#ifndef ____MT_PROTOCOL_H_
#define ____MT_PROTOCOL_H_

#define SYSTEM_TIME_UTC   0x08 


//网关协议相关命令
//M 表示主机发送指令
//S 表示服务器下发指令
//R 表示主机回复指令
typedef enum
{
    GLINK_M_CMD_QUERY_UPDATE = 0x21,			//服务器通知MCU升级
	GLINK_S_CMD_INFORM_UPDATE = 0x22,			//服务器通知MCU升级

	GLINK_S_CMD_UPDATE_DATA = 0x24,			    //向服务器获取固件数据包
    GLINK_R_CMD_UPDATE_DATA = 0x25,		        //服务器回复固件数据包

	GLINK_M_CMD_GETDATE = 0x29,                 //MCU请求获取服务器时间指令
	GLINK_S_CMD_GETDATE	= 0x2A,                 //服务器回复时间指令		
	
	GLINK_S_CMD_GATEWAY	= 0x50,	                //网关协议命令
	GLINK_R_CMD_GATEWAY	= 0x51,                 //网关协议回复
	
}en_mtprotocol_cmd;

enum
{
    WIFI_MQTT_EN,
    GSM_MQTT_EN,
};

#pragma pack(1)       //取消结构体的默认对齐
struct Data_Custom
{
    unsigned char Data_ID;              //帧ID默认为00；
    unsigned short Data_UTC;            //时间区，默认东八区北京：00 08
};


typedef struct{
    unsigned short Struct_Len;          //sizeof()
	unsigned char Data_Head;            //帧头默认0xAA
	unsigned short Data_Len;            //数据长度：命令+自定义数据的长度+校验值+帧尾。
    unsigned char Data_Cmd;             //命令
    struct Data_Custom Data_SelfMake;   //自定义数据
    unsigned char Data_Check;           //校验值：数据长度+命今+自定义数据的三个的异或值
    unsigned char Data_Tail;            //帧尾默认0x55
}Master_Server_Para;

#pragma pack()        //恢复结构体的默认对齐


void MCU_GetTime_Server(unsigned char comType);
#endif
