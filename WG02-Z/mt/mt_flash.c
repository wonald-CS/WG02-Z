#include "hal_flash.h"
#include "mt_flash.h"

void mt_flashEraseSector(unsigned int Dst_Addr);
void mt_flashWaitBusy(void);
void mt_flashWriteEnable(void);
//void mt_flash_test(void);

//获取制造商ID   FE16H
unsigned short mt_flashReadID(void)
{
	unsigned short Temp = 0;	  
	hal_spi2CSDrive(0); 			    
	hal_spi2ReadWriteByte(0x90);//????ID??	    
	hal_spi2ReadWriteByte(0x00); 	    
	hal_spi2ReadWriteByte(0x00); 	    
	hal_spi2ReadWriteByte(0x00); 	 			   
	Temp|=hal_spi2ReadWriteByte(0xFF)<<8;  //
	Temp|=hal_spi2ReadWriteByte(0xFF);	 
	hal_spi2CSDrive(1); 			    
	return Temp;
}  

void mt_flashInit(void)
{
 // static unsigned short produid;
	hal_spi2Init(); 
// 	produid = mt_flashReadID();
// 	mt_flash_test();
}


//pBuffer-读取数据存储地址,ReadAddr-Flash地址,NumByteToRead-读取字节数
void mt_flashRead(unsigned char *pBuffer,unsigned int ReadAddr,unsigned int NumByteToRead)   
{ 
	unsigned char  *pBuff;
	unsigned short i,num;  
	unsigned int RdAddr;
	RdAddr = ReadAddr;
	num = NumByteToRead;
	pBuff = pBuffer;
	hal_spi2CSDrive(0);                            //使能器件   
	hal_spi2ReadWriteByte(0x03);         //发送读取命令   -
//      00 12 34 56H
	hal_spi2ReadWriteByte((unsigned char )((RdAddr)>>16));  //发送24bit地址    
	hal_spi2ReadWriteByte((unsigned char )((RdAddr)>>8));   
	hal_spi2ReadWriteByte((unsigned char )RdAddr);   
	for(i=0;i<num;i++)
	{ 
		pBuff[i]=hal_spi2ReadWriteByte(0XFF);   //循环读数  
	}
	hal_spi2CSDrive(1);                             //取消片选     	      
}  


//SPI在一页(0~65535)内写入少于256个字节的数据
//在指定地址开始写入最大256字节的数据
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!	 
void mt_flashWritePage(unsigned char * pBuffer,unsigned int WriteAddr,unsigned short NumByteToWrite)
{
	unsigned char  *pBuff;
	unsigned short i,num;  
	unsigned int wAddr;
	pBuff = pBuffer;
	wAddr = WriteAddr;
	num = NumByteToWrite;
	mt_flashWriteEnable();                  //SET WEL 

	hal_spi2CSDrive(0);                              //使能器件   
	hal_spi2ReadWriteByte(W25X_PageProgram);      //发送写页命令   
	hal_spi2ReadWriteByte((unsigned char )((wAddr)>>16)); //发送24bit地址    
	hal_spi2ReadWriteByte((unsigned char )((wAddr)>>8));   
	hal_spi2ReadWriteByte((unsigned char )wAddr);   
	for(i=0;i<num;i++)
	  hal_spi2ReadWriteByte(pBuff[i]);//循环写数  
	hal_spi2CSDrive(1);   
	mt_flashWaitBusy();   //等待写入结束
} 
//SPI_FLASH写使能	
//将WEL置位   
void mt_flashWriteEnable(void)   
{
	hal_spi2CSDrive(0);                            //使能器件   
	hal_spi2ReadWriteByte(W25X_WriteEnable);      //发送写使能  0x06
	hal_spi2CSDrive(1);                           //取消片选     	      
} 

//读取SPI_FLASH的状态寄存器
//BIT7  6   5   4   3   2   1    0
//SPR   RV  TB BP2 BP1 BP0 WEL   
//SPR:默认0,状态寄存器保护位,配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护设置
//WEL:写使能锁定
//BUSY:忙标记位(1,忙;0,空闲)
//默认:0x00
unsigned char  mt_flashReadSR(void)   
{  
	unsigned char  byte=0;   
	hal_spi2CSDrive(0);                            //使能器件   
	hal_spi2ReadWriteByte(W25X_ReadStatusReg);    //发送读取状态寄存器命令    
	byte=hal_spi2ReadWriteByte(0Xff);             //读取一个字节  
	hal_spi2CSDrive(1);                             //取消片选     
	return byte;   
} 
//等待空闲
void mt_flashWaitBusy(void)   
{   
	while ((mt_flashReadSR()&0x01)==0x01);   // 等待BUSY位清空
}  

