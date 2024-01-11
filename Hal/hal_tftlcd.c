#include "hal_tftlcd.h"
#include "stm32F10x.h"

// AC Check Pin
#define LCD_SCLK__PORT       GPIOB
#define LCD_SCLK__PIN        GPIO_Pin_3

//-----------------LCD端口定义---------------- 
#define LCD_SCLK_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_3)//SCL=SCLK
#define LCD_SCLK_Set() GPIO_SetBits(GPIOB,GPIO_Pin_3)

#define LCD_MOSI_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_5)//SDA=MOSI
#define LCD_MOSI_Set() GPIO_SetBits(GPIOB,GPIO_Pin_5)

#define LCD_DC_Clr()   GPIO_ResetBits(GPIOB,GPIO_Pin_4)//DC
#define LCD_DC_Set()   GPIO_SetBits(GPIOB,GPIO_Pin_4)

#define LCD_CS_Clr()   GPIO_ResetBits(GPIOB,GPIO_Pin_6)//CS
#define LCD_CS_Set()   GPIO_SetBits(GPIOB,GPIO_Pin_6)

#define LCD_RES_Clr()  GPIO_ResetBits(GPIOA,GPIO_Pin_15)//RES
#define LCD_RES_Set()  GPIO_SetBits(GPIOA,GPIO_Pin_15)

#define LCD_BLK_Clr()  GPIO_ResetBits(GPIOC,GPIO_Pin_10)//BLK
#define LCD_BLK_Set()  GPIO_SetBits(GPIOC,GPIO_Pin_10)


void hal_tftlcdConfig(void)
{
    SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure;
	DMA_InitTypeDef  DMA_InitStructure;//DMA初始化结构体

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE);  //相关IO的初始化
		
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

   //RES-PA15
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;	 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//速度50MHz
 	GPIO_Init(GPIOA, &GPIO_InitStructure);	  //初始化GPIOA
	GPIO_SetBits(GPIOA,GPIO_Pin_15);
	
	//CMD-PB4
	//CS-PB6
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_6;	 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//速度50MHz
 	GPIO_Init(GPIOB, &GPIO_InitStructure);	  //初始化GPIOA	
	GPIO_SetBits(GPIOB,GPIO_Pin_4|GPIO_Pin_6);
	
		//BLK-PC10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;	 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//速度50MHz
 	GPIO_Init(GPIOC, &GPIO_InitStructure);	  //初始化GPIOA		
	GPIO_ResetBits(GPIOC,GPIO_Pin_10);
	
	//CLK-PB3
	//MOSI-PB5
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_3 |GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	



/* SPI3 configuration */ 
	SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx; //SPI1设置为单线
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;	                     //设置SPI1为主模式
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                  //SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;	 		                   //串行时钟在不操作时，时钟为高电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;		                   //第二个时钟沿开始采样数据
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;			                     //NSS信号由软件（使用SSI位）管理
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2; //定义波特率预分频的值:波特率预分频值为8
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;				         //数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;						               //CRC值计算的多项式
	SPI_Init(SPI3, &SPI_InitStructure);

	//使能DMA发送
	DMA_DeInit(DMA2_Channel2); 
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI3->DR; //数据传输目标地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; 	////数据传输方向，从内存读取发送到外设
	DMA_InitStructure.DMA_BufferSize = 1024;            //发送Buff数据大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //设置外设地址是否递增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //设置内存地址是否递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //外设数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //内存数据宽度为8位	

	DMA_InitStructure.DMA_Mode =   DMA_Mode_Normal;                         //普通缓存模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                        //高优先级
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;            //禁止DMA2个内存相互访问
	DMA_Init(DMA2_Channel2, &DMA_InitStructure);        //初始化DMA,SPI在DMA1的通道2

	SPI_I2S_DMACmd(SPI3,SPI_I2S_DMAReq_Tx,ENABLE); /////使能SPI2 DMA发送功能*/	
	SPI_Cmd(SPI3, ENABLE);//使能SPI3
}


// SPI3 DMA发送
void DMA_SPI3_TX(unsigned char *buffer,unsigned short len)
{
	DMA2->IFCR |=(0xf<<4);    //清除通道2上面所有的标志位    1111 0000
	DMA2_Channel2->CNDTR=len; //设置要传输的数据长度
	DMA2_Channel2->CMAR=(u32)buffer; //设置RAM缓冲区地址
	DMA2_Channel2->CCR|=0x1;   ///启动DMA
	while(!(DMA2->ISR&(1<<5)))  ///等待数据数据传输完成
    {;}
	//while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) == RESET){;}///等待数据数据传输完成
	DMA2_Channel2->CCR &=(uint32_t)~0x1;//关闭DMA
}


void LCD_Writ_Bus(unsigned char dat) 
{	
	LCD_CS_Clr();
	DMA_SPI3_TX(&dat,1);
}

void hal_Oled_Display_on(void)
{
	LCD_BLK_Set();
}

void hal_Oled_Display_off(void)
{
	LCD_BLK_Clr();
}

void hal_oled_RestH(void)
{
	LCD_RES_Set();
}
void hal_oled_RestL(void)
{
	LCD_RES_Clr();
}

/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA8(unsigned char dat)
{
	LCD_Writ_Bus(dat);
}

/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA(unsigned short dat)
{
	unsigned char d[2];
	d[0] = dat>>8;
	d[1] = dat;
	DMA_SPI3_TX(&d[0],2);
}


/******************************************************************************
      函数说明：LCD写入命令
      入口数据：dat 写入的命令
      返回值：  无
******************************************************************************/
void LCD_WR_REG(unsigned char dat)
{
	LCD_DC_Clr();//写命令
	LCD_Writ_Bus(dat);
	LCD_DC_Set();//写数据
}












