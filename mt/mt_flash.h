#ifndef _MT_FLASH_H
#define _MT_FLASH_H

#define FLASH_PAGE_SIZE       4096
  
	
#define W25X_WriteEnable		  0x06 
#define W25X_ReadStatusReg	  0x05 
#define W25X_ReadData	        0x03 
#define W25X_PageProgram		  0x02 
#define W25X_SectorErase		  0x20 
#define W25X_ManufactDeviceID	   0x90 

void mt_flashInit(void);
void mt_flash_SaveDat(unsigned int addr,unsigned char *p,unsigned char lon);
void mt_flashRead(unsigned char *pBuffer,unsigned int ReadAddr,unsigned int NumByteToRead);
#endif






