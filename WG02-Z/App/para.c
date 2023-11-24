#include "para.h"

unsigned char STM32_UID[12];

void ParaInit(void)
{
    unsigned char i;
	for(i=0; i<12; i++)
	{
		STM32_UID[i] = *((unsigned char *)(STM32_UID_ADDR+i));
	}
}

