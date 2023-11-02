#include "stm32F10x.h"
#include "hal_al6630.h"
#include "string.h"


stu_temHum temHumBufDat;


static void hal_timer3CapConfig(unsigned short arr,unsigned short psc)
{	 
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_ICInitTypeDef  TIM3_ICInitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
	
    //第一步：打开相关时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	//使能TIM3时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  //使能GPIOA时钟
	
	//第二步： 配置信号IO 
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_7;  //PA7 清除之前设置  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //PA0 输入  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_7);						 // 初始化为高电平
	

	//第3步：初始化定时器3 TIM3	 
	TIM_TimeBaseStructure.TIM_Period = arr; //设定计数器自动重装值 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	//预分频器   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
  
	//第4步：初始化TIM3输入捕获参数
	TIM3_ICInitStructure.TIM_Channel = TIM_Channel_2; //CC1S=01 	选择输入端 IC1映射到TI1上
	TIM3_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;	//上升沿捕获
	TIM3_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
	TIM3_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
	TIM3_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 配置输入滤波器 不滤波
	TIM_ICInit(TIM3, &TIM3_ICInitStructure);
	
	//第5步：中断分组初始化
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器 
	
	//第6步：打开定时器3向上计数 和通道2的捕获功能
	TIM_ITConfig(TIM3,TIM_IT_Update|TIM_IT_CC2,ENABLE);//允许更新中断 ,允许CC1IE捕获中断	
   
    TIM_Cmd(TIM3,DISABLE ); 	              // 
}
//功能: PA7 输入  输出 配置函数。    
//参数: direc : =0 输入         =1 输出 并输出低电平
static void hal_Tim3_SentDatPin(unsigned char direc)
{
	GPIO_InitTypeDef GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  //使能GPIOA时钟
	if(direc == 1)
	{
		GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_7;  //PA7 清除之前设置  
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //PA7  
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		GPIO_ResetBits(GPIOA,GPIO_Pin_7);				//PA7 下拉	 输出低电平	
	}
	else
	{
		GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_7;  //PA7 清除之前设置 	
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //PA 
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		GPIO_ResetBits(GPIOA,GPIO_Pin_7);						 //
	}
}


void hal_timer_capInit(void)
{
	temHumBufDat.temDat = 0;   //0.01
	temHumBufDat.humDat = 0;   //0.01
  temHumBufDat.step = TEMHUM_STEP0_IDEL;
  hal_timer3CapConfig(0,71);	
}


static void hal_Tim_CapInit(void)
{
	temHumBufDat.Capfalg = 0;
	temHumBufDat.len = 0;
	temHumBufDat.CapCount = 0;
	memset(temHumBufDat.Tim3_temHumBuf,0,5);
}
///
unsigned short hal_GetTemperatureDat(void)
{
	return temHumBufDat.temDat;
}
unsigned short hal_GethumidityDat(void)
{
	return temHumBufDat.humDat;
}

typedef enum
{
	TIMCAP_STATE_WAIT,
	TIMCAP_STATE_SUC,
	TIMCAP_STATE_FAIL,
}en_Tim3CapStateflag;




void hal_GetTemHumProc(void)
{
	static unsigned int getTemHumIntervalTim = 0;//TIME_INTERVAL_GETTEMHUM;
	unsigned char sumcheck;
	
	getTemHumIntervalTim ++;  //10ms  *300 = 3000ms  
	if(getTemHumIntervalTim > TIME_INTERVAL_GETTEMHUM)
	{
	    getTemHumIntervalTim = 0;		 
		temHumBufDat.step = TEMHUM_STEP0_START; 
	}
	
	
	switch(temHumBufDat.step)
	{
		case TEMHUM_STEP0_IDEL:
		{
		}
		break;
		case TEMHUM_STEP0_START: 
       {
			hal_Tim_CapInit();
			temHumBufDat.step = TEMHUM_STEP0_SENTHEAD_L;
			hal_Tim3_SentDatPin(1);
		}			
		break;	//00000010 1010 1100 
		        // 0000 0001 0001 1000 11000
		case TEMHUM_STEP0_SENTHEAD_L:
		{//发起起始信号 10ms
			GPIO_SetBits(GPIOA,GPIO_Pin_7);
			hal_Tim3_SentDatPin(0); ///配置为输入模式
			hal_Tim_CapInit();  //初始化参数
			temHumBufDat.step = TEMHUM_STEP0_GET_TEMHUM_DAT;
			hal_timer3CapConfig(0XFFFF,72-1);	///单片机工作的时钟是72M  0-1   71-72   1M  0.000 001S 1us
	        TIM_Cmd(TIM3, ENABLE);
			TIM_SetCounter(TIM3,0);
		}
		break;		
		case TEMHUM_STEP0_GET_TEMHUM_DAT:
		{
			if(temHumBufDat.Capfalg == 1)
			{//完成数据捕获
				temHumBufDat.Capfalg = 0;
				temHumBufDat.step = TEMHUM_STEP0_IDEL;
				sumcheck = temHumBufDat.Tim3_temHumBuf[0];
				sumcheck += temHumBufDat.Tim3_temHumBuf[1];
				sumcheck += temHumBufDat.Tim3_temHumBuf[2];
				sumcheck += temHumBufDat.Tim3_temHumBuf[3];				
				if(sumcheck == temHumBufDat.Tim3_temHumBuf[4])
				{///校验成功
					temHumBufDat.humDat =  temHumBufDat.Tim3_temHumBuf[0]*256 + temHumBufDat.Tim3_temHumBuf[1];
					temHumBufDat.temDat =  temHumBufDat.Tim3_temHumBuf[2]*256 + temHumBufDat.Tim3_temHumBuf[3];
				}
			}
		}
		break;				
	}
}
///
//定时器3中断服务程序	 
void TIM3_IRQHandler(void)
{   
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		
	}
	if(TIM_GetITStatus(TIM3, TIM_IT_CC2) != RESET)//捕获1发生捕获事件
	{	
		temHumBufDat.CapCount = TIM_GetCapture2(TIM3);
		
		if((temHumBufDat.CapCount<180) && (temHumBufDat.CapCount>140)) 
		{
			temHumBufDat.len = 0;
		}
		else
		{//00000000  00000000   00000000   00000000   00000000  
			if((temHumBufDat.CapCount>60) && (temHumBufDat.CapCount<90))
			{//0
			}
			else if((temHumBufDat.CapCount<135) && (temHumBufDat.CapCount>100))
			{//   9/8 = 1   9%8 = 1  1000 000>>1 = 0100 0000
				temHumBufDat.Tim3_temHumBuf[temHumBufDat.len/8] |= (0x80>>(temHumBufDat.len%8));
			}
			temHumBufDat.len ++;
		}
		if(temHumBufDat.len == 40)
		{///获取数据结束
			TIM_Cmd(TIM3, DISABLE); 
			temHumBufDat.Capfalg = 1;
		}			
		TIM_SetCounter(TIM3,0);
	}		    		     	    					   
	TIM_ClearITPendingBit(TIM3, TIM_IT_CC2|TIM_IT_Update); //清除中断标志位
}











