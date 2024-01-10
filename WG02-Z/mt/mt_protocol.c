#include "hal_eeprom.h"
#include "mt_protocol.h"
#include "mt_wifi.h"
#include "mt_4G.h"
#include "mt_API.h"
#include "mt_update.h"
#include "mt_flash.h"
#include "os_system.h"
#include "para.h"
#include "app.h"

Queue32	 mqttUpEventMsg;

///端点号为100 
const unsigned char EndPointUpType_Message[][17] = 
{
	"detector alarm   ",     ///探测器报警 detector
	"detector offline ",     ///探测器报警 detector
	"detector online  ",     ///探测器报警 detector
	"detector volt low",     ///探测器电池低压
	"detector tamper  ",     ///探测器防拆报警
	"host SOS alarm   ",     ///主机SOS报警
	"host volt low    ",     ///主机电池低压
	"host AC remove   ",     ///主机AC 移除
	"host AC restore  ",     ///主机AC恢复	
	"door open        ",     ///门磁打开
	"door close       ",     ///门磁关闭	
};

const unsigned char SystemArmType_Message[][20]=
{
	" Arm by host        ", ///通过主机 操作布撤防
	" Arm by remote      ", ///通过遥控器 操作布撤防
	" Arm by APP         ", ///通过APP 操作布撤防
	" Homearm by host    ", ///通过主机 操作布撤防
	" Homearm by remote  ", ///通过遥控器 操作布撤防
	" Homearm by APP     ", ///通过APP 操作布撤防	
	" Disarm by host     ", ///通过主机 操作布撤防
	" Disarm by remote   ", ///通过遥控器 操作布撤防
	" Disarm by APP      ", ///通过APP 操作布撤防	
};





static void gLink_M_Cmd_RqstGetDate(unsigned char comType);
static void mt_protocol_RecDatParsing(unsigned char comType,unsigned char *pdata,unsigned char len);
static void mt_4g_protocol_DataSet(unsigned char *pdat);

void S_gatewayUploadEndpointProc(unsigned char comType,unsigned char id,unsigned char type,unsigned short ep);
static void mt_protocol_EndpointDataParsing_Proc(unsigned char comType,unsigned char *pData,unsigned char len);
static void gLink_M_Cmd_QueryFirmWare(unsigned char comType);
void mt_proto_init(void)
{
    QueueEmpty(mqttUpEventMsg);
}
///需要处理的系统时间
///dat     防区号
void mt_proto_PutInEvent(unsigned char event,unsigned char dat)
{
	QueueDataIn(mqttUpEventMsg,&event,1);
	QueueDataIn(mqttUpEventMsg,&dat,1);	
}

