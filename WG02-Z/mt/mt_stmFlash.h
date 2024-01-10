#ifndef _MT_STMFLASH_H_
#define _MT_STMFLASH_H_


//////////////////////////////////////////////////////////////////////////////////////////////////////

#define STM32_FLASH_SIZE 256 	 	
 
//////////////////////////////////////////////////////////////////////////////////////////////////////


#define STM32_FLASH_BASE 0x08000000 

void Test_Write(unsigned int WriteAddr,unsigned short WriteData);
unsigned short mt_STMFLASH_ReadHalfWord(unsigned int faddr);
void mt_STMFLASH_Write_NoCheck(unsigned int WriteAddr,unsigned short *pBuffer,unsigned short NumToWrite);
void mt_STMFLASH_Write(unsigned int WriteAddr,unsigned short *pBuffer,unsigned short NumToWrite);
void mt_STMFLASH_Read(unsigned int ReadAddr,unsigned short *pBuffer,unsigned short NumToRead);




#endif

