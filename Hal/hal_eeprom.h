#ifndef _HAL_EEPROM_H
#define _HAL_EEPROM_H


void hal_eepromInit(void);
void I2C_Read(unsigned short address,unsigned char *pBuffer, unsigned short len);
void I2C_PageWrite(unsigned short address,unsigned char *pDat, unsigned short num);

#endif

