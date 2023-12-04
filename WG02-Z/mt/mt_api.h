#ifndef ____MT_API_H_
#define ____MT_API_H_

//´óÐ¡¶Ë×ª»»
#define SWAP16(x)  ((((unsigned short)x & 0x00FF) << 8 ) | (((unsigned short)x & 0xFF00) >> 8))


unsigned short mt_api_crc16(unsigned char *ptr, unsigned int len);
unsigned int SeekSrting(unsigned char *str1,unsigned char *str2,unsigned int st1long);
void StringhexToAsciiConversion(unsigned char *HexDat,unsigned char *ascDtt,unsigned int len);
void hexToAsciiConversion(unsigned char dat,unsigned char *Hdat,unsigned char *Ldat);
void asciiToHexConversion(unsigned char *asciiDat,unsigned char *hexDat,unsigned short asciiLen);
#endif
