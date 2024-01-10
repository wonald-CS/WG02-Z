#include "string.h"
#include "app.h"
#include "os_system.h"
#include "hal_gotoapp.h"
#include "mt_flash.h"
#include "mt_stmflash.h"
#include "mt_tftlcd.h"


#define FIRMWARE_NEWVERSION          0xAA
#define FIRMWARE_NEWVERSION_UPOK     0xBB

typedef enum
{
	FLASHADDR_HAVE_NEWVER = 0, ///有新的固件版本   0xAA 有新的固件要升级   0xBB固件升级成功或无固件需要升级
	FLASHADDR_FIRMWARE_VER = 1,  //2个字节
	FLASHADDR_FIRMWARE_UPVER = 3, //2个字节	
	FLASHADDR_FIRMWARE_BYTESSIZE = 5,  //两个字节
	FLASHADDR_FIRMWARE_PACKSIZE = 9,  //两个字节
	FLASHADDR_CHECK_CRC = 11,     //2个字节
	FLASHADDR_FIRMWARE_START = 13,	
}en_falshaddr;

////
typedef struct
{
	unsigned char newflag;///新固件标志位
	unsigned char version[2]; ///当前固件版本号
	unsigned char upversion[2];
	unsigned char verbytesSize[4]; //固件的尺寸
	unsigned char verPackSize[2];  //固件包的大小
	unsigned char crc16[2];
}str_falshDataHead;//HEAD_FALSH;
///////////////////////////////////////////////////
str_falshDataHead *datahead;


unsigned int UpdateWriteFlashOffset;			//写升级数据到Flash的偏移地址
unsigned short UpdateFile_256_Uint_Number; 		//升级数据总大小除以256字节的个数
unsigned short UpdateFile_256_Uint_Remain;		//升级数据总大小除以256字节后的余数
unsigned short UpadePercentage;					//升级进度,百分比
unsigned int UpdateFileSize;					//升级文件字节数
unsigned int havedUpfalshDddr;
unsigned char UpdateDataEndFlag;				//升级数据下发结束标志	 

unsigned char Downversion[2]; 
 
