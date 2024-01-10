#include "mt_update.h"
#include "mt_api.h"



unsigned short allcrc16,allcrc16last;



str_FirmwareUpdate  Firmware_Update;

void mt_update_Init(void)
{
	Firmware_Update.UpdateFalg = MT_UPDATE_STA_NO;
}
//////////////////////////////////////////////////////////////////////

void mt_update_Fireware(str_FirmwareUpdate date)
{
	if(Firmware_Update.UpdateFalg == MT_UPDATE_STA_NO)
	{
	    Firmware_Update.upVer.version[0] = date.upVer.version[0];
		Firmware_Update.upVer.version[1] = date.upVer.version[1];
		Firmware_Update.fireSize.bytesize[0] = date.fireSize.bytesize[0];
		Firmware_Update.fireSize.bytesize[1] = date.fireSize.bytesize[1];
		Firmware_Update.fireSize.bytesize[2] = date.fireSize.bytesize[2];
		Firmware_Update.fireSize.bytesize[3] = date.fireSize.bytesize[3];
		Firmware_Update.firePack.packSize[0] = date.firePack.packSize[0];
		Firmware_Update.firePack.packSize[1] = date.firePack.packSize[1];
		Firmware_Update.crc16[0] = date.crc16[0];
		Firmware_Update.crc16[1] = date.crc16[1];	
		allcrc16 = 0;
		allcrc16last = 0xffff;
		if(((Firmware_Update.upVer.version[0]<<8) + Firmware_Update.upVer.version[1]) > ((Firmware_Update.oldVer.version[0]<<8) + Firmware_Update.oldVer.version[1]))
		{
			Firmware_Update.UpdateFalg = MT_UPDATE_STA_NEW_VER;	
			Firmware_Update.HaveDowingPack = 0;
			Firmware_Update.HaveDowingByte = 0;		
		}	
	}
}
////
void mt_updata_SetFirmwareOldVersion(unsigned char i,unsigned char val)
{
	Firmware_Update.oldVer.version[i] = val;
}
////
unsigned char mt_update_GetUpState(void)
{
	 return Firmware_Update.UpdateFalg;
}

void mt_update_SetUpState(en_mtUpdateState flag)
{
	Firmware_Update.UpdateFalg = flag;
}