////.C 文件中编辑
void mt_wifi_Mqtt_DatUpdataTask(unsigned char comType)
{
	str_mqtt_Upevent event;
	if(QueueDataLen(mqttUpEventMsg) > 1)
	{
		if(mt_wifi_get_wifi_Mqtt_state() == STA_MQTT_READY)
		{
			QueueDataOut(mqttUpEventMsg,&event.event);
			QueueDataOut(mqttUpEventMsg,&event.buffer[0]);
			switch((unsigned char)event.event)
			{
				case ENDPOINT_UP_TYPE_GET_TIME:
					gLink_M_Cmd_RqstGetDate(comType);				
				break;
				case ENDPOINT_UP_QUERY_NEW_FIRMWARE:
				gLink_M_Cmd_QueryFirmWare(comType);
				break; 
				case ENDPOINT_UP_TYPE_DETECTOR_ALARM:///探测器报警 detector
					S_gatewayUploadEndpointProc(comType,event.buffer[0],ENDPOINT_UP_TYPE_DETECTOR_ALARM,WNG_ENDPOINT_SYSTEM_MES_UP);
				break;
				case ENDPOINT_UP_TYPE_DETECTOR_OFFLINE:///探测器离线detector
					S_gatewayUploadEndpointProc(comType,event.buffer[0],ENDPOINT_UP_TYPE_DETECTOR_OFFLINE,WNG_ENDPOINT_SYSTEM_MES_UP);
				break;				
				case ENDPOINT_UP_TYPE_DETECTOR_ONLINE:///探测器报警 detector
					S_gatewayUploadEndpointProc(comType,event.buffer[0],ENDPOINT_UP_TYPE_DETECTOR_ONLINE,WNG_ENDPOINT_SYSTEM_MES_UP);
				break;				
				
				case ENDPOINT_UP_TYPE_DETECTOR_BATLOW:///探测器电池低压
					S_gatewayUploadEndpointProc(comType,event.buffer[0],ENDPOINT_UP_TYPE_DETECTOR_BATLOW,WNG_ENDPOINT_SYSTEM_MES_UP);
				break;
				case ENDPOINT_UP_TYPE_DETECTOR_ALARM_TEMPER:///探测器防拆报警
					//S_gatewayUploadEndpointProc(comType,event.buffer[0],ENDPOINT_UP_TYPE_DETECTOR_BATLOW,WNG_ENDPOINT_SYSTEM_MES_UP);
				break;
				case ENDPOINT_UP_TYPE_HOST_ALARM_SOS:///主机SOS报警
					S_gatewayUploadEndpointProc(comType,0xff,ENDPOINT_UP_TYPE_HOST_ALARM_SOS,WNG_ENDPOINT_SYSTEM_MES_UP);
				break;
				case ENDPOINT_UP_TYPE_HOST_BATLOW:///主机电池低压
					S_gatewayUploadEndpointProc(comType,0xff,ENDPOINT_UP_TYPE_HOST_BATLOW,WNG_ENDPOINT_SYSTEM_MES_UP);
				break;
				case ENDPOINT_UP_TYPE_HOST_ACREMOVE:///主机AC移除
					S_gatewayUploadEndpointProc(comType,0xff,ENDPOINT_UP_TYPE_HOST_ACREMOVE,WNG_ENDPOINT_SYSTEM_MES_UP);
				break;
				
				case ENDPOINT_UP_TYPE_HOST_ACRESTORE:///主机AC恢复
					S_gatewayUploadEndpointProc(comType,0xff,ENDPOINT_UP_TYPE_HOST_ACRESTORE,WNG_ENDPOINT_SYSTEM_MES_UP);
				break;
				case ENDPOINT_UP_TYPE_DOOR_OPEN:///探测器门磁打开
					S_gatewayUploadEndpointProc(comType,event.buffer[0],ENDPOINT_UP_TYPE_DOOR_OPEN,WNG_ENDPOINT_SYSTEM_MES_UP);
				break;
				case ENDPOINT_UP_TYPE_DOOR_CLOSE:///探测器门磁关闭	
					  S_gatewayUploadEndpointProc(comType,event.buffer[0],ENDPOINT_UP_TYPE_DOOR_CLOSE,WNG_ENDPOINT_SYSTEM_MES_UP);
				break;

				case ENDPOINT_UP_ARM_TYPE_ARM_BY_HOST:///通过主机 操作布撤防
					S_gatewayUploadEndpointProc(comType,0xff,ENDPOINT_UP_ARM_TYPE_ARM_BY_HOST,WNG_ENDPOINT_SYSTEM_ARMDISARM_UP);
				break;
				case ENDPOINT_UP_ARM_TYPE_ARM_BY_REMOTE:///通过遥控器 操作布撤防
					S_gatewayUploadEndpointProc(comType,0xff,ENDPOINT_UP_ARM_TYPE_ARM_BY_REMOTE,WNG_ENDPOINT_SYSTEM_ARMDISARM_UP);
				break;
				case ENDPOINT_UP_ARM_TYPE_ARM_BY_APP:///通过APP 操作布撤防
					S_gatewayUploadEndpointProc(comType,0xff,ENDPOINT_UP_ARM_TYPE_ARM_BY_APP,WNG_ENDPOINT_SYSTEM_ARMDISARM_UP);
				break;
				case ENDPOINT_UP_ARM_TYPE_HOMEARM_BY_HOST:///通过主机 操作布撤防
					S_gatewayUploadEndpointProc(comType,0xff,ENDPOINT_UP_ARM_TYPE_HOMEARM_BY_HOST,WNG_ENDPOINT_SYSTEM_ARMDISARM_UP);	
				break;
				case ENDPOINT_UP_ARM_TYPE_HOMEARM_BY_REMOTE:///通过遥控器 操作布撤防
					S_gatewayUploadEndpointProc(comType,0xff,ENDPOINT_UP_ARM_TYPE_HOMEARM_BY_REMOTE,WNG_ENDPOINT_SYSTEM_ARMDISARM_UP);	
				break;
				case ENDPOINT_UP_ARM_TYPE_HOMEARM_BY_APP:///通过APP 操作布撤防	
					S_gatewayUploadEndpointProc(comType,0xff,ENDPOINT_UP_ARM_TYPE_HOMEARM_BY_APP,WNG_ENDPOINT_SYSTEM_ARMDISARM_UP);	
				break;
				case ENDPOINT_UP_ARM_TYPE_DISARM_BY_HOST:///通过主机 操作布撤防
					S_gatewayUploadEndpointProc(comType,0xff,ENDPOINT_UP_ARM_TYPE_DISARM_BY_HOST,WNG_ENDPOINT_SYSTEM_ARMDISARM_UP);	  
				break;
				case ENDPOINT_UP_ARM_TYPE_DISARM_BY_REMOTE:///通过遥控器 操作布撤防
					S_gatewayUploadEndpointProc(comType,0xff,ENDPOINT_UP_ARM_TYPE_DISARM_BY_REMOTE,WNG_ENDPOINT_SYSTEM_ARMDISARM_UP);	
				break;
				case ENDPOINT_UP_ARM_TYPE_DISARM_BY_APP:///通过APP 操作布撤防	
					S_gatewayUploadEndpointProc(comType,0xff,ENDPOINT_UP_ARM_TYPE_DISARM_BY_APP,WNG_ENDPOINT_SYSTEM_ARMDISARM_UP);	
				break;	
				
			}		
		}
	}
}

//网关数据打包成串口协议数据
///unsigned char comType  通讯的方式   0表示WIFI 通讯   1表示4G
///unsigned char *pData   通讯发送的数据
//AA0006290000082755
// AA:     帧头
// 00 06： 数据长度
// 29：    命令
// 00：    数据帧ID
// 00 08： UTC
// 27：    校验 
// 55：    帧尾
static void mt_protocol_DataPack(unsigned char comType,unsigned char *pData)
{
	unsigned char CheckXor;  ///异或校验
	unsigned char *pGWData;
  unsigned char DataBuff[200];
	unsigned short i,GWLen;
	pGWData = pData;
	GWLen = pGWData[0];
	GWLen <<= 8;
	GWLen |= pGWData[1];
	
	CheckXor = 0;
	DataBuff[2] = 0xAA;		//帧头
	
	DataBuff[3] = (GWLen>>8)&0xFF;		
	DataBuff[4] = GWLen&0xFF;;		//数据帧长度,包含校验值和帧尾

	GWLen -= 2;				//减去[0],[1]数据长度2个字节
	CheckXor = DataBuff[3];
	CheckXor ^= DataBuff[4];
	for(i=0; i<GWLen; i++)
	{
		//[0],[1]打包协议后的帧长度,[2]-0xAA，所以从下标从3开始
		DataBuff[i+5] = pGWData[i+2];
		CheckXor ^= pGWData[i+2];
	}
	GWLen = GWLen+5;
	
	DataBuff[GWLen++] = CheckXor;
	DataBuff[GWLen++] = 0x55;
	
	GWLen -= 2;
	DataBuff[0] = (GWLen>>8)&0xFF;
	DataBuff[1] = GWLen&0xFF;	
   if(comType == 0)
	{//0009AA0006290000082755
	   mt_wifi_Mqtt_SentDat(DataBuff);
	}
	else
	{
	   mt_4g_protocol_DataSet(DataBuff);
	}
}

