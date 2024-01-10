#ifndef ____MT_UPDATE_H_
#define ____MT_UPDATE_H_

#define FIRMWARE_NEWVERSION          0xAA
#define FIRMWARE_NEWVERSION_UPOK     0xBB
#define FIRMWARE_DEFAULT             0xCC


typedef enum
{
	FLASHADDR_HAVE_NEWVER = 0, ///有新的固件版本   0xAA 有新的固件要升级   0xBB固件升级成功或无固件需要升级
	FLASHADDR_FIRMWARE_VER = 1,  //2个字节
	FLASHADDR_FIRMWARE_UPVER = 3, //2个字节	
	FLASHADDR_FIRMWARE_BYTESSIZE = 5,  //4个字节
	FLASHADDR_FIRMWARE_PACKSIZE = 9,  //两个字节
	FLASHADDR_CHECK_CRC = 11,     //2个字节
	FLASHADDR_FIRMWARE_START = 13,	
}en_falshaddr;

typedef enum
{
	MT_UPDATE_STA_NO = 0,        //无升级任务
	MT_UPDATE_STA_NEW_VER,       ///有新的版本
	MT_UPDATE_STA_NOUP,         ///固件不升级	
	MT_UPDATE_STA_DOWN_FIRMWARE, ///开始下载固件
	MT_UPDATE_STA_DOWN_N0,
	MT_UPDATE_STA_DOWN_FAIL,     //固件下载失败
    MT_UPDATE_STA_DOWN_OVER,     ///升级完成
}en_mtUpdateState;

typedef struct
{
	unsigned char version[2];		
}un_Version;

typedef union  
{
	unsigned int bytesizeT;
	unsigned char bytesize[4];		
}un_firewareSiz;

typedef union  
{
	unsigned short packSizeT;
	unsigned char packSize[2];		
}un_firewarePockSiz;

typedef struct
{
	en_mtUpdateState UpdateFalg;          //0 没有升级的任务    1 表示服务器有新的固件推送   2 开始下载固件    3 固件下载完成
    un_Version oldVer;   ///系统之前的版本
	un_Version upVer;    ///系统新固件的版本
	un_firewareSiz fireSize;  ///固件大小  单位:字节
	un_firewarePockSiz firePack;  ///固件封包的个数
	unsigned char crc16[2];      //固件的整体校验值
	unsigned short HaveDowingPack; // 已经下载大固件包的大小
	unsigned int HaveDowingByte;   // 已经升级的固件的字节的大小	
}str_FirmwareUpdate;////固件的属性





extern str_FirmwareUpdate  Firmware_Update;

typedef struct
{
	unsigned char dataID;//数据帧ID
	unsigned char uPType;//固件的类型
	unsigned char ver[2];//un_Version downVer;  
	unsigned char state; //状态  0成功
	unsigned char packnum[2];//un_firewarePockSiz packSerialNum;  ///固件包的序号
	unsigned char datlen;     ///有效数据的长度
	unsigned char dat[102];  ///  有效数据+ 校验 
}str_FrimwareDownDat;


void mt_update_Init(void);
//void mt_update_Init(unsigned char(*getver)(unsigned char i));
void mt_update_Fireware(str_FirmwareUpdate date);
unsigned char mt_update_GetUpState(void);
void mt_update_SetUpState(en_mtUpdateState flag);
void mt_updata_SetFirmwareOldVersion(unsigned char i,unsigned char val);
void mt_update_upingStart(void);
unsigned char mt_update_upingPro(void(*SaveDat)(unsigned int addr,unsigned char *p,unsigned char lon),unsigned char *pdat,void(*falshRead)(unsigned char *pBuffer,unsigned int ReadAddr,unsigned int NumByteToRead));
unsigned short mt_update_pro(unsigned char comTyp,void (*getPack)(unsigned char comType,unsigned short packNo,unsigned char *ver));
//unsigned short mt_update_pro(void (*getPack)(unsigned short packNo,unsigned char *ver));
#endif


