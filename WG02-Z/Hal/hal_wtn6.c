#include "hal_wtn6.h"
#include "stm32F10x.h"
#include "hal_timer.h"

////语音芯片

												//5MS  100us
unsigned short wtn6[] = {50,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3};	
unsigned char Wtn6Playflag;			// CLK位置
unsigned char Wtn6VolueNum;			// 播放第几首语音
unsigned char Wtn6NextNum; 			// 下一首
volatile unsigned short Wtn6Timer;  //CLK持续时间

static void hal_Wtn6_PlayHandle(void);
	
static void hal_Wtn6Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);  	

	GPIO_InitStructure.GPIO_Pin = WTN6_CLK_PIN | WTN6_DAT_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; ; 
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; ; 
	GPIO_Init(GPIOC, &GPIO_InitStructure);	
	
	WTN6_DAT_HIGH;
	WTN6_CLK_HIGH;
	
}
void hal_SC8002Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = SC8002_SH_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
	GPIO_Init(SC8002_SH_PORT, &GPIO_InitStructure);	
	
	SC8002_SH_HIGH;
}
void hal_Wtn6Init(void)
{
	hal_Wtn6Config();
	hal_SC8002Config();
	Wtn6Playflag = 0;
	Wtn6VolueNum = 0;
	Wtn6NextNum = 0xff;
}

static void hal_Wtn6_PlayHandle(void)
{
    static unsigned char VolNum;
    static unsigned char Time_Num;
    Wtn6Timer--;
    if(Wtn6Timer == 0){
        Wtn6Playflag++;
        Wtn6Timer = wtn6[Wtn6Playflag];
        Time_Num = Wtn6Playflag;
        if(Time_Num % 2 == 0)
        {
            Time_Num = 1;
        }else{
            Time_Num = 0;
        }

        switch (Time_Num)
        {
					case 0:
					{
							if (Wtn6Playflag == 1)
							{
									VolNum = Wtn6VolueNum;
							}

							WTN6_CLK_LOW;
							if(VolNum & 0x01)
							{
									WTN6_DAT_HIGH;
							}else{
									WTN6_DAT_LOW;
							}             
							
							break;
					}

					case 1:
					{
							WTN6_CLK_HIGH;
							VolNum >>= 1;
						
							break;
					}
        
        }

			if(Wtn6Playflag == 17)
			{
				WTN6_DAT_HIGH;
				WTN6_CLK_HIGH;
				Wtn6Playflag = 0;
				if(Wtn6NextNum != 0xff)
				{
					hal_Wtn6_Play(Wtn6NextNum);
					Wtn6NextNum = 0xff;
				}
				return;
			}
    }

    hal_ResetTimer(T_WTN6,T_STA_START);	
}


void hal_Wtn6_Play(unsigned char VolNum)
{
	if(Wtn6Playflag == 0)
	{
		hal_CreatTimer(T_WTN6,hal_Wtn6_PlayHandle,2,T_STA_START);
		GPIO_SetBits(SC8002_SH_PORT,SC8002_SH_PIN);	
		WTN6_CLK_LOW;
		WTN6_DAT_LOW;
		Wtn6VolueNum = VolNum;
		Wtn6Playflag = 0;
		Wtn6Timer = wtn6[0];	
		Wtn6NextNum = 0xff;
	}
	else
	{
		Wtn6NextNum = VolNum;
	}
}



