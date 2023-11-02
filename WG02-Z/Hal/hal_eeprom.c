#include "stm32F10x.h"
#include "hal_eeprom.h"



#define I2C_SCL_PORT	GPIOB
#define I2C_SCL_PIN		GPIO_Pin_8

#define I2C_SDA_PORT	GPIOB
#define I2C_SDA_PIN		GPIO_Pin_9


static void hal_I2C_SDA(unsigned char bVal);
static void hal_I2C_SCL(unsigned char bVal);

static void hal_I2CConfig(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure; 
  /* Configure I2C2 pins: PB8->SCL and PB9->SDA */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  GPIO_InitStructure.GPIO_Pin =  I2C_SCL_PIN | I2C_SDA_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;  
  GPIO_Init(I2C_SCL_PORT, &GPIO_InitStructure);
  
  hal_I2C_SDA(1);
  hal_I2C_SCL(1);
}



static void hal_I2C_SDA(unsigned char bVal)
{
   if(bVal)
	 {
		 GPIO_SetBits(I2C_SDA_PORT,I2C_SDA_PIN);
	 }
	 else
	 {
		 GPIO_ResetBits(I2C_SDA_PORT,I2C_SDA_PIN);
	 }
}

static void hal_I2C_SCL(unsigned char bVal)
{ 
	 if(bVal)
	 {
		 GPIO_SetBits(I2C_SCL_PORT,I2C_SCL_PIN);
	 }
	 else
	 {
		 GPIO_ResetBits(I2C_SCL_PORT,I2C_SCL_PIN);
	 }
}


void hal_I2C_SDA_IO_Set(unsigned char IOMode)
{
	if(IOMode == 0)					//输出
	{
		GPIO_InitTypeDef  GPIO_InitStructure; 
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		GPIO_InitStructure.GPIO_Pin =   I2C_SDA_PIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;  
		GPIO_Init(I2C_SDA_PORT, &GPIO_InitStructure);
	}
	else if(IOMode == 1)			//输入	
	{
		GPIO_InitTypeDef  GPIO_InitStructure; 
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		GPIO_InitStructure.GPIO_Pin =   I2C_SDA_PIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  
		GPIO_Init(I2C_SDA_PORT, &GPIO_InitStructure);
	} 
}
 
unsigned char hal_I2C_SDA_INPUT(void)
{
	return GPIO_ReadInputDataBit(I2C_SDA_PORT, I2C_SDA_PIN);		
}

static void I2C_delay(unsigned short t)
{	

    unsigned short i=50,j,c; 
    c = t;
    for(j=0; j<c; j++)
    {
	   while(i) 
	   { 
			i--; 
	   } 
	}
}
 
 
static void I2C_Start(void)
{
	hal_I2C_SDA(1);
	I2C_delay(1);
	hal_I2C_SCL(1);
	I2C_delay(1);
	hal_I2C_SDA(0);
	I2C_delay(1);
}

 
static void I2C_Stop(void)
{
	hal_I2C_SDA(0);
	I2C_delay(1);
	hal_I2C_SCL(1);
	I2C_delay(1);
	hal_I2C_SDA(1);
	I2C_delay(1);
 
}

static void I2C_Ack(void)
{	
	hal_I2C_SCL(0);
	I2C_delay(1);
	hal_I2C_SDA(0);
	I2C_delay(1);
	hal_I2C_SCL(1);
	I2C_delay(1);
	hal_I2C_SCL(0);
	I2C_delay(1);
}

 
static void I2C_NoAck(void)
{	
	hal_I2C_SCL(0);
	I2C_delay(1);
	hal_I2C_SDA(1);
	I2C_delay(1);
	hal_I2C_SCL(1);
	I2C_delay(1);
	hal_I2C_SCL(0);
	I2C_delay(1);
}

 
static unsigned char I2C_WaitAck(void) 	
{
	hal_I2C_SDA(1);
	hal_I2C_SDA_IO_Set(1);	///配置为输入模式	 
	hal_I2C_SCL(1);
	I2C_delay(1);
	if(hal_I2C_SDA_INPUT())
	{
		return 0;
	}
	hal_I2C_SCL(0);
	hal_I2C_SDA_IO_Set(0);		 
	I2C_delay(1); 
	return 1;
}

 ////MSB  SendByte  0x96；  1001 0110 & 1000 0000
 //                         0001 1100
static void I2C_SendByte(unsigned char SendByte) 
{
  unsigned char i=0;
	unsigned char temp;
	temp = SendByte;
	for(i=0;i<8;i++)
	{
		hal_I2C_SCL(0);
		I2C_delay(1);
		if(temp&0x80)
		{
			hal_I2C_SDA(1);
		}
		else 
		{
			hal_I2C_SDA(0);
		}
		I2C_delay(1);
		hal_I2C_SCL(1);
		I2C_delay(1);
		temp<<=1;
	}
	hal_I2C_SCL(0);
	I2C_delay(1);
	hal_I2C_SDA(1);
	I2C_delay(1);
}

