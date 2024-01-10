#ifndef ____HAL_ADC_H_
#define ____HAL_ADC_H_



#define HAL_ADC_GETTIMES   10 //每轮检测ADC 检测的次数
 
#define HAL_ADC_GET_INTERVAL  50 //电池检测间隔时间    

/////电池低压 *1000.  放大100倍数
#define BATTERY_LEV0_VOLT_VAL		360	//电量最低
#define BATTERY_LEV1_VOLT_VAL		370	
#define BATTERY_LEV2_VOLT_VAL		380
#define BATTERY_LEV3_VOLT_VAL		390
#define BATTERY_LEV4_VOLT_VAL		400	//电量最高	

typedef enum
{
	STEP_IDLE,   //空闲
	STEP_STARTGET_VOLT,  //开始ADC 检测
	STEP_GETVOLT_FINISH,  // 电池电压检测完成
	STEP_DATEHANLE,      //底层数据分析

}en_batStep;

typedef enum
{
	LEVEL_LOW,
	LEVEL_VOLT_1,
	LEVEL_VOLT_2,
	LEVEL_VOLT_3,
	LEVEL_VOLT_4,
	LEVEL_FULL,
	LEVEL_NODATA,
	LEVEL_IDLE,
}en_batlevel;

typedef enum
{
	STA_BAT_CHARGING,
	STA_BAT_STOP,
}en_batChargeSta;



typedef struct
{
	en_batStep step;
	unsigned char adcTimes;      //每次检测 ADC 转换的次数
	unsigned int BatSumVolt;     //检测数值过程累计电压
	unsigned int VrefSumVolt;    //检测数值过程累计电压
	unsigned short Adc_Vref;     //ADC参考电压
	unsigned short Adc_Bat;      //Bat ADC的值
	en_batChargeSta chargeSta; ///充电状态
	unsigned short chargeCheckDelay;
	en_batlevel batRank;       //电池电压的状态
}str_batVolt;


void hal_Adc_batInit(void);
void hal_BatCheckProc(void);
unsigned char hal_adc_returnVoltLevel(void);

#endif



