#include "hal_wTn6.h"
#include "stm32F10x.h"
#include "hal_timer.h"

#define WTN6_CLK_PORT    GPIOC
#define WTN6_CLK_PIN     GPIO_Pin_1

#define WTN6_DAT_PORT    GPIOC
#define WTN6_DAT_PIN     GPIO_Pin_2

#define SC8002_SH_PORT	 GPIOA
#define SC8002_SH_PIN    GPIO_Pin_0

#define WTN6_CLK_LOW   GPIO_ResetBits(WTN6_CLK_PORT,WTN6_CLK_PIN)
#define WTN6_CLK_HIG   GPIO_SetBits(WTN6_CLK_PORT,WTN6_CLK_PIN)

#define WTN6_DAT_LOW   GPIO_ResetBits(WTN6_DAT_PORT,WTN6_DAT_PIN)
#define WTN6_DAT_HIG   GPIO_SetBits(WTN6_DAT_PORT,WTN6_DAT_PIN)

#define SC8002_SH_LOW   GPIO_ResetBits(SC8002_SH_PORT,SC8002_SH_PIN)
#define SC8002_SH_HIG   GPIO_SetBits(SC8002_SH_PORT,SC8002_SH_PIN)
unsigned short wtn6[] = {50,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3};	
unsigned char Wtn6Playflag;// CLK???
unsigned char Wtn6VolueNum;// ???????
unsigned char Wtn6NextNum; // ????????
volatile unsigned short Wtn6Timer;  //??WTN????
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

	GPIO_InitStructure.GPIO_Pin = SC8002_SH_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
	GPIO_Init(SC8002_SH_PORT, &GPIO_InitStructure);	
	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; ; 
	GPIO_Init(GPIOC, &GPIO_InitStructure);	
	
	GPIO_SetBits(GPIOC,GPIO_Pin_1);	
	GPIO_SetBits(GPIOC,GPIO_Pin_2);
	GPIO_SetBits(SC8002_SH_PORT,SC8002_SH_PIN);	
}

void hal_wtn6(void)
{
	hal_Wtn6Config();
	WTN6_DAT_HIG;
	WTN6_CLK_HIG;
	Wtn6Playflag = 0;
	Wtn6VolueNum = 0;
	Wtn6NextNum = 0xff;
}

static void hal_Wtn6_PlayHandle(void)
{
	static unsigned char VolueNum;
	Wtn6Timer--;
	if(Wtn6Timer == 0)
	{
			Wtn6Playflag ++;
		        Wtn6Timer= wtn6[Wtn6Playflag];
			switch(Wtn6Playflag)
			{
				case 1:
					VolueNum = Wtn6VolueNum;
					WTN6_CLK_LOW;
					if(VolueNum & 0x01)
					{
							WTN6_DAT_HIG;
					}
					else
					{
							WTN6_DAT_LOW;
					}		
				break;
				case 2:
				case 4:
				case 6:
				case 8:
				case 10:
				case 12:
				case 14:
			  case 16:
				{
						WTN6_CLK_HIG;	
					  VolueNum >>= 1;
				}		
				break;
				case 3:
				case 5:
				case 7:
				case 9:
				case 11:
				case 13:
				case 15:	
				{
					WTN6_CLK_LOW;	
					if(VolueNum & 0x01)
					{
							WTN6_DAT_HIG;
					}
					else
					{
							WTN6_DAT_LOW;
					}				
				}
				break;
			}
			if(Wtn6Playflag == 17)
			{
				WTN6_DAT_HIG;
				WTN6_CLK_HIG;
				Wtn6Playflag = 0;
				if(Wtn6NextNum != 0xff)
				{
					hal_Wtn6_PlayVolue(Wtn6NextNum);
					Wtn6NextNum = 0xff;
				}
				return;
			}
	}
	hal_ResetTimer(T_WTN6,T_STA_START);	
}


void hal_Wtn6_PlayVolue(unsigned char VolNum)
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



