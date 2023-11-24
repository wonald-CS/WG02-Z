#ifndef ____PARA_H_
#define ____PARA_H_

#define STM32_UID_ADDR		0x1ffff7e8

extern unsigned char STM32_UID[12];
#define MT_GET_MCU_UID(i)	(STM32_UID[i])

void ParaInit(void);



#endif
