#ifndef _MT_FLASH_H
#define _MT_FLASH_H

#define FLASH_PAGE_SIZE       4096

//W25X64读写
#define FLASH_ID 0XEF16
//指令表
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg		0x05 
#define W25X_WriteStatusReg		0x01 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 

void mt_flashInit(void);
void mt_SpiFlashRead(unsigned char* pBuffer,unsigned int ReadAddr,unsigned short NumByteToRead);
void mt_SpiFlashWrite(unsigned char* pBuffer,unsigned int WriteAddr,unsigned short NumByteToWrite);

#endif






