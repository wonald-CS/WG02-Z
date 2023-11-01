#include "stm32F10x.h"
#include "hal_eeprom.h"


static void hal_I2CConfig(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure; 
  /* Configure I2C2 pins: PB8->SCL and PB9->SDA */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  GPIO_InitStructure.GPIO_Pin =  I2C_SCL_PIN | I2C_SDA_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;  
  GPIO_Init(I2C_SCL_PORT, &GPIO_InitStructure);
  
  I2C_SCL_1();
  I2C_SDA_1();
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
	}else if(IOMode == 1)			//输入	
	{
		GPIO_InitTypeDef  GPIO_InitStructure; 
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		GPIO_InitStructure.GPIO_Pin =   I2C_SDA_PIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  
		GPIO_Init(I2C_SDA_PORT, &GPIO_InitStructure);
	} 
}


/*
*********************************************************************************************************
*	函 数 名: I2C_delay
*	功能说明: I2C总线位延迟
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void I2C_delay(unsigned short t)
{	
   unsigned short i=50,j; 
   for(j=0; j<t; j++)
   {
	   while(i) 
	   { 
            i--; 
	   } 
	}
}


/*
*********************************************************************************************************
*	函 数 名: i2c_Start
*	功能说明: CPU发起I2C总线启动信号
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void I2C_Start(void)
{
	/* 当SCL高电平时，SDA出现一个下跳沿表示I2C总线启动信号 */
	I2C_SDA_1();
	I2C_SCL_1();
	I2C_delay(1);
	I2C_SDA_0();
	I2C_delay(1);
	I2C_SCL_0();
	I2C_delay(1);
}
 
 
 
/*
*********************************************************************************************************
*	函 数 名: i2c_Start
*	功能说明: CPU发起I2C总线停止信号
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void I2C_Stop(void)
{
	/* 当SCL高电平时，SDA出现一个上跳沿表示I2C总线停止信号 */
	I2C_SDA_0();
	I2C_SCL_1();
	I2C_delay(1);
	I2C_SDA_1();
	I2C_delay(1);
}


/*
*********************************************************************************************************
*	函 数 名: i2c_NAck
*	功能说明: CPU产生1个ACK信号
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void I2C_Ack(void)
{	
	I2C_SCL_0();
	I2C_delay(1);
	I2C_SDA_0();
	I2C_delay(1);
	I2C_SCL_1();
	I2C_delay(1);
	I2C_SCL_0();
	I2C_delay(1);
}

 
static void I2C_NoAck(void)
{	
	I2C_SCL_0();
	I2C_delay(1);
	I2C_SDA_1();
	I2C_delay(1);
	I2C_SCL_1();
	I2C_delay(1);
	I2C_SCL_0();
	I2C_delay(1);
}

/*
*********************************************************************************************************
*	函 数 名: I2C_Wait_Ack
*	功能说明: 等待EEprom应答信号
*	形    参:  无
*	返 回 值: 1，接收应答失败，IIC直接退出/0，接收应答成功，什么都不做
*********************************************************************************************************
*/
unsigned char I2C_WaitAck(void)
{
	I2C_SDA_1();
	hal_I2C_SDA_IO_Set(1);		 
	I2C_SCL_1();
	I2C_delay(1);
	if(I2C_SDA_READ())
	{
		return 0;
	}
	I2C_SCL_0();
	hal_I2C_SDA_IO_Set(0);		 
	I2C_delay(1); 
	return 1;

}


/*
*********************************************************************************************************
*	函 数 名: I2C_SendByte
*	功能说明: 发送字节
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void I2C_SendByte(unsigned char SendByte) 
{
    unsigned char i = 0;
	unsigned char temp;
	temp = SendByte;
	for(i=0;i<8;i++)
	{
		I2C_SCL_0();
		I2C_delay(1);
		if(temp & 0x80)
		{
			I2C_SDA_1();
		}else 
		{
			I2C_SDA_0();
		}
		I2C_delay(1);
		I2C_SCL_1();
		I2C_delay(1);
		temp <<= 1;
	}
    
	I2C_SCL_0();
	I2C_delay(1);
    I2C_SDA_1();
	I2C_delay(1);
}


/*
*********************************************************************************************************
*	函 数 名: I2C_ReceiveByte
*	功能说明: 接收eeprom发送的字节
*	形    参:  无
*	返 回 值: 接收到的字节数据
*********************************************************************************************************
*/
static unsigned char I2C_ReceiveByte(void)  
{ 
	unsigned char i;
	unsigned char ReceiveByte=0;
	
	I2C_SCL_0();
	I2C_delay(1);
    //SDA脚不用操作
	//I2C_SDA_1();
	hal_I2C_SDA_IO_Set(1);		 //SDA设置成输入
	for(i=0; i<8; i++)
	{
		ReceiveByte <<= 1;
		I2C_SCL_1();
		I2C_delay(1);
		if(I2C_SDA_READ())
		{
			ReceiveByte|=0x01;
		}
		I2C_SCL_0();
        I2C_delay(1);
		
	}
	hal_I2C_SDA_IO_Set(0);		//SDA设置成输出
	I2C_delay(1);
	return ReceiveByte;
}



/*
*********************************************************************************************************
*	函 数 名: I2C_Read
*	功能说明: 连续读多个字节
*	形    参:  address:操作地址，  *pBuffer：保存接收到数据的指针   ，  len:接收数据的长度
*	返 回 值: 无
*********************************************************************************************************
*/
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
	
	while(length)
	{
		*pBuffer = I2C_ReceiveByte();
		if(length == 1)I2C_NoAck();
		else I2C_Ack(); 
		pBuffer++;
		length--;
	}
	I2C_Stop();
	 
}



/**********************************************************************************************************
*	函 数 名: I2C_PageWrite
*	功能说明: 页写函数,有自动翻页功能,24C64一页32Byte,num最大可写65523个字节
*	形    参:  address:操作地址，  *pDat,：要写入eeprom的数据指针   ，  len:接收数据的长度
*	返 回 值: 无
**********************************************************************************************************/
void I2C_PageWrite(unsigned short address,unsigned char *pDat, unsigned short num)
{
    //例子： addr:60        num:100
	unsigned char *pBuffer,j;
	unsigned short len,i,page,remainder,addr,temp;
	pBuffer = pDat;
	len = num;		
	addr = address;
	temp = 0;

    //addr = 60;     addr%64 == 60;   64-60 == 4;  即该页还有4个字节可以写
  	if(addr%EEPROM_PAGE_SIZE)	//判断要写的地址
	{
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
	len -= temp;	        //剩下多少长度的字节未写	                    len = 100 - 4 = 96	
	addr += temp;			//地址加上已经写入的字节                        addr = 60 + 4 = 64
 
	page = len/EEPROM_PAGE_SIZE;	   //剩下的字节还需要写多少页           page = 96 / 64 = 1		
	remainder = len%EEPROM_PAGE_SIZE;  //最后写不满一页的字节               remainder = 96 % 64 =  32
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




void hal_eeprom_Init()
{
    hal_I2CConfig();
}
