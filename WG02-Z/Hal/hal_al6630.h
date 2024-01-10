#ifndef ____HAL_AL6630_H_
#define ____HAL_AL6630_H_

#define TIME_INTERVAL_GETTEMHUM  300

typedef enum
{
	TEMHUM_STEP0_IDEL,
	TEMHUM_STEP0_START,
	TEMHUM_STEP0_SENTHEAD_L,
	TEMHUM_STEP0_SENTHEAD_H,
	TEMHUM_STEP0_GET_TEMHUM_DAT,	
	TEMHUM_STEP0_GET_TEMHUM_FNISH,
	TEMHUM_STEP0_OVER,	
}en_getTemHumStep;

typedef struct
{
	unsigned short temDat;  //保存实时的温度
	unsigned short humDat;  //保存实时的湿度
	unsigned char step;    //0 stop 1 sent head 2 get data
	unsigned char Capfalg; //数据获取的的标志 
	unsigned char len;     //获取的数据长度  40
	unsigned int   CapCount;  ///捕获计数
	unsigned char Tim3_temHumBuf[5]; //保存接收到的5个bytes
}stu_temHum;

void hal_timer_capInit(void);
void hal_GetTemHumProc(void);
unsigned short hal_GethumidityDat(void);
unsigned short hal_GetTemperatureDat(void);


#endif
