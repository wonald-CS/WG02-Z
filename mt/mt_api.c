#include "mt_api.h"

const unsigned short wCRCTalbeAbs[] =
{
	0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401, 0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400,
};
//wCRC = 0xffff;   a
// 01    a   02  b   03  c
//   c  04  d  05  e  06 f   
// 10000   10000  10000  
unsigned short mt_api_crc16(unsigned char *ptr, unsigned int len) 
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
///////////////////////////////////////////
unsigned short mt_api_crc16_continuous(unsigned char *ptr, unsigned int len,unsigned short crc16) 
{
//	unsigned short tt;
//	unsigned int i;
//	unsigned char j;
  unsigned short i;
	unsigned char temp[2];
	unsigned char chChar;
	unsigned short wCRC;
	 wCRC = crc16;
	
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


//////获取AT指令返回信息的长度
static unsigned int GetAtRspStrLen(unsigned char *p)
{
	unsigned int len;
	len=0;
	while(*p!=0)
	{
		len++;
		if(len>50)
		{
			break;
		}
		p++;
	}
	return len;
}
// 在字符串1中查找字符串2， 查看子字符串2是不是字符串1 的子函数
// 在字符串1中没有找到字符串2 返回0xff；
//返回字符串相同的起始下标
unsigned int SeekSrting(unsigned char *str1,unsigned char *str2,unsigned int st1long)
{
	unsigned int i,log,relog,zi;
	unsigned int st2len;
	st2len = GetAtRspStrLen(str2);
	if(st1long >= st2len)
	{
		log = st1long - st2len+1;////比较的总的次数 
	}
	else
		return 0xff;
	relog = 0;     //用来记录 比较过程中 有多少个字母相同了 
	for(i=0;i<log;i++)
	{
		if(*str1 == *str2)
		{
			zi = 1;
			while(zi)
			{
				relog ++;
				if(relog == st2len)
				{
					return i;		//字符串相同的起始下标
				}	
				str1++;
				str2++;				 
				if(*str1 == *str2)
				{
				}
				else
				{
					str1-=relog;
					str2-=relog;
					relog = 0;
					zi = 0;
					break;
				}
				
			}
		}
		str1++;
	}
	return 0xff;
}
///////
///////
void StringhexToAsciiConversion(unsigned char *HexDat,unsigned char *ascDtt,unsigned int len)
{
	  unsigned int lenx;
	  unsigned char temp;
	  unsigned char datah,datal;
	  lenx = len;
		while(lenx)
		{
			lenx--;
			temp = *HexDat;
			if((temp & 0x0f) < 0x0A)
			{
				datal	= ('0' + (temp & 0x0f));
			}
			else
			{
				datal	= ('a'+(temp & 0x0f) - 10);
			}
			temp >>= 4;	
			if((temp & 0x0f) < 0x0A)
			{
				datah	= ('0' + (temp & 0x0f));
			}
			else
			{
				datah	= ('a' + (temp & 0x0f) - 10);
			}	
			*ascDtt = datah;
			ascDtt ++;
			*ascDtt = datal;
			ascDtt ++;
			
			HexDat ++;	
		}		
}

void hexToAsciiConversion(unsigned char dat,unsigned char *Hdat,unsigned char *Ldat)
{
		if((dat & 0x0f) < 0x0A)
		{
			*Ldat	= ('0' + (dat & 0x0f));
		}
		else
		{
			*Ldat	= ('A'+(dat & 0x0f) - 10);
		}
		
		dat >>= 4;	
		if((dat & 0x0f) < 0x0A)
		{
			*Hdat	= ('0' + (dat & 0x0f));
		}
		else
		{
			*Hdat	= ('A'+(dat & 0x0f) - 10);
		}			
}

void asciiToHexConversion(unsigned char *asciiDat,unsigned short asciiLen,unsigned char *hexDat)
{
	unsigned char *pdata,len;
	pdata = asciiDat;
	len = asciiLen;
	while(len)
	{
		*hexDat = 0;
		if((*pdata >= '0') && (*pdata <= '9'))
		{
				*hexDat = (*pdata - '0') << 4;
		}
		else if((*pdata >= 'A') && (*pdata <= 'F')) 
		{
				*hexDat = (*pdata - 'A' + 10) << 4;
		}
		else if((*pdata >= 'a') && (*pdata <= 'f'))
		{
				*hexDat = (*pdata - 'a' + 10) << 4;
		}	
		pdata ++;
		
		if((*pdata >= '0') && (*pdata <= '9'))
		{
				*hexDat += (*pdata - '0');
		}
		else if((*pdata >= 'A') && (*pdata <= 'F')) 
		{
				*hexDat += (*pdata - 'A' + 10);
		}
		else if((*pdata >= 'a') && (*pdata <= 'f'))
		{
				*hexDat += (*pdata - 'a' + 10);
		}		
		
		pdata ++;
		hexDat ++;
		len -= 2;
	}
}