//1001 0110  96H
//ReceiveByte
//0000 0000 |= 0000 0001     0000 0001
//0000 0010 
//0000 0100      
//1001 0110
static unsigned char I2C_ReceiveByte(void)  
{ 
	unsigned char i;
	unsigned char ReceiveByte=0;
	
	hal_I2C_SCL(0);
	I2C_delay(1);
	hal_I2C_SDA(1);
	hal_I2C_SDA_IO_Set(1);		 //SDA设置成输入
	for(i=0; i<8; i++)
	{
		ReceiveByte <<= 1;
		I2C_delay(1);
		hal_I2C_SCL(1);
		I2C_delay(1);
		if(hal_I2C_SDA_INPUT())
		{
			ReceiveByte|=0x01;
		}
		hal_I2C_SCL(0);	
	}
	hal_I2C_SDA_IO_Set(0);		//SDA设置成输出
	I2C_delay(1);
	return ReceiveByte;
}
 

//连续读多个字节
// 0X1234
void I2C_Read(unsigned short address,unsigned char *pBuffer, unsigned short len)
{
	unsigned short length;
	length = len;
	I2C_Start();
	I2C_SendByte(0xA0);
	I2C_WaitAck();

	I2C_SendByte((address>>8)&0xFF);
	I2C_WaitAck();

	I2C_SendByte(address&0xFF);
	I2C_WaitAck();

	I2C_Start();
	I2C_SendByte(0xA1);
	I2C_WaitAck();
	
	//dat = I2C_ReceiveByte();
	while(length)
	{
		*pBuffer = I2C_ReceiveByte();
		if(length == 1)
			I2C_NoAck();
		else 
			I2C_Ack(); 
		pBuffer++;
		length--;
	}
	I2C_Stop();
	 
}

//页写函数,有自动翻页功能,24C64一页32Byte,num最大可写65523个字节
#define EEPROM_PAGE_SIZE 64

void I2C_PageWrite(unsigned short address,unsigned char *pDat, unsigned short num)
{
	unsigned char *pBuffer,j;
	unsigned short len,i,page,remainder,addr,temp;
	pBuffer = pDat;
	len = num;		
	addr = address;
	temp = 0;
	
	if(addr%EEPROM_PAGE_SIZE)	//判断要写的地址    60    //64
	{   //     64- 60 =  4    2
		temp = EEPROM_PAGE_SIZE-(addr%EEPROM_PAGE_SIZE);	//计算出当前地址还差多少字节满1页
		if(len<=temp)
		{
			temp = len;
		}
	}
	
	//先填满写入地址页
	if(temp)
	{
		I2C_Start();
		I2C_SendByte(0xA0);
		I2C_WaitAck();
		
		I2C_SendByte((addr>>8)&0xFF);
		I2C_WaitAck();
		
		I2C_SendByte(addr&0xFF);
		I2C_WaitAck();
		for(j=0; j<temp; j++)		 
		{
			I2C_SendByte(pBuffer[j]);
			I2C_WaitAck();	
		}
		I2C_Stop();
		I2C_delay(20000);	
	}
	len -= temp;			
	addr += temp;			//地址加上已经写入的字节
 
	page = len/EEPROM_PAGE_SIZE;	   //  1		
	remainder = len%EEPROM_PAGE_SIZE;  //  32 
	for(i=0; i<page; i++)		
	{
		I2C_Start();
		I2C_SendByte(0xA0);
		I2C_WaitAck();
		
		I2C_SendByte((addr>>8)&0xFF);
		I2C_WaitAck();
		
		I2C_SendByte(addr&0xFF);
		I2C_WaitAck();
		for(j=0;j<EEPROM_PAGE_SIZE;j++)
		{
			I2C_SendByte(pBuffer[temp+j]);
			I2C_WaitAck();
		}
		I2C_Stop();
		addr += EEPROM_PAGE_SIZE;
		temp += EEPROM_PAGE_SIZE;
		I2C_delay(20000);		
	}
	
	if(remainder)
	{
		I2C_Start();
		I2C_SendByte(0xA0);
		I2C_WaitAck();
		
		I2C_SendByte((addr>>8)&0xFF);
		I2C_WaitAck();
		
		I2C_SendByte(addr&0xFF);
		I2C_WaitAck();
		for(j=0; j<remainder; j++)		 
		{
			I2C_SendByte(pBuffer[temp+j]);
			I2C_WaitAck();	
		}
		I2C_Stop();
		I2C_delay(20000);		
	}
}

static void hal_eeprom_test(void)
{
	unsigned char eepromTestWriteBuffer[500];
	unsigned char eepromTestReadBuffer[500];
	unsigned short i;
	I2C_Read(250,eepromTestReadBuffer,500);
	
  for(i=0;i<500;i++)
	{
		eepromTestWriteBuffer[i] = i;
		eepromTestReadBuffer[i]=0xff;
	}
	I2C_PageWrite(250,eepromTestWriteBuffer,500);	
	I2C_Read(250,eepromTestReadBuffer,500);
}

 void hal_eepromInit(void)
{
	hal_I2CConfig();
	hal_eeprom_test();
}







