#ifndef _HAL_STMFLASH_H_
#define _HAL_STMFLASH_H_

void hal_stmFLashProgramHalfWord(unsigned int Address, unsigned short Data);
void hal_stmFLashLock(void);
void hal_stmFlashUNLock(void);
void hal_stmFLashEarsePage(unsigned int PageAddress);


#endif



