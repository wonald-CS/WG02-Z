#include "hal_stmflash.h"
#include "mt_stmflash.h"

//读取指定地址的半字(16位数据)
//faddr:读地址(此地址必须为2的倍数!!)
//返回值:对应数据.
unsigned short mt_STMFLASH_ReadHalfWord(unsigned int faddr)
{
	return *(volatile unsigned short*)faddr; 
}
  
//不检查的写入
//WriteAddr:起始地址
//pBuffer:数据指针
//NumToWrite:半字(16位)数   
void mt_STMFLASH_Write_NoCheck(unsigned int WriteAddr,unsigned short *pBuffer,unsigned short NumToWrite)   
{ 			 		 
	unsigned short i;
	for(i=0;i<NumToWrite;i++)
	{
		
		hal_stmFLashProgramHalfWord(WriteAddr,pBuffer[i]);
	    WriteAddr+=2;//地址增加2.
	}  
} 
//从指定地址开始写入指定长度的数据
//WriteAddr:起始地址(此地址必须为2的倍数!!)
//pBuffer:数据指针
//NumToWrite:半字(16位)数(就是要写入的16位数据的个数.)  
//#if STM32_FLASH_SIZE<256
//#define STM_SECTOR_SIZE 1024 //字节
//#else 
#define STM_SECTOR_SIZE	2048
//#endif		 


void mt_STMFLASH_Write(unsigned int WriteAddr,unsigned short *pBuffer,unsigned short NumToWrite)	
{
	unsigned short STMFLASH_BUF[STM_SECTOR_SIZE/2];//最多是2K字节
	unsigned int secpos;	   //扇区地址
	unsigned short secoff;	   //扇区内偏移地址(16位字计算)
	unsigned short secremain; //扇区内剩余地址(16位字计算)	   
 	unsigned short i;    
	unsigned int offaddr;   //去掉0X08000000后的地址
	if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))
		return;//非法地址				
	hal_stmFlashUNLock();				//解锁
	//const u8 TEXT_Buffer[]={"STM32 FLASH TEST"};
	// STMFLASH_Write(0X08070000,(u16*)TEXT_Buffer,SIZE);
	offaddr=WriteAddr-STM32_FLASH_BASE;		//实际偏移地址.
	secpos=offaddr/STM_SECTOR_SIZE;			//扇区地址  0~127 for STM32F103RBT6
	secoff=(offaddr%STM_SECTOR_SIZE)/2;		//在扇区内的偏移(2个字节为基本单位.)
	secremain=STM_SECTOR_SIZE/2-secoff;		//扇区剩余空间大小   
	if(NumToWrite<=secremain)secremain=NumToWrite;//不大于该扇区范围
	while(1) 
	{	
		mt_STMFLASH_Read(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//读出整个扇区的内容
		for(i=0;i<secremain;i++)//校验数据
		{
			if(STMFLASH_BUF[secoff+i]!=0XFFFF)break;//需要擦除  	  
		}
		if(i<secremain)//需要擦除
		{
			hal_stmFLashEarsePage(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//擦除这个扇区
			for(i=0;i<secremain;i++)//复制
			{
				STMFLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			mt_STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//写入整个扇区  
		}else mt_STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);//写已经擦除了的,直接写入扇区剩余区间. 				   
		if(NumToWrite==secremain)break;//写入结束了
		else//写入未结束
		{
			secpos++;				//扇区地址增1
			secoff=0;				//偏移位置为0 	 
		   	pBuffer+=secremain;  	//指针偏移
			WriteAddr+=secremain;	//写地址偏移	   
		   	NumToWrite-=secremain;	//字节(16位)数递减
			if(NumToWrite>(STM_SECTOR_SIZE/2))secremain=STM_SECTOR_SIZE/2;//下一个扇区还是写不完
			else secremain=NumToWrite;//下一个扇区可以写完了
		}	 
	};	
	hal_stmFLashLock();//上锁
}


//从指定地址开始读出指定长度的数据
//ReadAddr:起始地址
//pBuffer:数据指针
//NumToWrite:半字(16位)数
void mt_STMFLASH_Read(unsigned int ReadAddr,unsigned short *pBuffer,unsigned short NumToRead)   	
{
	unsigned short i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=mt_STMFLASH_ReadHalfWord(ReadAddr);//读取2个字节.
		ReadAddr+=2;//偏移2个字节.	
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//WriteAddr:起始地址
//WriteData:要写入的数据
void Test_Write(unsigned int WriteAddr,unsigned short WriteData)   	
{
	unsigned short buffert[128],i;
	for(i=0;i<128;i++)
	{
		buffert[i] = WriteData;
	}
	mt_STMFLASH_Write(WriteAddr,&buffert[0],128);//写入一个字 
}







