#ifndef ____MT_API_H_
#define ____MT_API_H_

unsigned short mt_api_crc16(unsigned char *ptr, unsigned int len);
unsigned short mt_api_crc16_continuous(unsigned char *ptr, unsigned int len,unsigned short crc16); 

unsigned int SeekSrting(unsigned char *str1,unsigned char *str2,unsigned int st1long);
void StringhexToAsciiConversion(unsigned char *HexDat,unsigned char *ascDtt,unsigned int len);
void hexToAsciiConversion(unsigned char dat,unsigned char *Hdat,unsigned char *Ldat);
void asciiToHexConversion(unsigned char *asciiDat,unsigned short asciiLen,unsigned char *hexDat);
#endif