//AA0006290000082755
//AT+MQTTPUB=0,"38FFFFFF3032533551310743_up","AA0006290000082755",2,0
//MCU请求获取服务器时间,utc-时区  
//unsigned char comType  通讯的方式   0表示WIFI 通讯   1表示4G
static void gLink_M_Cmd_RqstGetDate(unsigned char comType)
{
	unsigned char DataBuff[128];
	unsigned short idx;
	idx = 2;
	//AA 00 06 
	//29 00 03 20 0C 55     +800北京时间
	DataBuff[idx++] = GLINK_M_CMD_GETDATE;	//命令  0X29
	DataBuff[idx++] = 0;					//数据帧ID
	DataBuff[idx++] = (SYSTEM_TIME_UTC>>8)&0xFF;					//数据帧ID
	DataBuff[idx++] = SYSTEM_TIME_UTC&0xFF;						//UTC高字节
	DataBuff[0] = (idx>>8)&0xFF;	 //idx = 6;
	DataBuff[1] = idx&0xFF;	
	mt_protocol_DataPack(comType,DataBuff);
}//



// AA000C2A0007E6081803121A04FE55
// AA:     帧头
// 00 0C： 数据长度
// 2A：    命令
// 00：    数据帧ID
// 07 E6： 年
// 08：      月
// 18：      日
// 03:        周
// 12:        时
// 1A:       分
// 04:       秒
// FE:      校验
// 55:      帧尾
//  comType 处理方式    0WIFI    1GSM
//extern void testss1(void);


static void mt_protocol_RecDatParsing(unsigned char comType,unsigned char *pdata,unsigned char len)
{
	unsigned char *bdata;
	unsigned short lenx;
	unsigned short versionDat;
	str_FirmwareUpdate update;
	bdata = pdata;
	if(bdata[0]  == 0xAA)
	{  
			if(len > 3)
			{
					lenx = bdata[1] << 8;
					lenx += bdata[2];			
			}
			else
			{
				return;
			}  
			if(bdata[lenx + 2] == 0x55)
			{
				 switch(bdata[3])
				 {
					case GLINK_S_CMD_GETDATE:
					{//  GLINK_S_CMD_GETDATE
						if(lenx == 0x0C)
						{//AA00122A0007E60211040B0D47B755
							MT_SET_DATE_YEAR((bdata[5]<<8) + bdata[6]);
							MT_SET_DATE_MON(bdata[7]);
							MT_SET_DATE_DAY(bdata[8]);
							MT_SET_DATE_WEEK(bdata[9]);
							MT_SET_DATE_HOUR(bdata[10]);
							MT_SET_DATE_MIN(bdata[11]);
							MT_SET_DATE_SEC(bdata[12]);					
						}
					}
					break;
					case GLINK_S_CMD_INFORM_UPDATE:
					{
						
						if(bdata[5] == 0x00)
						{// MCU固件信息      
							update.upVer.version[0] = bdata[6];   // 服务器固件的版本号
							update.upVer.version[1] = bdata[7];
							versionDat = (update.upVer.version[0] << 8) + update.upVer.version[1];
							if(versionDat > ((MT_GET_PARA_SYS_FIRMWARE(0)<<8) + MT_GET_PARA_SYS_FIRMWARE(1)))
							{////服务器下发的固件版本号 比主机当前的版本号 要高
								update.fireSize.bytesize[3] = bdata[8];
								update.fireSize.bytesize[2] = bdata[9];
								update.fireSize.bytesize[1] = bdata[10];
								update.fireSize.bytesize[0] = bdata[11];
								update.firePack.packSize[1] = bdata[12];  
								update.firePack.packSize[0] = bdata[13];
								update.crc16[0] = bdata[14];
								update.crc16[1] = bdata[15];								
								mt_update_Fireware(update);										
							}
						}
					}
					break;					

					case GLINK_R_CMD_UPDATE_DATA:
					{
						mt_update_upingPro(&mt_flash_SaveDat,&bdata[4],&mt_flashRead);	
					}    
					break;		
	 
					case GLINK_S_CMD_GATEWAY:
					{///数据透传   对端点操作

						mt_protocol_EndpointDataParsing_Proc(comType,&bdata[1],lenx);	
	
					}    
					break;							
				 }
			}
	}
	//	bdata ++;
}
//  comType 处理方式    0 WIFI    1GSM
//服务器接收数据处理函数   WIFI
void mt_protocol_WIFIMqttRecHandle(unsigned char *pdata,unsigned char len)
{
	mt_protocol_RecDatParsing(0,pdata,len);
}

//服务器接收数据处理函数    4G
void mt_protocol_4GMqttRecHandle(unsigned char *pdata,unsigned char len)
{///WIFI 和4G    WIFI 
	if(mt_wifi_get_wifi_Mqtt_state() != STA_MQTT_READY)
	{////如果WIFI 的MQTT通讯不正常
		mt_protocol_RecDatParsing(1,pdata,len);
	}
}
////

