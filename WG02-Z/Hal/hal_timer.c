#include "stm32F10x.h"
#include "hal_timer.h"
#include "hal_led.h"
#include "string.h"

volatile Stu_TimerTypedef Stu_Timer[T_SUM];

/*******************************************************************************
* Function Name  : static void hal_timer4Config(void)
* Description    : ��ʱ��Ӳ�����ú���
* Input          : None
* Output         : None
* Return         : None
* Attention		 	 : None
*******************************************************************************/
static void hal_timer4Config(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	
	TIM_DeInit(TIM4); 
	TIM_TimeBaseStructure.TIM_Period = 50; 			    // 50uS
	TIM_TimeBaseStructure.TIM_Prescaler = SystemCoreClock/1000000 - 1;              
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);	
	
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE); ///�򿪶�ʱ��4�ж�				 
	TIM_Cmd(TIM4, ENABLE);   ///�򿪶�ʱ��4

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����NVIC�жϷ���2
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;///��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;        ///�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


/*******************************************************************************
* Function Name  : void hal_timerInit(void)
* Description    : ��ʱ����ʼ��
* Input          : None
* Output         : None
* Return         : None
* Attention		 	 : None
*******************************************************************************/
void hal_timerInit(void)
{
	unsigned char i;
	hal_timer4Config();
	for(i=0; i<T_SUM; i++)
	{
		Stu_Timer[i].state = T_STA_STOP;
		Stu_Timer[i].CurrentCount = 0;
		Stu_Timer[i].Period = 0;
		Stu_Timer[i].func = 0;
	}
}

//////////////////////////////



/*******************************************************************************
* Function Name  : hal_CreatTimer(TIMER_ID_TYPEDEF id,void (*proc)(void), unsigned short Period,unsigned char state)
* Description    : ������ʱ�� 
* Input          : - id����ʱ��ID
*									- (*proc)() ����ָ�� 
*									- Period ��ʱ���ڣ���λ50us
* 								- state ��ʱ����ʼ״̬
* Output         : None
* Return         : None
* Attention		 	 : None
*******************************************************************************/
void hal_CreatTimer(TIMER_ID_TYPEDEF id,void (*proc)(void), unsigned short Period,TIMER_STATE_TYPEDEF state)
{	
	Stu_Timer[id].state = state;
 
	Stu_Timer[id].CurrentCount = 0;
	Stu_Timer[id].Period = Period;
	Stu_Timer[id].func = proc;
}



/*******************************************************************************
* Function Name  : unsigned char hal_CtrlTimerAction(TIMER_ID_TYPEDEF id,TIMER_STATE_TYPEDEF sta)
* Description    : ���ƶ�ʱ������ 
* Input          : - id����ʱ��ID
*								 	 - sta ��ʱ��״̬
* Output         : None
* Return         : None
* Attention		 	 : None
*******************************************************************************/
TIMER_RESULT_TYPEDEF hal_CtrlTimerAction(TIMER_ID_TYPEDEF id,TIMER_STATE_TYPEDEF sta)
{
	if(Stu_Timer[id].func)		//�ж϶�ʱ���Ƿ����
	{
		Stu_Timer[id].state = sta;
		return T_SUCCESS;
	}else
	{
		return T_FAIL;
	}
}

/*******************************************************************************
* Function Name  : TIMER_STATE_RESULT_TYPEDEF	hal_GetTimerState(TIMER_ID_TYPEDEF id)
* Description    : ��ȡ��ʱ��״̬
* Input          : - id����ʱ��ID
*								 	 - sta ��ʱ��״̬
* Output         : None
* Return         : None
* Attention		 	 : None
*******************************************************************************/
TIMER_STATE_TYPEDEF	hal_GetTimerState(TIMER_ID_TYPEDEF id)
{
	if(Stu_Timer[id].func)		//�ж϶�ʱ���Ƿ����
	{
		return Stu_Timer[id].state;
	 
	}else
	{
		return T_STA_INVAILD;
	}
}


/*******************************************************************************
* Function Name  : hal_DeleteTimer(TIMER_ID_TYPEDEF id)
* Description    : ɾ����ʱ�� 
* Input          : - id����ʱ��ID
*								 
* Output         : None
* Return         : None
* Attention		 	 : None
*******************************************************************************/
TIMER_RESULT_TYPEDEF hal_DeleteTimer(TIMER_ID_TYPEDEF id)
{
	if(Stu_Timer[id].func)
	{
		Stu_Timer[id].state = T_STA_STOP;
	 
		Stu_Timer[id].CurrentCount = 0;
		//Stu_Timer[id].Period = 0;
		Stu_Timer[id].func = 0;
		return T_SUCCESS;
	}else
	{
		return T_FAIL;
	}
}

/*******************************************************************************
* Function Name  : hal_ResetTimer(TIMER_ID_TYPEDEF id,TIMER_STATE_TYPEDEF sta)
* Description    : ��λ��ʱ��״̬�ͼ�ʱʱ��
* Input          : - id����ʱ��ID
*								 	 - sta ��ʱ��״̬
* Output         : None
* Return         : None
* Attention		 	 : None
*******************************************************************************/
TIMER_RESULT_TYPEDEF hal_ResetTimer(TIMER_ID_TYPEDEF id,TIMER_STATE_TYPEDEF sta)
{
	if(Stu_Timer[id].func)		//�ж϶�ʱ���Ƿ����
	{
		Stu_Timer[id].state = sta;
		Stu_Timer[id].CurrentCount = 0;
		 
		return T_SUCCESS;
	}else
	{
		return T_FAIL;
	}
}
/*******************************************************************************
* Function Name  : static void Hal_TimerHandle(void)
* Description    : ��ʱ���жϼ�ʱ����
* Input          : None
* Output         : None
* Return         : None
* Attention		 	 : None
*******************************************************************************/
static void Hal_TimerHandle(void)
{
	unsigned char i;
	 
	for(i=0; i<T_SUM; i++)
	{
		if((Stu_Timer[i].func) && (Stu_Timer[i].state==T_STA_START))
		{	
				Stu_Timer[i].CurrentCount++;
			 if(Stu_Timer[i].CurrentCount >= Stu_Timer[i].Period)
			 {
				 Stu_Timer[i].state = T_STA_STOP;
			   Stu_Timer[i].func();
			}
		}	
	}
}

/*******************************************************************************
* Function Name  : void TIM4_IRQHandler(void)
* Description    : ��ʱ���жϻص�����
* Input          : None
* Output         : None
* Return         : None
* Attention		 	 : None
*******************************************************************************/
void TIM4_IRQHandler(void)
{
	Hal_TimerHandle();
	TIM_ClearFlag(TIM4, TIM_FLAG_Update);
}

/////////////////////////////////