//FLASH 块写操作
//必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
//具有自动换页功能 
//在指定地址开始写入指定长度的数据,但是要确保地址不越界!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)
//CHECK OK
void mt_flashWrite_Secor(unsigned char * pBuffer,unsigned int WriteAddr,unsigned short NumByteToWrite)   
{ 			 		 
	unsigned char  *pBuff;  ////数据地址指针
	unsigned short num;  
	unsigned int wAddr;  ///写的起始地址
	unsigned short pageremain;	
	pBuff = pBuffer;
	num = NumByteToWrite;
	wAddr = WriteAddr;
	pageremain=256-wAddr%256; //单页剩余的字节数		 	    
	if(num<=pageremain)
		pageremain=num;//不大于256个字节
	while(1)
	{	   
		        mt_flashWritePage(pBuff,wAddr,pageremain);
			if(num==pageremain)
				break;//写入结束了
			else //NumByteToWrite>pageremain
			{
				pBuff+=pageremain;
				wAddr+=pageremain;	//200  56   100
				num-=pageremain;			  //减去已经写入了的字节数
				if(num>256)
					pageremain=256; //一次可以写入256个字节
				else 
					pageremain=num; 	  //不够256个字节了
			}		
	}	    
} 

//写SPI FLASH  
//在指定地址开始写入指定长度的数据
//该函数带擦除操作!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大256)  
void mt_flashWrite(unsigned char * pBuffer,unsigned int WriteAddr,unsigned short NumByteToWrite)   
 { 
		 unsigned char  SPI_FLASH_BUF[4096];
//	   unsigned char testaa[200];
		 unsigned char  *pBuff;
	          unsigned int secpos;        ///需要写的起始的扇区
	           unsigned short secoff;      ///写入到额起始扇区的 偏移地址
	           unsigned short secremain;	 ///第一个写入扇区需要写入的数据的个数     
		  unsigned short i,num;  

		 unsigned int wAddr;
		 pBuff = pBuffer;
		 wAddr = WriteAddr;
		 num = NumByteToWrite;  ////
		 secpos=wAddr/4096;//扇区地址        
		 secoff=wAddr%4096;//在扇区内的偏移
		 secremain=4096-secoff;//扇区剩余空间大小   
	   if(num<=secremain)  ///num  是需要写入数据的格式   如果需要写入的数据的个数小于本扇区剩余的个数
			 secremain=num;//不大于4096个字节   在同一个区里面写
		 while(1) 
		 {	
				 mt_flashRead(SPI_FLASH_BUF,secpos*4096,4096);//读出整个扇区的内容
				 mt_flashEraseSector(secpos);//擦除这个扇区
				 
				 for(i=0;i<secremain;i++)	   //复制
				 {
				        SPI_FLASH_BUF[i+secoff]=pBuff[i];
				 }
				mt_flashWrite_Secor(SPI_FLASH_BUF,secpos*4096,4096);//写入整个扇区 写已经擦除了的,直接写入扇区剩余区间. 				 

				 if(num==secremain)   ///需要写入的数据长度和 数据长度一致的话，
					 break;//写入结束了
				 
				 else//写入未结束
				 {
					 secpos++;//扇区地址增1
					 secoff=0;//偏移位置为0 	 

					 pBuff+=secremain;  //指针偏移
					 wAddr+=secremain;//写地址偏移	   
					 num-=secremain;				//字节数递减
					 if(num>4096)
						 secremain=4096;	//下一个扇区还是写不完
					 else 
						 secremain=num;			//下一个扇区可以写完了
				 }	 
		 }	 	 
 } 
 //擦除一个扇区
//Dst_Addr:扇区地址 0~511 for w25x16
//擦除一个扇区的最少时间:45ms,最大300ms
void mt_flashEraseSector(unsigned int Dst_Addr)   
{   
	unsigned int DstAddr;
	DstAddr = Dst_Addr;
	DstAddr*=4096;
	mt_flashWriteEnable();                  //SET WEL 	 
	mt_flashWaitBusy();   
	hal_spi2CSDrive(0);                              //使能器件   
	hal_spi2ReadWriteByte(W25X_SectorErase);      //发送扇区擦除指令 
	hal_spi2ReadWriteByte((unsigned char )((DstAddr)>>16));  //发送24bit地址    
	hal_spi2ReadWriteByte((unsigned char )((DstAddr)>>8));   
	hal_spi2ReadWriteByte((unsigned char )DstAddr);  
	hal_spi2CSDrive(1);                             //取消片选     	      
	mt_flashWaitBusy();   				   //等待擦除完成
}

/*unsigned char falshtest[6000];
void mt_flash_test(void)
{
	unsigned int i;
	unsigned int falshdadrx;
	falshdadrx = 4000;
	for(i=0;i< 6000;i++)
	{
		falshtest[i] = i;
  }		
	mt_flashWrite(&falshtest[0],falshdadrx,6000);
         for(i=0;i< 6000;i++)
	{
		falshtest[i] = 0;
       }	
  mt_flashRead(&falshtest[0],falshdadrx,6000);
}
*/

