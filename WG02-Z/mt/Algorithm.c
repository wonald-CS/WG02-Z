#include "Algorithm.h"

const unsigned short wCRCTalbeAbs[] =
{
	0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401, 0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400,
};

 
unsigned short Algorithm_crc16(unsigned char *ptr, unsigned int len) 
{
	unsigned short wCRC = 0xFFFF;
	unsigned short i;
	unsigned char chChar;
	unsigned char temp[2];
	for (i = 0; i < len; i++)
	{
		chChar = *ptr++;
		wCRC = wCRCTalbeAbs[(chChar ^ wCRC) & 15] ^ (wCRC >> 4);
		wCRC = wCRCTalbeAbs[((chChar >> 4) ^ wCRC) & 15] ^ (wCRC >> 4);
	}
	temp[0] = wCRC&0xFF; 
	temp[1] = (wCRC>>8)&0xFF;
	wCRC = (temp[0]<<8)|temp[1];
	return wCRC;
}
