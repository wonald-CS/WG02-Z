#include "stm32F10x.h"
#include "hal_al6630.h"
#include "string.h"

//温湿度芯片

Str_TemHum TemHum_Para;


static void hal_timer3CapConfig(unsigned short arr,unsigned short psc)
{	 
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_ICInitTypeDef  TIM3_ICInitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  	NVIC_InitTypeDef NVIC_InitStructure;
	//打开时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	//使能TIM3时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  //使能GPIOA时钟
	//配置GPIO口
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_7;  //PA7 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //无配上拉电阻，所以推挽输出，空闲时PA7为高电平  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_7);						 // 初始化为高电平
	
	//初始化定时器3 TIM3（定时器计数器溢出时进入中断服务程序）	 
	TIM_TimeBaseStructure.TIM_Period = arr; //设定计数器自动重装值 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	//预分频器   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
  
	//初始化TIM3输入捕获参数（定时器捕获到下降沿即进入中断服务程序）
	TIM3_ICInitStructure.TIM_Channel = TIM_Channel_2; 
	TIM3_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;	//下降沿捕获
	TIM3_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; 
	TIM3_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
	TIM3_ICInitStructure.TIM_ICFilter = 0x00;
	TIM_ICInit(TIM3, &TIM3_ICInitStructure);
	
	//中断分组初始化
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器 
	
	//打开定时器3向上计数和通道2的捕获功能
	TIM_ITConfig(TIM3,TIM_IT_Update|TIM_IT_CC2,ENABLE);//允许更新中断 ,允许CC2IE捕获中断	
    
	TIM_Cmd(TIM3,DISABLE); 	
}


//功能: PA7 输入  输出 配置函数。    
//参数: direc : =0 输入         =1 输出 并输出低电平
static void hal_Tim3_SentDatPin(unsigned char direc)
{
	GPIO_InitTypeDef GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  //使能GPIOA时钟
	if(direc == 1){
		GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_7; 
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		//PA7输出 低电平（起始信号，用作唤醒al6630）
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		GPIO_ResetBits(GPIOA,GPIO_Pin_7);						
	}else{
		GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_7;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 	//PA7输入
		GPIO_Init(GPIOA, &GPIO_InitStructure);			
	}
}


unsigned short Get_Temperature_Data(void)
{
	return TemHum_Para.Temperature;
}

unsigned short Get_Humidity_Data(void)
{
	return TemHum_Para.Humidity;
}


void hal_Al6630_Init(void)
{
	TemHum_Para.Temperature = 0;
	TemHum_Para.Humidity = 0;
	TemHum_Para.TemHum_Step = TemHum_Step0_Idle;
	hal_timer3CapConfig(0XFFFF,72-1);;  //配置定时器
}


void Str_TemHum_Init(void)
{
	TemHum_Para.TemHum_Step = TemHum_Step3_Respond;
	TemHum_Para.Down_GetPos = 0;
	TemHum_Para.Data_Len = 0;
	TemHum_Para.Data_GetFlag = 0;
	memset(TemHum_Para.TemHum_Rec,0,5);
}

void hal_GetTemHum_Proc(void)
{
	static unsigned int GetTemHum_Timer;
	unsigned char SumCheck;
	unsigned char iLoop;
	
	GetTemHum_Timer ++;
	
	if(GetTemHum_Timer > Det_TemHum_Time)
	{
		GetTemHum_Timer = 0;
		TemHum_Para.TemHum_Step = TemHum_Step1_Start;
	}


	switch (TemHum_Para.TemHum_Step)
	{
	case TemHum_Step0_Idle:
		break;

	case TemHum_Step1_Start:
		Str_TemHum_Init();
		hal_Tim3_SentDatPin(1);
		break;

	case TemHum_Step3_Respond:
		GPIO_SetBits(GPIOA,GPIO_Pin_7);	   //TemHum_Step2_McuRelease,短暂拉高PA7以作主机释放(10~200us) 
		TIM_Cmd(TIM3,ENABLE); 			   //使能定时器（计数和捕获）	
		hal_Tim3_SentDatPin(0);			   //经过测试，释放主机到此处把PA7重新配置为输入，用了14~30us，符合条件
		break;

	case TemHum_Step4_Data:
		if(TemHum_Para.Data_GetFlag)
		{
			TemHum_Para.TemHum_Step = TemHum_Step0_Idle;
			TemHum_Para.Data_GetFlag = 0;
			SumCheck = TemHum_Para.TemHum_Rec[0];
			for(iLoop=1;iLoop<4;iLoop++)
			{
				SumCheck += TemHum_Para.TemHum_Rec[iLoop];
			}

			if (SumCheck == TemHum_Para.TemHum_Rec[4])      
			{
				TemHum_Para.Humidity =  (TemHum_Para.TemHum_Rec[0] << 8) +  TemHum_Para.TemHum_Rec[1];
				TemHum_Para.Temperature = (TemHum_Para.TemHum_Rec[2] << 8) +  TemHum_Para.TemHum_Rec[3];
			}
			
			SumCheck = 0;
			
		}
		break;
	}

}


//定时器3中断服务程序	 
void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)	//向上计数溢出事件
	{
		
	}

	if(TIM_GetITStatus(TIM3, TIM_IT_CC2) != RESET)//发生捕获事件。捕获完一次，从该捕获位置开始记录，如160us，到下一次捕获240us，那么TIM_GetCapture1(TIM3) = 80us；
	{
		TemHum_Para.Down_GetPos = TIM_GetCapture2(TIM3);
		//if((TemHum_Para.Down_GetPos < 170) && (TemHum_Para.Down_GetPos > 150))   //第二个下降沿，响应结束此处应该为160us（不需要判断第一个下降沿）
		if((TemHum_Para.Down_GetPos < 180) && (TemHum_Para.Down_GetPos > 140))
		{
			TemHum_Para.Data_Len = 0;
			TemHum_Para.TemHum_Step = TemHum_Step4_Data;
		}
		else
		{
			//数据先低后高，那么取下一个数据下降沿的数值作前一个数据判断0或1
			//if ((TemHum_Para.Down_GetPos > 70) && (TemHum_Para.Down_GetPos < 85))    //数据0
			if ((TemHum_Para.Down_GetPos > 60) && (TemHum_Para.Down_GetPos < 90))
			{	
			}
			//else if ((TemHum_Para.Down_GetPos > 116) && (TemHum_Para.Down_GetPos < 130))  //数据1
			else if((TemHum_Para.Down_GetPos<135) && (TemHum_Para.Down_GetPos>100))
			{
				TemHum_Para.TemHum_Rec[TemHum_Para.Data_Len/8] |= (0x80 >> (TemHum_Para.Data_Len%8));
			}	

			TemHum_Para.Data_Len ++;	

		}
		if(TemHum_Para.Data_Len == 40)
		{///获取数据结束
			TIM_Cmd(TIM3, DISABLE); 
			TemHum_Para.Data_GetFlag = 1;
			
		}			
		TIM_SetCounter(TIM3,0);
		
	}

	TIM_ClearITPendingBit(TIM3,TIM_IT_CC2|TIM_IT_Update); //清除中断标志位		
	
}

