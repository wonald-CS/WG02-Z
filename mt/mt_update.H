#ifndef ____MT_UPDATE_H_
#define ____MT_UPDATE_H_

#define FIRMWARE_NEWVERSION          0xAA
#define FIRMWARE_NEWVERSION_UPOK     0xBB
#define FIRMWARE_DEFAULT             0xCC


typedef enum
{
	FLASHADDR_HAVE_NEWVER = 0, ///���µĹ̼��汾   0xAA ���µĹ̼�Ҫ����   0xBB�̼������ɹ����޹̼���Ҫ����
	FLASHADDR_FIRMWARE_VER = 1,  //2���ֽ�
	FLASHADDR_FIRMWARE_UPVER = 3, //2���ֽ�	
	FLASHADDR_FIRMWARE_BYTESSIZE = 5,  //4���ֽ�
	FLASHADDR_FIRMWARE_PACKSIZE = 9,  //�����ֽ�
	FLASHADDR_CHECK_CRC = 11,     //2���ֽ�
	FLASHADDR_FIRMWARE_START = 13,	
}en_falshaddr;

typedef enum
{
	MT_UPDATE_STA_NO = 0,        //����������
	MT_UPDATE_STA_NEW_VER,       ///���µİ汾
	MT_UPDATE_STA_NOUP,         ///�̼�������	
	MT_UPDATE_STA_DOWN_FIRMWARE, ///��ʼ���ع̼�
	MT_UPDATE_STA_DOWN_N0,
	MT_UPDATE_STA_DOWN_FAIL,     //�̼�����ʧ��
    MT_UPDATE_STA_DOWN_OVER,     ///�������
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
	en_mtUpdateState UpdateFalg;          //0 û������������    1 ��ʾ���������µĹ̼�����   2 ��ʼ���ع̼�    3 �̼��������
    un_Version oldVer;   ///ϵͳ֮ǰ�İ汾
	un_Version upVer;    ///ϵͳ�¹̼��İ汾
	un_firewareSiz fireSize;  ///�̼���С  ��λ:�ֽ�
	un_firewarePockSiz firePack;  ///�̼�����ĸ���
	unsigned char crc16[2];      //�̼�������У��ֵ
	unsigned short HaveDowingPack; // �Ѿ����ش�̼����Ĵ�С
	unsigned int HaveDowingByte;   // �Ѿ������Ĺ̼����ֽڵĴ�С	
}str_FirmwareUpdate;////�̼�������





extern str_FirmwareUpdate  Firmware_Update;

typedef struct
{
	unsigned char dataID;//����֡ID
	unsigned char uPType;//�̼�������
	unsigned char ver[2];//un_Version downVer;  
	unsigned char state; //״̬  0�ɹ�
	unsigned char packnum[2];//un_firewarePockSiz packSerialNum;  ///�̼��������
	unsigned char datlen;     ///��Ч���ݵĳ���
	unsigned char dat[102];  ///  ��Ч����+ У�� 
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

