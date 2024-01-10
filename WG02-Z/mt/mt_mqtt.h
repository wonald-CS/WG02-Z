#ifndef ____MT_MQTT_H_
#define ____MT_MQTT_H_

#define MQTT_LINKID_SIZE_MAX    	30
#define MQTT_USERNAME_SIZE_MAX  	30
#define MQTT_PASSWORD_SIZE_MAX  	32
#define MQTT_SERVERIP_SIZE_MAX  	16
#define MQTT_SERVERPORT_SIZE_MAX  	10
#define MQTT_TPIC_SIZE_MAX          30



//extern str_mqtt_Upevent mqtt_upEvent;


typedef enum
{
	MQTT_REC_MESSAGE_NEW, ///有新固件
	MQTT_REC_MESSAGE_FREE,///没有固件
}en_mqtt_recNewFlag;

typedef struct
{
	unsigned char linkID[MQTT_LINKID_SIZE_MAX];
	unsigned char username[MQTT_USERNAME_SIZE_MAX];
	unsigned char password[MQTT_PASSWORD_SIZE_MAX];
	unsigned char serverIp[MQTT_SERVERIP_SIZE_MAX];
	unsigned char serverPort[MQTT_SERVERPORT_SIZE_MAX];
	unsigned char subtopic[MQTT_TPIC_SIZE_MAX];
  unsigned char pubtopic[MQTT_TPIC_SIZE_MAX];
	en_mqtt_recNewFlag Newflag;    ///报警主机是否有新的固件 
}str_mqtt_parameter;

extern str_mqtt_parameter  mqtt_para;

void mt_mqtt_init(void);

void mt_mqtt_SetNewFlag(en_mqtt_recNewFlag flag);
unsigned char mt_mqtt_GetNewFlag(void);

#endif

