#include "stm32f10x.h"
#include "hal_stmflash.h"

void hal_stmFLashProgramHalfWord(unsigned int Address, unsigned short Data)
{
	FLASH_ProgramHalfWord(Address,Data);
}
///
void hal_stmFLashLock(void)
{
	FLASH_Lock(); 
}

void hal_stmFlashUNLock(void)
{
	FLASH_Unlock();	
}

void hal_stmFLashEarsePage(unsigned int PageAddress)
{
	FLASH_ErasePage(PageAddress);
}