void AppInit(void)
{
	unsigned char buffer[20];
	mt_flashRead(buffer,FLASHADDR_HAVE_NEWVER,13);
	datahead = (str_falshDataHead *)buffer;
	if(datahead ->newflag == FIRMWARE_NEWVERSION)//0xbb)//
	{
		havedUpfalshDddr = FLASHADDR_FIRMWARE_START;
		UpdateFileSize = datahead ->verbytesSize[0] << 24;
		UpdateFileSize |= (datahead ->verbytesSize[1] <<16);
		UpdateFileSize |= (datahead ->verbytesSize[2] << 8);
		UpdateFileSize |= datahead ->verbytesSize[3];
		
		Downversion[0] = datahead ->upversion[0];
		Downversion[1] = datahead ->upversion[1];
		/////////
		///固件升级 256为单位 升级的 
		///   1000  256 256 256 232
	    UpdateFile_256_Uint_Number = UpdateFileSize>>8;		//计算总共有多少组256字节数据，方便后面写入Flash
		UpdateFile_256_Uint_Remain = UpdateFileSize%256;	//计算出不到256字节的数据量
		
		UpdateWriteFlashOffset = 0;
		UpadePercentage = 0;	  ///升级的百分比
		LCD_ShowString(30,100,"                ",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
		LCD_ShowString(60,60,"Firmware Upgrade...",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_24,0);
		LCD_ShowString(130,120,"0%",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);	
 
	}
	else
		GotoApp();
}
///////
void AppProc(void)
{
	unsigned short STM32FlashBuff[128];
	unsigned char tData[256];//idx;
	unsigned short i,len;
	static unsigned short RcvData_256_Uint_Number=0;  //已经升级的固件的包的个数
//1000  256 256 256 232
	if(RcvData_256_Uint_Number < UpdateFile_256_Uint_Number)
	{////升级正包的数据
		mt_flashRead(tData,havedUpfalshDddr,256);//25Q64
		havedUpfalshDddr +=256;
		
		for(i=0; i<128; i++)
		{
			//把8位数据转换成16位,STM32 Flash是半字或字对齐
			STM32FlashBuff[i] = (tData[i*2+1]<<8)&0xFF00;
			STM32FlashBuff[i] |= tData[i*2]&0xFF;					
		}			
///////////C800 C900 CA00。。。800 C800
		mt_STMFLASH_Write((APP_FLASH_ADDR+UpdateWriteFlashOffset),STM32FlashBuff,128);		//数据写入STM32Flash
		UpdateWriteFlashOffset+=256; 
		
		RcvData_256_Uint_Number++;
		
		
		UpadePercentage = (UpdateWriteFlashOffset*1000)/UpdateFileSize;
		if(UpadePercentage < 10)
		{
			tData[0] = '0';
			tData[1] = '.';
			tData[2] = UpadePercentage+'0'; 
			tData[3] = '%'; 
			tData[4] = '\0';
		}
		else if(UpadePercentage < 100)
		{
			tData[0] = (UpadePercentage/10)+'0';
			tData[1] = '.';
			tData[2] = (UpadePercentage%10)+'0';
			tData[3] = '%';
			tData[4] = '\0';
		}
		else if(UpadePercentage < 1000)
		{
			tData[0] = (UpadePercentage/100)+'0';	
			tData[1] = ((UpadePercentage%100)/10)+'0';				
			tData[2] = '.';
			tData[3] = (UpadePercentage%10)+'0';
			tData[4] = '%';
			tData[5] = '\0';
		}
		else if(UpadePercentage >= 1000)
		{
			
			tData[0] = (UpadePercentage/1000)+'0';
			tData[1] = ((UpadePercentage%1000)/100)+'0';
			tData[1] = ((UpadePercentage%100)/10)+'0';
			tData[1] = '.';
			tData[2] = (UpadePercentage%10)+'0';
			tData[3] = '%';
			tData[4] = '\0';
		}
		LCD_ShowString(130,120,tData,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
	}
	else if(RcvData_256_Uint_Number == UpdateFile_256_Uint_Number)
	{////
		mt_flashRead(tData,havedUpfalshDddr,UpdateFile_256_Uint_Remain);  
		len = UpdateFile_256_Uint_Remain;
		if(len%2)		//判断剩余升级数据字节是否为双数,单数补1字节，保持半字对齐
		{///3
			tData[len-1] = 0xFF;
			len++;
		}
		len /=2;
		
		for(i=0; i<len; i++)
		{
			//把8位数据转换成16位,STM32 Flash是半字或字对齐
			STM32FlashBuff[i] = (tData[i*2+1]<<8)&0xFF00;
			STM32FlashBuff[i] |= tData[i*2]&0xFF;					
		}
		//
		mt_STMFLASH_Write(APP_FLASH_ADDR+UpdateWriteFlashOffset,STM32FlashBuff,len);		//数据写入STM32Flash
		UpdateWriteFlashOffset += UpdateFile_256_Uint_Remain;				
		
		UpadePercentage = (UpdateWriteFlashOffset*1000)/UpdateFileSize;

		if(UpadePercentage < 1000)
		{
			
			tData[0] = (UpadePercentage/100)+'0';	
			tData[1] = ((UpadePercentage%100)/10)+'0';				
			tData[2] = '.';
			tData[3] = (UpadePercentage%10)+'0';
			tData[4] = '%';
			tData[5] = '\0';
		}
		else if(UpadePercentage >= 1000)
		{
			
			tData[0] = (UpadePercentage/1000)+'0';
			tData[1] = ((UpadePercentage%1000)/100)+'0';
			tData[2] = ((UpadePercentage%100)/10)+'0';
			tData[3] = '.';
			tData[4] = (UpadePercentage%10)+'0';
			tData[5] = '%';
			tData[6] = '\0';
		}		
		LCD_ShowString(130,120,tData,HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
		 
		tData[0] = FIRMWARE_NEWVERSION_UPOK;  //固件升级成功或无固件需要升级; 
		tData[1] = Downversion[0]; 
		tData[2] = Downversion[1];
		mt_flash_SaveDat(FLASHADDR_HAVE_NEWVER,&tData[0],3);  ///更新 固件升级标志位			
		LCD_ShowString(0,120,"update successfully",HUE_LCD_FONT,HUE_LCD_BACK,FORTSIZE_32,0);
		GotoApp();
	}
	 
}


































