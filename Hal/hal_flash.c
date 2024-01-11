#include "stm32F10x.h"
#include "hal_flash.h"

void hal_spi2Init(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* Enable SPI2 and GPIOA clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	/* Configure SPI2 pins: NSS, SCK, MISO and MOSI */
	GPIO_InitStructure.GPIO_Pin = SPI2_SCK_PIN | SPI2_MISO_PIN | SPI2_MOSI_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(SPI2_SCK_PORT, &GPIO_InitStructure);
	
	//SPI2 NSS 
	GPIO_InitStructure.GPIO_Pin = SPI2_NSS_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(SPI2_NSS_PORT, &GPIO_InitStructure);
	GPIO_SetBits(SPI2_NSS_PORT,SPI2_NSS_PIN);
	
	/* SPI2 configuration */ 
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //SPI1设置为两线全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;	                     //设置SPI1为主模式
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                  //SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;	 		                   //串行时钟在不操作时，时钟为高电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;		                   //第二个时钟沿开始采样数据
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;			                     //NSS信号由软件（使用SSI位）管理
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; //定义波特率预分频的值:波特率预分频值为8
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;				         //数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;						               //CRC值计算的多项式
	SPI_Init(SPI2, &SPI_InitStructure);
	/* Enable SPI2  */
	SPI_Cmd(SPI2, ENABLE); 											  //使能SPI2外设
	
	hal_spi2CSDrive(1);
	
}  

void hal_spi2CSDrive(unsigned char sta)
{
	if(sta)
		GPIO_SetBits(SPI2_NSS_PORT,SPI2_NSS_PIN);		
	else
		GPIO_ResetBits(SPI2_NSS_PORT,SPI2_NSS_PIN);
}

//SPIx 读写一个字节
//返回值:读取到的字节
unsigned char  hal_spi2ReadWriteByte(unsigned char  TxData)
{		
	unsigned char retry=0;				 
	while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_TXE)==RESET)//等待发送区空	
	{
		retry++;
		if(retry>200)
			return 0;
	}	
  SPI_I2S_SendData(SPI2,TxData);	
	retry=0;
	while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE)==RESET)//等待发送区空	
	{
		retry++;
		if(retry>200)
			return 0;
	}	  						    
	return SPI_I2S_ReceiveData(SPI2);//SPI2->DR;          //返回收到的数据						    
}