void mt_4G_Mqtt_DatUpdataTask(unsigned char comType)
{
	str_mqtt_Upevent event;
	//static unsigned char mqtt_event_sentdatbuffer[200];
	if(mt_wifi_get_wifi_Mqtt_state() != STA_MQTT_READY)
	{////如果WIFI 的MQTT通讯不正常
		if(QueueDataLen(mqttUpEventMsg) > 1)
		{
			if(mt_getgsmMqttState() == GSM_MQTT_READY)
			{	
					QueueDataOut(mqttUpEventMsg,&event.event);
					QueueDataOut(mqttUpEventMsg,&event.buffer[0]);
					switch((unsigned char)event.event)
					{
						case ENDPOINT_UP_TYPE_GET_TIME:
							gLink_M_Cmd_RqstGetDate(comType);		
						break;
					    case ENDPOINT_UP_QUERY_NEW_FIRMWARE:
							gLink_M_Cmd_QueryFirmWare(comType);
				        break; 
						case ENDPOINT_UP_TYPE_DETECTOR_ALARM:///探测器报警 detector
							S_gatewayUploadEndpointProc(comType,event.buffer[0],ENDPOINT_UP_TYPE_DETECTOR_ALARM,WNG_ENDPOINT_SYSTEM_MES_UP);
						break;
						case ENDPOINT_UP_TYPE_DETECTOR_OFFLINE:///探测器离线detector
							S_gatewayUploadEndpointProc(comType,event.buffer[0],ENDPOINT_UP_TYPE_DETECTOR_OFFLINE,WNG_ENDPOINT_SYSTEM_MES_UP);
						break;				
						case ENDPOINT_UP_TYPE_DETECTOR_ONLINE:///探测器报警 detector
							S_gatewayUploadEndpointProc(comType,event.buffer[0],ENDPOINT_UP_TYPE_DETECTOR_ONLINE,WNG_ENDPOINT_SYSTEM_MES_UP);
						break;				
						
						case ENDPOINT_UP_TYPE_DETECTOR_BATLOW:///探测器电池低压
							S_gatewayUploadEndpointProc(comType,event.buffer[0],ENDPOINT_UP_TYPE_DETECTOR_BATLOW,WNG_ENDPOINT_SYSTEM_MES_UP);
						break;
						case ENDPOINT_UP_TYPE_DETECTOR_ALARM_TEMPER:///探测器防拆报警
							//S_gatewayUploadEndpointProc(comType,event.buffer[0],ENDPOINT_UP_TYPE_DETECTOR_BATLOW,WNG_ENDPOINT_SYSTEM_MES_UP);
						break;
						case ENDPOINT_UP_TYPE_HOST_ALARM_SOS:///主机SOS报警
							S_gatewayUploadEndpointProc(comType,0xff,ENDPOINT_UP_TYPE_HOST_ALARM_SOS,WNG_ENDPOINT_SYSTEM_MES_UP);
						break;
						case ENDPOINT_UP_TYPE_HOST_BATLOW:///主机电池低压
							S_gatewayUploadEndpointProc(comType,0xff,ENDPOINT_UP_TYPE_HOST_BATLOW,WNG_ENDPOINT_SYSTEM_MES_UP);
						break;
						case ENDPOINT_UP_TYPE_HOST_ACREMOVE:///主机AC移除
							S_gatewayUploadEndpointProc(comType,0xff,ENDPOINT_UP_TYPE_HOST_ACREMOVE,WNG_ENDPOINT_SYSTEM_MES_UP);
						break;
						
						case ENDPOINT_UP_TYPE_HOST_ACRESTORE:///主机AC恢复
							S_gatewayUploadEndpointProc(comType,0xff,ENDPOINT_UP_TYPE_HOST_ACRESTORE,WNG_ENDPOINT_SYSTEM_MES_UP);
						break;
						case ENDPOINT_UP_TYPE_DOOR_OPEN:///探测器门磁打开
							S_gatewayUploadEndpointProc(comType,event.buffer[0],ENDPOINT_UP_TYPE_DOOR_OPEN,WNG_ENDPOINT_SYSTEM_MES_UP);
						break;
						case ENDPOINT_UP_TYPE_DOOR_CLOSE:///探测器门磁关闭	
							  S_gatewayUploadEndpointProc(comType,event.buffer[0],ENDPOINT_UP_TYPE_DOOR_CLOSE,WNG_ENDPOINT_SYSTEM_MES_UP);
						break;
						
						

						case ENDPOINT_UP_ARM_TYPE_ARM_BY_HOST:///通过主机 操作布撤防
							S_gatewayUploadEndpointProc(comType,0xff,ENDPOINT_UP_ARM_TYPE_ARM_BY_HOST,WNG_ENDPOINT_SYSTEM_ARMDISARM_UP);
						break;
						case ENDPOINT_UP_ARM_TYPE_ARM_BY_REMOTE:///通过遥控器 操作布撤防
							S_gatewayUploadEndpointProc(comType,0xff,ENDPOINT_UP_ARM_TYPE_ARM_BY_REMOTE,WNG_ENDPOINT_SYSTEM_ARMDISARM_UP);
						break;
						case ENDPOINT_UP_ARM_TYPE_ARM_BY_APP:///通过APP 操作布撤防
							S_gatewayUploadEndpointProc(comType,0xff,ENDPOINT_UP_ARM_TYPE_ARM_BY_APP,WNG_ENDPOINT_SYSTEM_ARMDISARM_UP);
						break;
						case ENDPOINT_UP_ARM_TYPE_HOMEARM_BY_HOST:///通过主机 操作布撤防
							S_gatewayUploadEndpointProc(comType,0xff,ENDPOINT_UP_ARM_TYPE_HOMEARM_BY_HOST,WNG_ENDPOINT_SYSTEM_ARMDISARM_UP);	
						break;
						case ENDPOINT_UP_ARM_TYPE_HOMEARM_BY_REMOTE:///通过遥控器 操作布撤防
							S_gatewayUploadEndpointProc(comType,0xff,ENDPOINT_UP_ARM_TYPE_HOMEARM_BY_REMOTE,WNG_ENDPOINT_SYSTEM_ARMDISARM_UP);	
						break;
						case ENDPOINT_UP_ARM_TYPE_HOMEARM_BY_APP:///通过APP 操作布撤防	
							S_gatewayUploadEndpointProc(comType,0xff,ENDPOINT_UP_ARM_TYPE_HOMEARM_BY_APP,WNG_ENDPOINT_SYSTEM_ARMDISARM_UP);	
						break;
						case ENDPOINT_UP_ARM_TYPE_DISARM_BY_HOST:///通过主机 操作布撤防
							S_gatewayUploadEndpointProc(comType,0xff,ENDPOINT_UP_ARM_TYPE_DISARM_BY_HOST,WNG_ENDPOINT_SYSTEM_ARMDISARM_UP);	  
						break;
						case ENDPOINT_UP_ARM_TYPE_DISARM_BY_REMOTE:///通过遥控器 操作布撤防
							S_gatewayUploadEndpointProc(comType,0xff,ENDPOINT_UP_ARM_TYPE_DISARM_BY_REMOTE,WNG_ENDPOINT_SYSTEM_ARMDISARM_UP);	
						break;
						case ENDPOINT_UP_ARM_TYPE_DISARM_BY_APP:///通过APP 操作布撤防	
							S_gatewayUploadEndpointProc(comType,0xff,ENDPOINT_UP_ARM_TYPE_DISARM_BY_APP,WNG_ENDPOINT_SYSTEM_ARMDISARM_UP);	
						break;
					}
			}
		}	
	}
}


