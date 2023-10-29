#include "stm32F10x.h"
#include "hal_adc.h"

str_batVolt hal_batVolt;

static void hal_AdcInit(void);
static unsigned short hal_adc_GetAdcDat(unsigned char ch);
static en_batlevel hal_adc_batVoltCheck(void);

void  hal_Adc_batInit(void)
{
	hal_AdcInit();
	hal_batVolt.step = STEP_IDLE;
	hal_batVolt.adcTimes = 0;
	hal_batVolt.BatSumVolt = 0;
	hal_batVolt.VrefSumVolt = 0;
	hal_batVolt.batRank = LEVEL_FULL;
	hal_batVolt.chargeSta = STA_BAT_CHARGING;	
	hal_batVolt.chargeCheckDelay = 0;
}

void hal_BatCheckProc(void)
{
	static unsigned short interalTim = 0;
	static en_batlevel level = LEVEL_FULL;
	static unsigned char times = 0;
	en_batlevel batlevelSta;
	
	batlevelSta = hal_adc_batVoltCheck();
	if(batlevelSta == LEVEL_IDLE)
	{
		interalTim ++; ///电池电量检测间隔时间
		if(interalTim > HAL_ADC_GET_INTERVAL) ///50*10 =500
		{
			interalTim = 0;
			hal_batVolt.step = STEP_STARTGET_VOLT;
		}		
	}
	else if(batlevelSta == LEVEL_NODATA)
	{	
	}
	else
	{
		if(batlevelSta != level)
		{	
			times ++;   ///之前检测的电池电量 和新获取的电池电量不一样的次数
			if(times > 3)
			{
				times = 0;
				level = batlevelSta;
				hal_batVolt.batRank = level;
			}
		}			
		else
		{
			times = 0;
		}	 
	}
}


///3.2  3.10 3.12 3.13 3.15 3.16 3.10 3.12 3.13 3.15 3.16  / 10

static en_batlevel hal_adc_batVoltCheck(void)
{
	unsigned int batVoltage;
	switch((unsigned char)hal_batVolt.step)
	{
		case STEP_IDLE://空闲
		{
			hal_batVolt.adcTimes = 0;
			hal_batVolt.BatSumVolt = 0;
			hal_batVolt.VrefSumVolt = 0;
			return LEVEL_IDLE;
		}
//		break;
		case STEP_STARTGET_VOLT://开始ADC 检测
		{
			hal_batVolt.BatSumVolt += hal_adc_GetAdcDat(ADC_Channel_8);
			hal_batVolt.VrefSumVolt += hal_adc_GetAdcDat(ADC_Channel_10);
			
			hal_batVolt.adcTimes ++;
			if(hal_batVolt.adcTimes >= HAL_ADC_GETTIMES)
			{
				hal_batVolt.adcTimes = 0;
				hal_batVolt.step = STEP_GETVOLT_FINISH;
			}				
		}
		break;
		case STEP_GETVOLT_FINISH:// 电池电压检测完成
		{
			hal_batVolt.Adc_Vref = hal_batVolt.VrefSumVolt/HAL_ADC_GETTIMES;
			hal_batVolt.Adc_Bat = hal_batVolt.BatSumVolt/HAL_ADC_GETTIMES;
			hal_batVolt.step = STEP_DATEHANLE;
		}
		break;	
		case STEP_DATEHANLE://底层数据分析
		{
			 hal_batVolt.step = STEP_IDLE;
			///// 讲获取的ADC 的值转换成实际的电压值  电压值扩大100倍 *2是因为电压被分压
			 batVoltage = 2*((250*hal_batVolt.Adc_Bat)/hal_batVolt.Adc_Vref); 
			 if(batVoltage > BATTERY_LEV4_VOLT_VAL)
			 {
					return LEVEL_FULL;
			 }
			 else if(batVoltage > BATTERY_LEV3_VOLT_VAL)
			 {
					return LEVEL_VOLT_4;
			 }
			 else if(batVoltage > BATTERY_LEV4_VOLT_VAL)
			 {
					return LEVEL_VOLT_3;
			 }				 
			 else if(batVoltage > BATTERY_LEV1_VOLT_VAL)
			 {
					return LEVEL_VOLT_2;
			 }
			 else if(batVoltage > BATTERY_LEV0_VOLT_VAL)
			 {
					return LEVEL_VOLT_1;
			 }
			 else
			 {
					return LEVEL_LOW;
			 } 
		}
	}
	return LEVEL_NODATA;
}


unsigned char hal_adc_returnVoltLevel(void)
{
	return hal_batVolt.batRank;
}









static void hal_AdcInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE); 
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE); 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_AFIO, ENABLE);
  
	//PB0-电池电量检测 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);


////设置ADC分频因子6 72M/8=9,ADC最大时间不能超过14M
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);

	/* ADC1 configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;		// 独立工作模式;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;				//多通道扫描模式;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;		//连续转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	// 转换触发方式：转换由软件触发启动;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC 数据右对齐 ;
	ADC_InitStructure.ADC_NbrOfChannel = 2;					//进行规则转换的 ADC 通道的数目为3; 
	ADC_Init(ADC1, &ADC_InitStructure);

	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);

	/* 复位 ADC1 的校准寄存器 */	
	ADC_ResetCalibration(ADC1);

	/* 等待 ADC1 校准寄存器复位完成 */	
	while(ADC_GetResetCalibrationStatus(ADC1));

	/* 开始 ADC1 校准 */	
	ADC_StartCalibration(ADC1);

	/* 等待 ADC1 校准完成 */	
	while(ADC_GetCalibrationStatus(ADC1));
	 
	/* 启动 ADC1 转换 */ 
	//ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}


//获得ADC值
static unsigned short hal_adc_GetAdcDat(unsigned char ch)   
{
  	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}
/////













