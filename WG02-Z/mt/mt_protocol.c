#include "mt_protocol.h"
#include "mt_wifi.h"
#include "string.h"

Master_Server_Para Master_Server_Str;






// AA:     帧头
// 00 06： 数据长度
// 29：    命令
// 00：    数据帧ID
// 00 08： UTC
// 27：    校验 
// 55：    帧尾
/*******************************************************************************************
*@description:网关数据打包成串口协议数据
*@param[in]：comType: 通讯方式,*pData:通讯发送的数据
*@return：无
*@others：
********************************************************************************************/
static void mt_protocol_DataPack(unsigned char comType)
{
	unsigned char CheckXor = 0;  ///异或校验
    unsigned char DataBuff[200];
	unsigned short GWLen;

    memset(DataBuff, 0, sizeof(DataBuff));

	GWLen = Master_Server_Str.Struct_Len;
    DataBuff[0] = (GWLen>>8)&0xFF;;
    DataBuff[1] = GWLen&0xFF;
	
	DataBuff[2] = Master_Server_Str.Data_Head;		//帧头

	GWLen = Master_Server_Str.Data_Len;
	DataBuff[3] = (GWLen>>8)&0xFF;		
	DataBuff[4] = GWLen&0xFF;		//数据帧长度,包含校验值和帧尾

	//数据长度00 06 
	//CMD:29 自定义数据:00 00 08

	//校验值：数据长度+命今+自定义数据的异或值
	CheckXor = DataBuff[3];
	CheckXor ^= DataBuff[4];
    
    DataBuff[5] = Master_Server_Str.Data_Cmd;
    CheckXor ^= DataBuff[5];
    DataBuff[6] = Master_Server_Str.Data_SelfMake.Data_ID;
    CheckXor ^= DataBuff[6];

    GWLen = Master_Server_Str.Data_SelfMake.Data_UTC;
	DataBuff[7] = (GWLen>>8)&0xFF;		
	DataBuff[8] = GWLen&0xFF;		//数据帧长度,包含校验值和帧尾
    CheckXor ^= DataBuff[7];
    CheckXor ^= DataBuff[8];

    Master_Server_Str.Data_Check = CheckXor;
    DataBuff[9] = Master_Server_Str.Data_Check;
	DataBuff[10] = Master_Server_Str.Data_Tail;
    


    if(comType == 0)
	{//0009AA0006290000082755
		mt_wifi_Mqtt_SentDat(DataBuff);
	}
	else
	{
		//mt_4g_protocol_DataSet(DataBuff);
	}
}


/*******************************************************************************************
*@description:MCU请求获取服务器时间
*@param[in]：comType: WIFI_MQTT_EN表示WIFI 通讯,GSM_MQTT_EN表示4G
*@return：无
*@others：
        //AA0006290000082755
        //AT+MQTTPUB=0,"38FFFFFF3032533551310743_up","AA0006290000082755",2,0
********************************************************************************************/
void MCU_GetTime_Server(unsigned char comType)
{
    Master_Server_Str.Struct_Len = sizeof(Master_Server_Para) - 2;        //0x000B - 2
    Master_Server_Str.Data_Head = 0xAA;
    Master_Server_Str.Data_Len = Master_Server_Str.Struct_Len - 3;    //减去帧头和数据长度的sizeof
    Master_Server_Str.Data_Cmd = GLINK_M_CMD_GETDATE;
    Master_Server_Str.Data_SelfMake.Data_ID = 0x00;
    Master_Server_Str.Data_SelfMake.Data_UTC = 0x0008;
    Master_Server_Str.Data_Tail = 0x55;


    mt_protocol_DataPack(WIFI_MQTT_EN);

    
}