/////////////////////
static void mt_4g_protocol_DataSet(unsigned char *pdat)
{
	str_GsmState sta;
	unsigned short lon;
	unsigned char idx,pid,hchar,lchar;
    unsigned char *pmdat;
    pmdat = pdat;
	
	sta.State = GSM_MQTT_PUB;
	lon = pmdat[0];
	lon += pmdat[1];				
	idx = 2;
	pid = 2;
	while(lon--)
	{
		hexToAsciiConversion(pmdat[idx],&hchar,&lchar);
		sta.SentBuffer[pid ++] = hchar;	
		sta.SentBuffer[pid ++] = lchar;
		idx++;
		if(idx >= (WIFI_MQTT_SENTDATASIZE_MAX-6))
			break;
	}		
	sta.SentBuffer[0] = 0;
	sta.SentBuffer[1] = (pid-2);
	mt_setgsmMqttState(sta);					
}

//unsigned char comType     通讯的方式  0-WIFI  1-4G
//unsigned char id          防区号   FF 代表是主机或APP   1-20 代表的是探测器
//unsigned char type        类型     告警信息
//unsigned short ep         端点号   

//端点数据上报的  数据的打包
void S_gatewayUploadEndpointProc(unsigned char comType,unsigned char id,unsigned char type,unsigned short ep)
{
	unsigned char tData[100],i,j;
	unsigned char sId;
	unsigned short idx;
	sId = id;
	i = 2; 
	tData[i++] = GLINK_S_CMD_GATEWAY;			//网关协议回复指令	
	tData[i++] = 0;								//数据帧ID
	tData[i++] = GLINK_M_GCMD_REPORT_TO_ENDPOINT;		//获取端点数据指令
	tData[i++] = (ep>>8)&0xFF;					//端点索引号高字节
	tData[i++] = ep&0xFF;						//端点索引号低字节
	tData[i++] = 2;										//端点类型:告警
	tData[i++] = 10;									//数据类型:数组
	tData[i++] = 0;									//[9]端点数据长度高字节
	tData[i++] = 0;									//[10]端点数据长度低字节
	idx = 0;		//记录端点数据长度
	tData[i++] = MT_GET_DATE_MON();      ///月
	tData[i++] = '-';	            
	tData[i++] = MT_GET_DATE_DAY();      ///日
	tData[i++] = ' ';		
	tData[i++] = MT_GET_DATE_HOUR();     ///时
	tData[i++] = ':';													//:
	tData[i++] = MT_GET_DATE_MIN();      ///分
	tData[i++] = ' ';				     //空格
    tData[i++] = sId;	
	idx = 9;	
    if(ep == WNG_ENDPOINT_SYSTEM_MES_UP)
	{///系统数据上报 端点 100
		///探测器名称 
		if(sId == 0xff)
		{///报警主机
			tData[i++] = 'H';      /// 
			tData[i++] = 'O';      /// 
			tData[i++] = 'S';      /// 
			tData[i++] = 'T';      /// 		
			for(j=0; j<12; j++)
			{
				tData[i++] = ' ';			//传感器名称
			}
		}
		else  
		{
			//传感器告警事件
			sId -= 1;		//传感器实际存储索引等于ID-1	
			for(j=0; j<16; j++)
			{
				tData[i++] = MT_GET_PARA_DEVICE_NAME_BYTE(sId,j);			//传感器名称
			}
		}
		idx += 16;
 
		if((type >= ENDPOINT_UP_TYPE_DETECTOR_ALARM) && (type <= ENDPOINT_UP_TYPE_DOOR_CLOSE))
		{
			for(j=0; j<17; j++)
			{
				tData[i++] = EndPointUpType_Message[type - ENDPOINT_UP_TYPE_DETECTOR_ALARM][j]; 	//传感器名称
			}	
			idx += 17;				
		}
	}
	else if(ep == WNG_ENDPOINT_SYSTEM_ARMDISARM_UP)
	{//101  
		if((type >= ENDPOINT_UP_ARM_TYPE_ARM_BY_HOST) && (type <= ENDPOINT_UP_ARM_TYPE_DISARM_BY_APP))
		{
			for(j=0; j<20; j++)
			{///布撤防操作的方式  遥控器/主机键盘/APP
				tData[i++] = SystemArmType_Message[type - ENDPOINT_UP_ARM_TYPE_ARM_BY_HOST][j]; 	//传感器名称
			}
			idx += 20;
		}
	}
	//端点数据结束
	tData[9] = (idx>>8)&0xFF;					//端点数据长度高字节
	tData[10] = idx&0xFF;	

	tData[0] = (i>>8)&0xFF;	 
	tData[1] = i&0xFF;
	
	mt_protocol_DataPack(comType,tData);	
	//gLinkDataPack(tData);
}