////��ʼ�����̼�
void mt_update_upingStart(void)
{
	Firmware_Update.UpdateFalg = MT_UPDATE_STA_DOWN_FIRMWARE;
	Firmware_Update.HaveDowingPack = 0;
	Firmware_Update.HaveDowingByte = 0;
	allcrc16 = 0;
	allcrc16last = 0xffff;
}
//////////
unsigned char mt_update_upingPro(void(*SaveDat)(unsigned int addr,unsigned char *p,unsigned char lon),unsigned char *pdat,void(*falshRead)(unsigned char *pBuffer,unsigned int ReadAddr,unsigned int NumByteToRead))
{
	unsigned short datacrc16,datacrc16t;
	unsigned char *dat;
	unsigned int addrt;
	unsigned char len;
	unsigned char buffert[13];
	unsigned short pocknum;
	str_FrimwareDownDat *downdat;
	
	dat = pdat;
	if(Firmware_Update.UpdateFalg == MT_UPDATE_STA_DOWN_FIRMWARE)
	{//
        downdat = (str_FrimwareDownDat *)dat;
		if((downdat->uPType == 0) &&(downdat->state == 0))
		{///�����̼�������  0   ��ȡ���ݵ�״̬ ��0
			 pocknum = ((downdat->packnum[0] << 8) | downdat->packnum[1]); 
			 if(pocknum == Firmware_Update.HaveDowingPack)
			 {///
				 datacrc16 = mt_api_crc16(&downdat->dat[0],(downdat->datlen));
				 
				 datacrc16t = (downdat->dat[downdat->datlen]<<8) | downdat->dat[downdat->datlen+1];
				 if(datacrc16t == datacrc16)			
				 {///CRC У��ɹ� 
					  allcrc16 = mt_api_crc16_continuous((&downdat->dat[0]),(downdat->datlen),allcrc16last);
					  allcrc16last = allcrc16;
					  addrt = Firmware_Update.HaveDowingByte +FLASHADDR_FIRMWARE_START;   ///�洢�̼��ĵ�ַ
					 ////���صĹ̼��洢�Ǵ�FLASH�ĵ�һ���ֽڿ�ʼ�洢		
					  SaveDat(addrt,&downdat->dat[0],downdat->datlen);   ///�������� FLASH 
					 
					 
						Firmware_Update.HaveDowingPack ++;   //��������ż�1����ʼ��ȡ��һ������
						len = downdat->datlen;
					 	Firmware_Update.HaveDowingByte += len; //���������ݵĸ�������	  ��ʾ�Ѿ����صĹ̼��Ĵ�С  �ֽ�				 
					    if(Firmware_Update.HaveDowingPack == Firmware_Update.firePack.packSizeT)
						{
							if(Firmware_Update.HaveDowingByte == Firmware_Update.fireSize.bytesizeT)
							{////���ص����������У�飬���У��OK�����ʾ���ع̼��ɹ�
								// datacrc16 = Update_OverallData_CRC16(falshRead,Firmware_Update.HaveDowingByte);
								 datacrc16t = ((Firmware_Update.crc16[0]<<8) | Firmware_Update.crc16[1]);
								 if(allcrc16 == datacrc16t)
								 {///У��ɹ�			
									 // Firmware_Update.UpdateFalg = FIRMWARE_NEWVERSION;
									  buffert[0] = FIRMWARE_NEWVERSION;
									  buffert[1] = Firmware_Update.oldVer.version[0];
									  buffert[2] = Firmware_Update.oldVer.version[1];								 
									  buffert[3] = Firmware_Update.upVer.version[0];
									  buffert[4] = Firmware_Update.upVer.version[1];	
									  buffert[5] = Firmware_Update.fireSize.bytesize[3];
									  buffert[6] = Firmware_Update.fireSize.bytesize[2];
									  buffert[7] = Firmware_Update.fireSize.bytesize[1];
									  buffert[8] = Firmware_Update.fireSize.bytesize[0];
									  buffert[9] = Firmware_Update.firePack.packSize[1];//= date.firePack.packSize[0];
									  buffert[10] = Firmware_Update.firePack.packSize[0];// = date.firePack.packSize[1];									 
									  buffert[11] = Firmware_Update.crc16[0];// = date.crc16[0];
									  buffert[12] = Firmware_Update.crc16[1];// = date.crc16[1];										 									 
									  SaveDat(FLASHADDR_HAVE_NEWVER,&buffert[0],13);   ///��������	

									  buffert[0] = 0;
									  buffert[1] = 0;
									  buffert[2] = 0;								 
									  buffert[3] = 0;
									  buffert[4] = 0;	
									  buffert[5] = 0;
									  buffert[6] = 0;
									  buffert[7] = 0;
									  buffert[8] = 0;
									  buffert[9] = 0;//= date.firePack.packSize[0];
									  buffert[10] = 0;								 
									  buffert[11] = 0;
									  buffert[12] = 0;
									  falshRead(&buffert[0],FLASHADDR_HAVE_NEWVER,13);
									 
									  mt_update_SetUpState(MT_UPDATE_STA_DOWN_OVER);
								 }
								 else
								 {////У��ʧ�ܣ��̼�����ʧ�ܡ�
										mt_update_SetUpState(MT_UPDATE_STA_DOWN_FAIL);
								 }
							}	
							else
							{////�̼�����ʧ��  ���ص��ֽڸ�������
								mt_update_SetUpState(MT_UPDATE_STA_DOWN_FAIL);
							}								
						}
						else
						{
							return 0;						
						}
				 }			 
			 }
		}
	}
	return 1;
}
///////
//////����ֵ  �̼������İٷֱ�  short  �����66.6% ����ֵ 666��
unsigned short mt_update_pro(unsigned char comTyp,void (*getPack)(unsigned char comType,unsigned short packNo,unsigned char *ver))
{
	static unsigned int packseries = 0xffff;
	static unsigned short delayt = 0;
	static unsigned char resentTimes = 0;  //���·��͵Ĵ���
	if(Firmware_Update.UpdateFalg == MT_UPDATE_STA_DOWN_FIRMWARE)
	{///���ϵͳ�������ع̼�
		if(packseries != Firmware_Update.HaveDowingPack)
		{////���Ϸ��ͻ�ȡ�̼���ָ��
			delayt = 0;
			resentTimes = 0;
			packseries = Firmware_Update.HaveDowingPack;
			getPack(comTyp,Firmware_Update.HaveDowingPack,&Firmware_Update.upVer.version[0]);
		}
		else
		{
			delayt ++;
			if(delayt > 350)
			{///3����û�л�Ӧ ���ظ����ͻ�ȡָ��
				delayt = 0;
				packseries = Firmware_Update.HaveDowingPack;
				getPack(comTyp,Firmware_Update.HaveDowingPack,&Firmware_Update.upVer.version[0]);
				resentTimes ++;
				if(resentTimes >  20)
				{///���·��͵Ĵ�������20�ˣ����ʾ���س�ʱ������ʧ��
					  packseries = 0xffff;
					  resentTimes = 0;
						return 0xffff;
				}					
			}		
		}
	}
	return ((Firmware_Update.HaveDowingPack *1000) / Firmware_Update.firePack.packSizeT);	
}
















