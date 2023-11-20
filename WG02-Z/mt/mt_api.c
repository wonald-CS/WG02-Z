#include "mt_api.h"

const unsigned short wCRCTalbeAbs[] =
{
	0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401, 0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400,
};

 
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


//获取AT指令返回信息的长度
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


/*******************************************************************************************
*@description:查找是否有字符串相同
*@param[in]：*pdata：缓存数组
*@return：有相同字符串，则返回字符串相同的起始下标。若无，返回0xff
*@others：
********************************************************************************************/
unsigned int SeekSrting(unsigned char *str1,unsigned char *str2,unsigned int st1long)
{
	unsigned int i,log,relog,zi;
	unsigned int st2len;
	st2len = GetAtRspStrLen(str2);
	if(st1long >= st2len)
	{
		log = st1long - st2len+1;////比较的总的次数 
	}
	else{
		return 0xff;
	}
		
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
				else{
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