//app.h 中添加
//extern stu_system_mode *pStuSystemMode;
//#define GET_SYSTEM_WORK_MODE  pStuSystemMode->ID
//0001
//0001000A0A55
static void mt_protocol_gatewayGetEndPointPro(unsigned char comType,unsigned char *pDat,unsigned char len)
{
	unsigned char *pData;
	unsigned char tData[100],i,j,numx;
	unsigned short endPointNum,id;
	pData = pDat;
	endPointNum = pData[2];
	endPointNum <<= 8;
	endPointNum |= pData[3];					//读取端点号
	
	i = 2; 
	tData[i++] = GLINK_R_CMD_GATEWAY;			//网关协议回复指令	
	tData[i++] = pData[0];						//数据帧ID
	tData[i++] = GLINK_M_GCMD_GET_ENDPOINT;		//获取端点数据
	tData[i++] = 0;								     //操作结果
	tData[i++] = pData[2];							//端点索引号高字节
	tData[i++] = pData[3];						//端点索引号低字节

	switch(endPointNum)
	{
		case GNL_ENDPOINT_DEVICE_INFO:
				tData[i++] = ENDPOINT_TYPE_R;					//端点类型-只读
				tData[i++] = 10;  //0X0A						//数据类型-数组
				tData[i++] = 0;									//数据长度高字节
				tData[i++] = 25;									//数据长度低字节
				tData[i++] = (TYPE_PRODUCT_UNMBER>>8)&0xFF;;	//产品类型高字节
				tData[i++] = TYPE_PRODUCT_UNMBER&0xFF;			//产品类型低字节
				
				tData[i++] = (GatewayProtocolVer>>8)&0xFF;							 //协议版本高字节
				tData[i++] = GatewayProtocolVer&0xFF;								 //协议版本低字节
				tData[i++] = 0;						                                 //预留
				tData[i++] = 0;							                             //预留
				//tData[i++] = (MCU_HARDWARE_VERSION>>8)&0xFF;						 //MCU固件版本高字节
				//tData[i++] = MCU_HARDWARE_VERSION&0xFF;								 //MCU固件版本低字节
				
				tData[i++] = MT_GET_PARA_SYS_FIRMWARE(0);						 //MCU固件版本高字节
				tData[i++] = MT_GET_PARA_SYS_FIRMWARE(1);								 //MCU固件版本低字节
						
		
		
				tData[i++] = (MCU_HARDVER_VERSION>>8)&0xFF;					   //硬件版本号高字节
				tData[i++] = MCU_HARDVER_VERSION&0xFF;								 //硬件版本号低字节

				tData[i++] = 0x00;// 							 //MAC  ID
				tData[i++] = 0x00;// 							 //MAC  ID		
				tData[i++] = 0x00;// 							 //MAC  ID
				tData[i++] = MT_GET_MCU_UID(0);// 							 //MAC  ID
				tData[i++] = MT_GET_MCU_UID(1);// 							 //MAC  ID
				tData[i++] = MT_GET_MCU_UID(2);// 							 //MAC  ID
				tData[i++] = MT_GET_MCU_UID(3);// 							 //MAC  ID		
				tData[i++] = MT_GET_MCU_UID(4);// 							 //MAC  ID
				tData[i++] = MT_GET_MCU_UID(5);// 							 //MAC  ID
				tData[i++] = MT_GET_MCU_UID(6);// 							 //MAC  ID		
				tData[i++] = MT_GET_MCU_UID(7);// 							 //MAC  ID
				tData[i++] = MT_GET_MCU_UID(8);// 							 //MAC  ID		
				tData[i++] = MT_GET_MCU_UID(9);// 							 //MAC  ID
				tData[i++] = MT_GET_MCU_UID(10);// 							 //MAC  ID
				tData[i++] = MT_GET_MCU_UID(11);// 							 //MAC  ID	
		break;
		case GNL_ENDPOINT_SYSTEM_MODE:
				tData[i++] = ENDPOINT_TYPE_WR;					//端点类型-只读
				tData[i++] = 10;								//数据类型-数组
				tData[i++] = 0;									//数据长度高字节
				tData[i++] = 1;									//数据长度低字节
			    tData[i++] = GET_SYSTEM_WORK_MODE;
		break;
		case GNL_ENDPOINT_PHONE_NUMBER1:
		case GNL_ENDPOINT_PHONE_NUMBER2:
		case GNL_ENDPOINT_PHONE_NUMBER3:
		case GNL_ENDPOINT_PHONE_NUMBER4:
		case GNL_ENDPOINT_PHONE_NUMBER5:
		case GNL_ENDPOINT_PHONE_NUMBER6:				
			id = endPointNum-GNL_ENDPOINT_PHONE_NUMBER1;	//电话号码编号
			tData[i++] = ENDPOINT_TYPE_WR;					//端点类型-只读
			tData[i++] = 10;								//数据类型-数组
			tData[i++] = 0;									//数据长度高字节
		    numx = i;
			tData[i++] = 20;									//数据长度低字节
			if(id < 6)
			{
				for(j=0;j<20;j++)
				{
					if(MT_GET_PARA_SYS_PHONENUMBER(id,j) < 10)
					{
						tData[i++] = 	MT_GET_PARA_SYS_PHONENUMBER(id,j);
					}
					else
					{
						tData[numx] = j;
						break;
					}		
				}					
			}
		break;
		case GNL_ENDPOINT_GET_SENSOR_ID:
		{
			tData[i++] = ENDPOINT_TYPE_R;					//端点类型-只读
			tData[i++] = 10;								//数据类型-数组
			tData[i++] = 0;									//数据长度高字节
		    numx = i;
			tData[i++] = 20;									//数据长度低字节
			id = 0;
			for(j=0;j<PARA_DTC_SUM;j++)
			{
				if(MT_GET_PARA_DEVICE_MARK(j) == 1)
				{
					tData[i++] = 	0x00;
					tData[i++] = 	MT_GET_PARA_DEVICE_ID(j);
					id += 2;
				}
			}
            tData[numx] = id;				
		}
		break;
		case GNL_ENDPOINT_SENSOR_ATT1:
		case GNL_ENDPOINT_SENSOR_ATT2:			//??-???????	
		case GNL_ENDPOINT_SENSOR_ATT3:			//??-???????
		case GNL_ENDPOINT_SENSOR_ATT4:			//??-???????		
		case GNL_ENDPOINT_SENSOR_ATT5:			//??-???????
		case GNL_ENDPOINT_SENSOR_ATT6:			//??-???????	
		case GNL_ENDPOINT_SENSOR_ATT7:			//??-???????
		case GNL_ENDPOINT_SENSOR_ATT8:			//??-???????			
		case GNL_ENDPOINT_SENSOR_ATT9:			//??-???????
		case GNL_ENDPOINT_SENSOR_ATT10:		//??-???????	
		case GNL_ENDPOINT_SENSOR_ATT11:			//??-???????
		case GNL_ENDPOINT_SENSOR_ATT12:			//??-???????			
		case GNL_ENDPOINT_SENSOR_ATT13:		//??-???????
		case GNL_ENDPOINT_SENSOR_ATT14:			//??-???????		
		case GNL_ENDPOINT_SENSOR_ATT15:			//??-???????
		case GNL_ENDPOINT_SENSOR_ATT16:			//??-???????	
		case GNL_ENDPOINT_SENSOR_ATT17:			//??-???????
		case GNL_ENDPOINT_SENSOR_ATT18:			//??-???????			
		case GNL_ENDPOINT_SENSOR_ATT19:			//??-???????
		case GNL_ENDPOINT_SENSOR_ATT20:			//??-???????				
		{
			id = endPointNum-GNL_ENDPOINT_SENSOR_ATT1;	//电话号码编号
			tData[i++] = ENDPOINT_TYPE_WR;	//端点类型-
			tData[i++] = 10;								//数据类型-数组
			tData[i++] = 0;									//数据长度高字节
		  numx = i;
			tData[i++] = 22;									//数据长度低字节

			//传感器ID
			tData[i++] = 	0x00;    //传感器ID 高字节
			tData[i++] = 	MT_GET_PARA_DEVICE_ID(id);    ////传感器ID 低字节
			tData[i++] = 	MT_GET_PARA_DEVICE_MARK(id);  ////探测器学习标志位
			
			//传感器类型
			tData[i++] = 	0x00;    //传感器类型高字节
			tData[i++] = 	MT_GET_PARA_DEVICE_ALARMTYPE(id);  ////传感器类型 低字节
			//传感器类型			
			for(j=0;j<16;j++)
			{///探测器名称
				tData[i++] = MT_GET_PARA_DEVICE_NAME_BYTE(id,j);
			}			
			tData[i++] = MT_GET_PARA_DEVICE_ALARMTYPE(id);  //防区防线
			tData[i++] = 0;  //预留	
		}
		break;	
		default:
			id = endPointNum-GNL_ENDPOINT_SENSOR_ATT1;	//电话号码编号
			tData[i++] = ENDPOINT_TYPE_WR;	//端点类型-
			tData[i++] = 10;								//数据类型-数组
			tData[i++] = 0;									//数据长度高字节
			tData[i++] = 0;									//数据长度低字节		
		break;
	}	 
	tData[0] = (i>>8)&0xFF;	 
	tData[1] = i&0xFF;
	mt_protocol_DataPack(comType,tData);	
}

