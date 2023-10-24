#ifndef _MT_LORA_H_
#define _MT_LORA_H_


#define App_Net_Len 19
#define Else_Move_Len 6

typedef struct Data_AppNet_Re{
    unsigned char Head;
    unsigned char Len;
    unsigned char Data[16];
    unsigned char CheckSum;
}Data_AppNet_Receive;

typedef struct Data_Else_Re{
    unsigned char Head;
    unsigned char Len;
    unsigned char Data[3];
    unsigned char CheckSum;
}Data_Else_Receive;


typedef struct Data_AppNet_Dle{
	unsigned char cmd;              //功能码
	unsigned char node[2];          //CRC
	unsigned char Macaddr[12];      //探测器MAC地址
	unsigned char detectorType;     //探测器类型
}Data_AppNet_Handle;


typedef enum
{
	LORADET_LEARN_ING,
	LORADET_LEARN_OK,
	LORADET_LEARN_FAIL,
}en_LoraApplyNet;

typedef struct 
{
	en_LoraApplyNet state;
	unsigned char code;  
}str_LoraAppNetState;



typedef enum{
  Detector_IDLE = 0,
  Detector_AppNet,                //探测器申请入网
  Detector_HeartBeat,             //心跳包
  Detector_LowBatrery,            //电池低压
  Detector_SysAlert,              //系统报警
  Detector_Disarm = 6,            //撤防 
  Detector_AwayArm,               //离家布防       
  Detector_HomeArm,               //在家布防
  Detector_DoorClose,             //门磁关门
  
  Detector_Respose_AppNet = 0x81,
  Detector_Respose_HeartBeat,              //心跳包
  Detector_Respose_LowBatrery,            //电池低压
  Detector_Respose_SysAlert,              //系统报警
  Detector_Respose_Disarm = 0x86,         //撤防 
  Detector_Respose_AwayArm,               //离家布防       
  Detector_Respose_HomeArm,               //在家布防
  Detector_Respose_DoorClose,             //门磁关门 

  Detector_None = 0xff,
}Detector_EVENT;

typedef str_LoraAppNetState (*loraRxDataApplyNet_callback_t)(Detector_EVENT event,Data_AppNet_Handle pData); 
typedef unsigned char (*loraRxDataPro_callback_t)(Detector_EVENT event,Data_AppNet_Handle pData);

void mt_lora_Init(void);
void mt_lora_Config(void);
void mt_loraRxApplyNet_callback_Register(loraRxDataApplyNet_callback_t pCBS);
void mt_lora_loracomm_callback_Register(loraRxDataPro_callback_t pCBS);

#endif
