#ifndef ____MT_MQTT_H_
#define ____MT_MQTT_H_



#define MQTT_LINKID_SIZE_MAX    	30
#define MQTT_USERNAME_SIZE_MAX  	30
#define MQTT_PASSWORD_SIZE_MAX  	32
#define MQTT_SERVERIP_SIZE_MAX  	16
#define MQTT_SERVERPORT_SIZE_MAX  	10
#define MQTT_TPIC_SIZE_MAX          30

//typedef enum
//{
//	MQTT_UPEVENT_GET_TIME = (unsigned char)0,  ///
//	MQTT_UPEVENT_DISARM,
//	MQTT_UPEVENT_ARM,
//	MQTT_UPEVENT_HOMEARM,
//	MQTT_UPEVENT_ALARM, 
//}en_mqtt_Upevent;

typedef struct
{
	unsigned char event;
	unsigned char buffer[1];
}str_mqtt_Upevent;

//extern str_mqtt_Upevent mqtt_upEvent;

typedef enum 
{
	STA_MQTT_CLEAN=0,
	STA_MQTT_CONF,
	STA_MQTT_CONN,
	STA_MQTT_SUB,
	STA_MQTT_SUB_FIRMWARE_UPDATE,
	STA_MQTT_READY,
	STA_MQTT_PUB,
	
}en_mqtt_sta;

typedef enum
{
	MQTT_REC_MESSAGE_NEW, ///
	MQTT_REC_MESSAGE_FREE,
}en_mqtt_recNewFlag;

typedef struct
{
	unsigned char linkID[MQTT_LINKID_SIZE_MAX];
	unsigned char username[MQTT_USERNAME_SIZE_MAX];
	unsigned char password[MQTT_PASSWORD_SIZE_MAX];
	unsigned char serverIp[MQTT_SERVERIP_SIZE_MAX];
	unsigned char serverPort[MQTT_SERVERPORT_SIZE_MAX];
	unsigned char subtopic[MQTT_TPIC_SIZE_MAX];
	unsigned char subtopic_firmwareUpdate[MQTT_TPIC_SIZE_MAX];
  unsigned char pubtopic[MQTT_TPIC_SIZE_MAX];
	en_mqtt_recNewFlag Newflag;  
}str_mqtt_parameter;

extern str_mqtt_parameter  mqtt_para;

void mt_mqtt_init(void);

#endif