//AA000C50
//00 数据帧ID
//02 更改端点数据
//0002  端点号
//01  端点类型
//0A   数据类型
//0001  有效数据长度
//00  有效数据
//0A  校验
//55  帧尾

static void mt_protocol_gatewayChangeEndPointPro(unsigned char comType,unsigned char *pDat,unsigned char len)
{
//	Stu_Device DevPara;
	unsigned char *pData;
	unsigned char tData[250],i,id,j,errFlag,changeFlag,temp;//,code[3],i;
	unsigned short endPointNum,temp16;
	pData = pDat;
	changeFlag = 0;
    errFlag = 0;
	endPointNum = (pData[2]<<8)&0xFF00;
	endPointNum |= pData[3];						//读取端点号
	i = 2; 
	tData[i++] = GLINK_R_CMD_GATEWAY;			//网关协议回复指令	
	tData[i++] = pData[0];							//数据帧ID
	tData[i++] = GLINK_M_GCMD_CHANGE_ENDPOINT;		//修改端点数据
	tData[i++] = 0;									//[5] 操作结果 0成功 1失败
	tData[i++] = pData[2];							//端点索引号高字节
	tData[i++] = pData[3];							//端点索引号低字节
	tData[i++] = ENDPOINT_TYPE_WR;					//端点类型,可读写
	tData[i++] = 10;								    //数据类型,10-数组
	switch(endPointNum)
	{
		case GNL_ENDPOINT_DEVICE_INFO:
		break;
		case GNL_ENDPOINT_SYSTEM_MODE:
		{
			////端点-系统设置
			tData[i++] = 0;									//回复数据长度高字节
			tData[i++] = 1;								//回复数据长度低字节  
			tData[i++] = pData[8];								//回复数据长度低字节  
			if(pData[8] != 0xFF)			//判断是否修改主机防线
			{
				temp = pData[8];
				if(temp<3)
				{
					changeFlag = 0;			//这里设置0,主机防线改变不需要掉电存储
					if(temp==0)
					{//离家布防
						SystemMode_Change(0xff,SYSTEM_MODE_ENARM_HOST,SYSTEM_MODE_OPER_APP); 
					}
					else if(temp==1)
					{//在家布防
						SystemMode_Change(0xff,SYSTEM_MODE_HOMEARM_HOST,SYSTEM_MODE_OPER_APP); 
					}
					else if(temp==2)
					{//系统撤防
						SystemMode_Change(0xff,SYSTEM_MODE_DISARM_HOST,SYSTEM_MODE_OPER_APP); 
					}
				}
				else
				{
					errFlag=1;			//参数错误
				}
			}
		}
		break;
		case GNL_ENDPOINT_PHONE_NUMBER1:
		case GNL_ENDPOINT_PHONE_NUMBER2:
		case GNL_ENDPOINT_PHONE_NUMBER3:
		case GNL_ENDPOINT_PHONE_NUMBER4:
		case GNL_ENDPOINT_PHONE_NUMBER5:
		case GNL_ENDPOINT_PHONE_NUMBER6:
		{
			temp16 = (pData[6]<<8)&0xFF00;
			temp16 |= pData[7];
			if(temp16 < 20)		//判断数据长度是否正确
			{
				id = endPointNum-GNL_ENDPOINT_PHONE_NUMBER1;	//电话号码编号
				if(id < 6) 
				{ 
					tData[i++] = 0;									//回复数据长度高字节
					tData[i++] = 20;								//回复数据长度低字节  
					for(j=0; j<20; j++)
					{
						if(pData[j+8]<10)// || (pData[j+8]==0xFF))
						{
							MT_SET_PARA_SYS_PHONENUMBER(id,j,pData[j+8]);
							tData[i++] = MT_GET_PARA_SYS_PHONENUMBER(id,j);		//填充回复数据缓存
							changeFlag = 1;
						}
						else
						{
							while(j<20)
							{
									MT_SET_PARA_SYS_PHONENUMBER(id,j,0xff);
								  j++;
							}	
							//errFlag = 1;
							changeFlag = 0;
							break;
						}
					}
					if((changeFlag) && (!errFlag))
					{
						I2C_PageWrite(STU_SYSTEMPARA_OFFSET,(unsigned char*)(&mt_sEPR),sizeof(mt_sEPR)); 
					}
				}
				else
				{
					errFlag = 1;								//参数错误
				}
			}
			else 
			{
				errFlag = 1;								//参数错误
			}
		}
		break;
		case GNL_ENDPOINT_GET_SENSOR_ID:
		{//
		
		}
		break;
		case GNL_ENDPOINT_SENSOR_ATT1:
		case GNL_ENDPOINT_SENSOR_ATT2:			//??-???????	
		case GNL_ENDPOINT_SENSOR_ATT3:			//??-???????
		case GNL_ENDPOINT_SENSOR_ATT4:			//??-???????		
		case GNL_ENDPOINT_SENSOR_ATT5:			//??-???????
		case GNL_ENDPOINT_SENSOR_ATT6:			//??-???????	
		case GNL_ENDPOINT_SENSOR_ATT7:			//??-???????
		case GNL_ENDPOINT_SENSOR_ATT8:			//??-???????			
		case GNL_ENDPOINT_SENSOR_ATT9:			//??-???????
		case GNL_ENDPOINT_SENSOR_ATT10:		  //??-???????	
		case GNL_ENDPOINT_SENSOR_ATT11:			//??-???????
		case GNL_ENDPOINT_SENSOR_ATT12:			//??-???????			
		case GNL_ENDPOINT_SENSOR_ATT13:		  //??-???????
		case GNL_ENDPOINT_SENSOR_ATT14:			//??-???????		
		case GNL_ENDPOINT_SENSOR_ATT15:			//??-???????
		case GNL_ENDPOINT_SENSOR_ATT16:			//??-???????	
		case GNL_ENDPOINT_SENSOR_ATT17:			//??-???????
		case GNL_ENDPOINT_SENSOR_ATT18:			//??-???????			
		case GNL_ENDPOINT_SENSOR_ATT19:			//??-???????
		case GNL_ENDPOINT_SENSOR_ATT20:			//??-???????				
		{
			temp16 = (pData[6]<<8)&0xFF00;
			temp16 |= pData[7];
			changeFlag = 0;
			if(temp16 == 23)		//判断数据长度是否正确
			{
				id = endPointNum-GNL_ENDPOINT_PHONE_NUMBER1;	//电话号码编号	
				if((pData[8] == 0) && (pData[9] == MT_GET_PARA_DEVICE_ID(id)))
				{// 防区存储的ID 需要一致，才可以配置相关参数
					tData[i++] = pData[8];
					tData[i++] = pData[9];
					if((MT_GET_PARA_DEVICE_MARK(id) == 1) && (pData[10] == 0))
							MT_SET_PARA_DEVICE_MARK(id,pData[10]);//
					tData[i++] = pData[10];
					tData[i++] = pData[11];
					
					if(pData[12] < DTC_TYP_SUM)///探测器类型
					{
							MT_SET_PARA_DEVICE_ALARMTYPE(id,(ZONE_TYPED_TYPEDEF)pData[12]);
							tData[i++] = pData[12];					
					}
					else
						  errFlag=1;			//参数错误
					for(j=0;j<16;j++)
					{
						  MT_SET_PARA_DEVICE_NAME_BYTE(id,j,pData[j+13]);
					  	tData[i++] = pData[j+13];	
					}
					if(pData[29] < STG_DEV_AT_SUM)
						MT_SET_PARA_DEVICE_ALARMTYPE(id,(ZONE_TYPED_TYPEDEF)pData[29]);	
					tData[i++] = pData[29];
					tData[i++] = pData[30];
					changeFlag = 1;
				}
				else
				{
					errFlag=1;			//参数错误
				}
				if((changeFlag) && (!errFlag))
				{
					I2C_PageWrite(STU_DEVICEPARA_OFFSET,(unsigned char*)(&dERP),sizeof(dERP));	//将新参数写入EEPROM 
				}					
			}
		}
		break;
		default:
		break;
	}	 
	if(errFlag)
	{
		tData[5] = 1;		//操作失败
		i = 10;
	} 
	tData[0] = (i>>8)&0xFF;	 
	tData[1] = i&0xFF;
	mt_protocol_DataPack(comType,tData);	
}	







