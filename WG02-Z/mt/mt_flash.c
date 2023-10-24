#include "hal_flash.h"
#include "mt_flash.h"


unsigned char mt_SpiFlashReadSR(void)   
{  
	unsigned char byte=0;   
	hal_spi2CSDrive(0);                               
	hal_spi2ReadWriteByte(W25X_ReadStatusReg);    
	byte=hal_spi2ReadWriteByte(0Xff);            
	hal_spi2CSDrive(1);                             
	return byte;   
} 


void mt_SpiFlashWriteSR(unsigned char sr)   
{   
	hal_spi2CSDrive(0);                           
	hal_spi2ReadWriteByte(W25X_WriteStatusReg);   
	hal_spi2ReadWriteByte(sr);           
	hal_spi2CSDrive(1);                                
}



void mt_SpiFlashWaitBusy(void)   
{   
	while((mt_SpiFlashReadSR()&0x01)==0x01);   
} 


void mt_SpiFlashWriteEnable(void)   
{
	hal_spi2CSDrive(0);                           
	hal_spi2ReadWriteByte(W25X_WriteEnable);     
	hal_spi2CSDrive(1);                                         	      
}


void mt_SpiFlashWriteDisable(void)   
{  
	hal_spi2CSDrive(0);                          
  	hal_spi2ReadWriteByte(W25X_WriteDisable);     
	hal_spi2CSDrive(1);    	      
} 	



void mt_SpiFlashEraseSector(unsigned int Dst_Addr)   
{   
	Dst_Addr*=4096;
	mt_SpiFlashWriteEnable();                
	mt_SpiFlashWaitBusy();   
	hal_spi2CSDrive(0);                              
	hal_spi2ReadWriteByte(W25X_SectorErase);      
	hal_spi2ReadWriteByte((unsigned char )((Dst_Addr)>>16));  
	hal_spi2ReadWriteByte((unsigned char )((Dst_Addr)>>8));   
	hal_spi2ReadWriteByte((unsigned char )Dst_Addr);  
	hal_spi2CSDrive(1);                             	      
	mt_SpiFlashWaitBusy();   				   

}


unsigned short mt_flashReadID(void)
{
	unsigned short Temp = 0;
	hal_spi2CSDrive(0);               
	hal_spi2ReadWriteByte(W25X_ManufactDeviceID);
	hal_spi2ReadWriteByte(0x00);
	hal_spi2ReadWriteByte(0x00);
	hal_spi2ReadWriteByte(0x00);


	Temp |= hal_spi2ReadWriteByte(0xFF) << 8;
	Temp |= hal_spi2ReadWriteByte(0xFF);

	hal_spi2CSDrive(1);              

	return Temp;
}  



void mt_SpiFlashRead(unsigned char* pBuffer,unsigned int ReadAddr,unsigned short NumByteToRead)   
{ 
 	unsigned short i;  

	hal_spi2CSDrive(0);                            
    hal_spi2ReadWriteByte(W25X_ReadData);        
    hal_spi2ReadWriteByte((unsigned char)((ReadAddr)>>16));  
    hal_spi2ReadWriteByte((unsigned char)((ReadAddr)>>8));   
    hal_spi2ReadWriteByte((unsigned char)ReadAddr);   

    for(i=0;i<NumByteToRead;i++)
	{ 
        pBuffer[i]=hal_spi2ReadWriteByte(0XFF);   
    }

	hal_spi2CSDrive(1);                           	      
} 


void mt_SpiFlashWritePage(unsigned char* pBuffer,unsigned int WriteAddr,unsigned short NumByteToWrite)
{
 	unsigned short i;  
    mt_SpiFlashWriteEnable();                 		
	hal_spi2CSDrive(0);                          
    hal_spi2ReadWriteByte(W25X_PageProgram);      
    hal_spi2ReadWriteByte((unsigned char)((WriteAddr)>>16)); 
    hal_spi2ReadWriteByte((unsigned char)((WriteAddr)>>8));   
    hal_spi2ReadWriteByte((unsigned char)WriteAddr);   
    for(i=0;i<NumByteToWrite;i++)
	{
		hal_spi2ReadWriteByte(pBuffer[i]);		  
	} 
	hal_spi2CSDrive(1);                          
	mt_SpiFlashWaitBusy();					  		 
} 



void mt_SpiFlashWriteSector(unsigned char* pBuffer,unsigned int WriteAddr,unsigned short NumByteToWrite)   
{ 		
		 
	unsigned short pageremain;	   
	pageremain = 256-WriteAddr%256;  	    
	if(NumByteToWrite <= pageremain) 
	{
		pageremain = NumByteToWrite;
	}
	while(1)
	{	   
		mt_SpiFlashWritePage(pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)break;
	 	else //NumByteToWrite>pageremain
		{
			pBuffer += pageremain;     			
			WriteAddr += pageremain;			

			NumByteToWrite -= pageremain;			  
			if(NumByteToWrite>256)
			{ 
		 	 	 pageremain = 256; 
			}	
			else 
			{
				pageremain=NumByteToWrite; 	  
			}
		}
	};	    
} 

	   
void mt_SpiFlashWrite(unsigned char* pBuffer,unsigned int WriteAddr,unsigned short NumByteToWrite)   
{ 
	
	unsigned int secpos;
	unsigned short secoff;
	unsigned short secremain;	   
 	unsigned short i; 
	unsigned char SPI_FLASH_BUF[4096];   

	secpos=WriteAddr/4096;
	secoff=WriteAddr%4096;
	secremain=4096-secoff;

	if(NumByteToWrite<=secremain)
	{
		secremain=NumByteToWrite;
	}
	
	while(1) 
	{	
		mt_SpiFlashRead(SPI_FLASH_BUF,secpos*4096,4096);
		for(i=0;i<secremain;i++)
		{
			if(SPI_FLASH_BUF[secoff+i]!=0XFF)break;
		}
		if(i<secremain)
		{
			mt_SpiFlashEraseSector(secpos);
			for(i=0;i<secremain;i++)	  
			{
				SPI_FLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			mt_SpiFlashWriteSector(SPI_FLASH_BUF,secpos*4096,4096);

		}else 
		{
			mt_SpiFlashWriteSector(pBuffer,WriteAddr,secremain);
		}
					   
		if(NumByteToWrite==secremain)break;
		else
		{
			secpos++;
			secoff=0;

		   	pBuffer+=secremain; 
			WriteAddr+=secremain;
		   	NumByteToWrite-=secremain;				
			if(NumByteToWrite>4096)secremain=4096;	
			else secremain=NumByteToWrite;			
		}	 
	};	 	 
}

/*
unsigned char falshtest[6000];
void mt_flash_test(void)
{
	unsigned int i;
	unsigned int falshdadrx;
	falshdadrx = 4000;
	for(i=0;i< 6000;i++)
	{
		falshtest[i] = i;
  	}		
	mt_SpiFlashWrite(&falshtest[0],falshdadrx,6000);
	for(i=0;i< 6000;i++)
	{
		falshtest[i] = 0;
	}	
 	mt_SpiFlashRead(&falshtest[0],falshdadrx,6000);
}
*/


void mt_flashInit(void)
{
	hal_spi2Init(); 
 	mt_flashReadID();
	//mt_flash_test();
}






