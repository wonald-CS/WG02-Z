#ifndef ____MT_LORA_H_
#define ____MT_LORA_H_

typedef enum //typevent :char
{
	LORA_COM_APPLY_NET = 1, ///申请入网
	LORA_COM_HEAT,   ///心跳包
	LORA_COM_BAT_LOW,//电池低压
	LORA_COM_ALARM,  //系统报警
	LORA_COM_TAMPER, //防拆按键
	LORA_COM_DISARM, //系统撤防  遥控器
    LORA_COM_AWAYARM,  //离家布防
	LORA_COM_HOMEARM, //在家布防
	LORA_COM_CLOSE,   //门磁开门

	LORA_COM_RESPOSE_APPLY_NET = 0X81, ///申请入网
	LORA_COM_RESPOSE_HEAT,  ///心跳包
	LORA_COM_RESPOSE_BAT_LOW,//电池低压
	LORA_COM_RESPOSE_ALARM,  //系统报警
	LORA_COM_RESPOSE_TAMPER, //防拆按键
	LORA_COM_RESPOSE_DISARM, //系统撤防  遥控器
    LORA_COM_RESPOSE_AWAYARM,  //离家布防
	LORA_COM_RESPOSE_HOMEARM, //在家布防
	LORA_COM_RESPOSE_CLOSE,   //门磁开门
	
	LORA_COM_RESPOSE_NONODE = 0xff,   //没有找到相关节点

}en_lora_eventTypedef;


typedef enum
{
	LORADET_LEARN_ING,
	LORADET_LEARN_OK,
	LORADET_LEARN_FAIL,
}en_LoraApplyNet;

typedef struct 
{
	en_LoraApplyNet state;
	unsigned char code;  //
}str_LoraAppNetState;


typedef struct  
{
	//en_lora_eventTypedef cmd;
	unsigned char cmd;
	unsigned char node[2];
	unsigned char addr[12];
	unsigned char detectorType;/* data */
}str_cmdApplyNet;

#define STR_COMAPPLYNET_SIZE	sizeof(str_cmdApplyNet)	

typedef str_LoraAppNetState (*loraRxDataApplyNet_callback_t)(en_lora_eventTypedef event,str_cmdApplyNet pData); 
typedef unsigned char (*loraRxDataPro_callback_t)(en_lora_eventTypedef event,str_cmdApplyNet pData);

void mt_lora_init(void);
void mt_lora_Pro(void);
void mt_loraRxApplyNet_callback_Register(loraRxDataApplyNet_callback_t pCBS);
void mt_lora_loracomm_callback_Register(loraRxDataPro_callback_t pCBS);

#endif