//AA

//000950
//00 数据帧ID
//01  
//0001000A0A55

//AA000950


//extern void hal_usart2TxMsgInput(unsigned char *pData);
//数据头格式[0][1]数据长度 [2]通讯指令 [3]数据帧ID [4]网关协议指令
static void mt_protocol_EndpointDataParsing_Proc(unsigned char comType,unsigned char *pData,unsigned char len)
{
	//数据从数据帧ID开始
	unsigned char *pDat;
	unsigned short lenx;
	pDat = pData;
	len = (pDat[0]<<8)&0xFF00;
	len |= pDat[1];
	len -= 3;		//3=命令(1字节)+校验值(1字节)+帧尾(1字节)	
//注意：通过回调函数发送数据从数据帧ID字节开始,即[3]	
	switch(pDat[4])			//查询命令
	{
		case GLINK_M_GCMD_GET_ENDPOINT:  /////服务器从终端 获取端点数据	
		  mt_protocol_gatewayGetEndPointPro(comType,&pDat[3],lenx);
		break;
		case GLINK_M_GCMD_CHANGE_ENDPOINT: ////服务器更改设备的端点数据
		  mt_protocol_gatewayChangeEndPointPro(comType,&pDat[3],lenx);
		break;                                        
	}
}

static void gLink_M_Cmd_QueryFirmWare(unsigned char comType)
{
	unsigned char DataBuff[128];
	unsigned short idx;
	idx = 2;
	DataBuff[idx++] = GLINK_M_CMD_QUERY_UPDATE;	//命令
	DataBuff[idx++] = 0;					//数据帧ID
	DataBuff[idx++] = 0;					//获取MCU固件版本
	DataBuff[0] = (idx>>8)&0xFF;	
	DataBuff[1] = idx&0xFF;	
	mt_protocol_DataPack(comType,DataBuff);
}

///////////
void mt_protocol_GetFirmwarePack(unsigned char comType,unsigned short packNum,unsigned char *ver)
{
	unsigned char DataBuff[128];
	unsigned short idx;
    static unsigned char dataID = 0;
	idx = 2;
	//AA 00 06 
	//29 00 03 20 0C 55     +800北京时间
	DataBuff[idx++] = GLINK_S_CMD_UPDATE_DATA;	//命令
	DataBuff[idx++] = dataID++;					//数据帧ID   
 	DataBuff[idx++] = 0;					      //MCU updata;
	DataBuff[idx++] = *ver;					// version H
	ver++;
	DataBuff[idx++] = *ver;				  //version L		
	DataBuff[idx++] = (packNum>>8)&0xFF;					//数据帧ID
	DataBuff[idx++] = packNum&0xFF;						//UTC高字节	
	
	
	DataBuff[0] = (idx>>8)&0xFF;	
	DataBuff[1] = idx&0xFF;	
	mt_protocol_DataPack(comType,DataBuff);
}
////////







