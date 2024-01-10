#include "para.h"
#include "hal_eeprom.h"



//para.c
SystemPara_InitTypeDef mt_sEPR;  ///系统参数
Stu_DTC dERP[PARA_DTC_SUM];	//EEPROM设备参数数据结构

unsigned char STM32_UID[12];
volatile stu_system_time stuSystemtime;

static void mt_para_SystemParaCheck(void);
static unsigned char mt_para_DtcParaCheck(void);
void mt_para_FactoryReset(void);
void mt_para_dtatest(void);
void ParaInit(void)
{
  unsigned char i;
  
  //void ParaInit(void)   
  //  unsigned char i;
	I2C_Read(STU_SYSTEMPARA_OFFSET,(unsigned char*)(&mt_sEPR),sizeof(mt_sEPR));		
	I2C_Read(STU_DEVICEPARA_OFFSET,(unsigned char*)(&dERP),sizeof(dERP));

	mt_para_SystemParaCheck();
	if(mt_para_DtcParaCheck())
	{
		 mt_para_FactoryReset();
	} 
	for(i=0; i<12; i++)
	{
		STM32_UID[i] = *((unsigned char *)(STM32_UID_ADDR+i));
	}
	
	MT_SET_DATE_YEAR(2022);
	MT_SET_DATE_MON(8);
	MT_SET_DATE_DAY(27);
	MT_SET_DATE_WEEK(6);
	MT_SET_DATE_HOUR(18);
	MT_SET_DATE_MIN(11);
	MT_SET_DATE_SEC(30);
 	//mt_para_dtatest();
}
///
void mt_para_dtatest(void)
{
	unsigned short i,j;
	for(i=0; i<4; i++)
	{///
		dERP[i].ID = i+1;
		dERP[i].Mark = 1;
		dERP[i].NameNum = 0;
		
	  dERP[i].Name[0] = 'Z';
		dERP[i].Name[1] = 'o';
		dERP[i].Name[2] = 'n';
		dERP[i].Name[3] = 'e';
		dERP[i].Name[4] = '-';
		
			
		dERP[i].Name[5] = '0'+((i+1)/100);
		dERP[i].Name[6] = '0'+(((i+1)%100)/10);
		dERP[i].Name[7] = '0'+(((i+1)%100)%10);		
		
		for(j=8; j<26; j++)
		{
			dERP[i].Name[j] = ' ';
		}
		if(i%2)
			dERP[i].DTCType = DTC_DOOR;
		else
			dERP[i].DTCType = DTC_REMOTE;
		
		
		dERP[i].ZoneType = ZONE_TYP_1ST;
		dERP[i].Code[0] = 0;
		dERP[i].Code[1] = 0;
		dERP[i].Code[2] = 0;
		dERP[i].Code[3] = 0;
		dERP[i].Code[4] = 0;
		dERP[i].Code[5] = 0;
		dERP[i].Code[6] = 0;
		dERP[i].Code[7] = 0;
		dERP[i].Code[8] = 0;
		dERP[i].Code[9] = 0;		
		dERP[i].Code[10] = 0;
		dERP[i].Code[11] = i;			
		dERP[i].node[0] = 0;
		dERP[i].node[1] = i;				
	
	}		
}







static void mt_para_SystemParaCheck(void)
{
	unsigned char i,j;
	if(((MT_GET_PARA_SYS_FIRMWARE(0) == 0) && (MT_GET_PARA_SYS_FIRMWARE(1) == 0)) ||((MT_GET_PARA_SYS_FIRMWARE(0) == 0xff) && (MT_GET_PARA_SYS_FIRMWARE(1) == 0xff)))
	{
		MT_SET_PARA_SYS_FIRMWARE(0,0);
		MT_SET_PARA_SYS_FIRMWARE(1,0x64);//v1.00
	}
	if((MT_GET_PARA_SYS_ADMINPASSWORD(0)>9)||(MT_GET_PARA_SYS_ADMINPASSWORD(1)>9)||(MT_GET_PARA_SYS_ADMINPASSWORD(2)>9)||(MT_GET_PARA_SYS_ADMINPASSWORD(3)>9))
	{///0-9   ABC   &
		MT_SET_PARA_SYS_ADMINPASSWORD(0,0);
		MT_SET_PARA_SYS_ADMINPASSWORD(1,0);
		MT_SET_PARA_SYS_ADMINPASSWORD(2,0);
		MT_SET_PARA_SYS_ADMINPASSWORD(3,0);
	}	
	for(i=0; i<7; i++)
	{
		for(j=0; j<20; j++)
		{
			if(MT_GET_PARA_SYS_PHONENUMBER(i,j) > 9)
			{//0-9   0XFF  表示值是空的
				MT_SET_PARA_SYS_PHONENUMBER(i,j,0xFF);
			}
		}
	}	
}

static unsigned char mt_para_DtcParaCheck(void)
{
	unsigned char i;
	unsigned char error = 0;
	
	for(i=0; i<PARA_DTC_SUM; i++)
	{
		if(dERP[i].ID >= PARA_DTC_SUM)
		{
			error = 1;
		}
		if(dERP[i].Mark > 1)
		{
			error = 1;
		}
		if(dERP[i].DTCType>= DTC_TYP_SUM)
		{
			error = 1;
		}	
		if(dERP[i].ZoneType>= STG_DEV_AT_SUM)
		{
			error = 1;
		}
		dERP[i].sleepTimes = 0;
	}
	return error;
}

////系统参数恢复出厂设置
void mt_para_FactoryReset(void)
{
	unsigned short i;
	unsigned char j;
	//
	for(i=0; i<PARA_DTC_SUM; i++)
	{///
		dERP[i].ID = 0;
		dERP[i].Mark = 0;
		dERP[i].NameNum = 0;
		for(j=0; j<16; j++)
		{
			dERP[i].Name[i] = 0;
		}
		dERP[i].DTCType = DTC_DOOR;
		dERP[i].ZoneType = ZONE_TYP_1ST;
		dERP[i].Code[0] = 0;
		dERP[i].Code[1] = 0;
		dERP[i].Code[2] = 0;
		dERP[i].Code[3] = 0;
		dERP[i].Code[4] = 0;	
		dERP[i].Code[5] = 0;
		dERP[i].Code[6] = 0;
		dERP[i].Code[7] = 0;
		dERP[i].Code[8] = 0;
		dERP[i].Code[9] = 0;	
		dERP[i].Code[10] = 0;			
	    dERP[i].Code[11] = 0;	
		
		dERP[i].node[0] = 0;
		dERP[i].node[1] = 0;			
	}
	
	for(i=0; i<7; i++)
	{
		for(j=0; j<20; j++)
		{
			MT_SET_PARA_SYS_PHONENUMBER(i,j,0xFF);
		}
	}
	for(i=0; i<4; i++)
	{
		MT_SET_PARA_SYS_ADMINPASSWORD(i,0);
	}
	
	I2C_PageWrite(STU_SYSTEMPARA_OFFSET,(unsigned char*)(&mt_sEPR),sizeof(mt_sEPR));
	I2C_PageWrite(STU_DEVICEPARA_OFFSET,(unsigned char*)(&dERP),sizeof(dERP));
	
	I2C_Read(STU_SYSTEMPARA_OFFSET,(unsigned char*)(&mt_sEPR),sizeof(mt_sEPR));		
	I2C_Read(STU_DEVICEPARA_OFFSET,(unsigned char*)(&dERP),sizeof(dERP));
}






void SystemTime_local(void)
{
	static unsigned int systemDelaytimer = 0;
	systemDelaytimer ++;
	if(systemDelaytimer > 20000)
	{
		systemDelaytimer = 0;
		stuSystemtime.sec ++;
		if(stuSystemtime.sec  == 60)
		{
			stuSystemtime.sec = 0;
			stuSystemtime.min++;
			if(stuSystemtime.min  == 60)
			{
				stuSystemtime.min = 0;
				stuSystemtime.hour ++;
				if(stuSystemtime.hour == 24)
				{
						stuSystemtime.hour = 0;
						stuSystemtime.day ++;
						stuSystemtime.week++;
						if(stuSystemtime.week ==7)
						{
							stuSystemtime.week = 0;
						}
						if(stuSystemtime.day == 30)
						{		
							stuSystemtime.day	= 0;
							stuSystemtime.mon ++;
							if(stuSystemtime.mon == 13)
							{
								stuSystemtime.mon = 1;
								stuSystemtime.year++;
							}
						}						
				}
			}
		}	
	}
}

 
////////////////////////////////////////
//传感器添加,返回0xFE表示已经学习,返回0xFF则学习失败，，学习成功，则返回探测器的区号
str_ParaDetectorApplyNetState AddDtc(Stu_DTC *pDevPara)
{
	Stu_DTC DevPara;
	unsigned char i,j,Temp,NameStrIndex;
  str_ParaDetectorApplyNetState applyNetSta;
	NameStrIndex = 0;
	Temp = 0;
	
	for(i=0; i<PARA_DTC_SUM; i++)
	{//判断探测器是否存在
		if(dERP[i].Mark && 
		(dERP[i].Code[0]==pDevPara->Code[0]) &&
		(dERP[i].Code[1]==pDevPara->Code[1]) &&
		(dERP[i].Code[2]==pDevPara->Code[2]) &&
		(dERP[i].Code[3]==pDevPara->Code[3]) &&		
		(dERP[i].Code[4]==pDevPara->Code[4]) &&
		(dERP[i].Code[5]==pDevPara->Code[5]) &&
		(dERP[i].Code[6]==pDevPara->Code[6]) &&
		(dERP[i].Code[7]==pDevPara->Code[7]) &&
		(dERP[i].Code[8]==pDevPara->Code[8]) &&		
		(dERP[i].Code[9]==pDevPara->Code[9]) &&
		(dERP[i].Code[10]==pDevPara->Code[10]) &&
		(dERP[i].Code[11]==pDevPara->Code[11]) &&
		(dERP[i].node[0]==pDevPara->node[0]) &&
		(dERP[i].node[1]==pDevPara->node[1]) 		
		)			
		{		
			applyNetSta.state = DET_HAVED_LEARN;
			applyNetSta.code = i;
			return (applyNetSta);									//探测器已存在  返回已学习的探测器类型
		}	
	}
	for(i=0; i<PARA_DTC_SUM; i++)
	{//1-20
		if(!dERP[i].Mark)
		{//   == ！0
			DevPara.Name[0] = 'Z';
			DevPara.Name[1] = 'o';
			DevPara.Name[2] = 'n';
			DevPara.Name[3] = 'e';
			DevPara.Name[4] = '-';
			NameStrIndex = 5;
			Temp = 	i+1;	
			
			DevPara.Name[NameStrIndex++] = '0'+(Temp/100);
			DevPara.Name[NameStrIndex++] = '0'+((Temp%100)/10);
			DevPara.Name[NameStrIndex++] = '0'+((Temp%100)%10);
			
			for(j=NameStrIndex; j<16; j++)
			{
				DevPara.Name[j] = 0;					//把没用到的名字字节清零
			}
			DevPara.Name[16] = 0;
			DevPara.ID = i+1;  ///防区号 
			DevPara.Mark = 1;  //已经学习
			DevPara.NameNum = Temp;

			DevPara.DTCType = pDevPara->DTCType;  //遥控器  门磁  烟感
			DevPara.ZoneType = pDevPara->ZoneType; ///防线类型  24小时防线  第1防线  第2防线
	
			DevPara.Code[0] = pDevPara->Code[0];
			DevPara.Code[1] = pDevPara->Code[1];
			DevPara.Code[2] = pDevPara->Code[2];
			DevPara.Code[3] = pDevPara->Code[3];
			DevPara.Code[4] = pDevPara->Code[4];
			DevPara.Code[5] = pDevPara->Code[5];
			DevPara.Code[6] = pDevPara->Code[6];
			DevPara.Code[7] = pDevPara->Code[7];
			DevPara.Code[8] = pDevPara->Code[8];
			DevPara.Code[9] = pDevPara->Code[9];			
			DevPara.Code[10] = pDevPara->Code[10];
			DevPara.Code[11] = pDevPara->Code[11];
			DevPara.node[0] = pDevPara->node[0];
			DevPara.node[1] = pDevPara->node[1];	
			DevPara.sleepTimes = 0;	
			////dERP[0];
			I2C_PageWrite(STU_DEVICEPARA_OFFSET+i*STU_DTC_SIZE,(unsigned char*)(&DevPara),sizeof(DevPara)); //新设备信息写入EEPROM
			I2C_Read(STU_DEVICEPARA_OFFSET+i*STU_DTC_SIZE,(unsigned char*)&dERP[i],STU_DTC_SIZE);
			
			applyNetSta.state = DET_UNLEARN;
			applyNetSta.code = i;  //i+1 表示防区号
			return (applyNetSta);							//学习成功，返回探测器的存储下标
		}
	}
	applyNetSta.state = DET_LEARN_FAIL;
	applyNetSta.code = i;	
	return applyNetSta;			//学习失败
}

//检查DTC是否存在，0不存在，1存在
unsigned char CheckPresenceofDtc(unsigned char i)
{///dERP[20];
	unsigned char result;
	result = 0;
	if(i < PARA_DTC_SUM)	//防溢出检测
	{
		if(dERP[i].Mark)
		{
			result = 1;
		}
	}
	return result;
}


//获取指定探测器的结构体数据,*pdDevPara-外部结构体指针，id-要获取的探测器ID
void GetDtcStu(Stu_DTC *pdDevPara, unsigned char idx)
{
	unsigned char i;
 
	if(idx >= PARA_DTC_SUM)		
	{
		return;			//id异常
	}
	
	pdDevPara->ID = dERP[idx].ID;
	pdDevPara->DTCType = dERP[idx].DTCType;
	pdDevPara->Mark = dERP[idx].Mark ;
	pdDevPara->NameNum = dERP[idx].NameNum;
	pdDevPara->sleepTimes = dERP[idx].sleepTimes;
	
	for(i=0; i<16; i++)
	{
		pdDevPara->Name[i] = dERP[idx].Name[i];
	}
	pdDevPara->DTCType = dERP[idx].DTCType;
	pdDevPara->ZoneType = dERP[idx].ZoneType;
	pdDevPara->Code[0] = dERP[idx].Code[0];
	pdDevPara->Code[1] = dERP[idx].Code[1];
	pdDevPara->Code[2] = dERP[idx].Code[2];
	pdDevPara->Code[3] = dERP[idx].Code[3];
	pdDevPara->Code[4] = dERP[idx].Code[4];	
	pdDevPara->Code[5] = dERP[idx].Code[5];
	pdDevPara->Code[6] = dERP[idx].Code[6];
	pdDevPara->Code[7] = dERP[idx].Code[7];
	pdDevPara->Code[8] = dERP[idx].Code[8];
	pdDevPara->Code[9] = dERP[idx].Code[9];			
	pdDevPara->Code[10] = dERP[idx].Code[10];
	pdDevPara->Code[11] = dERP[idx].Code[11];
	pdDevPara->node[0] = dERP[idx].node[0];
	pdDevPara->node[1] = dERP[idx].node[1];	
}



//修改探测器属性,id->指定探测器ID psDevPara->探测器属性结构体
void SetDtcAbt(unsigned char id,Stu_DTC *psDevPara)
{
	unsigned char i;
	if(id >= PARA_DTC_SUM)		
	{
		return;			//id异常
	}
	dERP[id].ID = psDevPara->ID;
	dERP[id].Mark = psDevPara->Mark ;
	dERP[id].NameNum =  psDevPara->NameNum;
	
	for(i=0; i<16; i++)
	{
		dERP[id].Name[i] = psDevPara->Name[i];
	}
	dERP[id].DTCType = psDevPara->DTCType;
	dERP[id].ZoneType = psDevPara->ZoneType;
	dERP[id].Code[0] = psDevPara->Code[0];
	dERP[id].Code[1] = psDevPara->Code[1];
	dERP[id].Code[2] = psDevPara->Code[2];
  dERP[id].Code[3] = psDevPara->Code[3];
	dERP[id].Code[4] = psDevPara->Code[4];
	dERP[id].Code[4] = psDevPara->Code[4];
	dERP[id].Code[5] = psDevPara->Code[5];
	dERP[id].Code[6] = psDevPara->Code[6];
	dERP[id].Code[7] = psDevPara->Code[7];
	dERP[id].Code[8] = psDevPara->Code[8];
	dERP[id].Code[9] = psDevPara->Code[9];			
	dERP[id].Code[10] = psDevPara->Code[10];
	dERP[id].Code[11] = psDevPara->Code[11];
	dERP[id].node[0] = psDevPara->node[0];
	dERP[id].node[1] = psDevPara->node[1];
	I2C_PageWrite(STU_DEVICEPARA_OFFSET+id*STU_DTC_SIZE,(unsigned char*)psDevPara,STU_DTC_SIZE); //新设备信息写入EEPROM
	I2C_Read(STU_DEVICEPARA_OFFSET+id*STU_DTC_SIZE,(unsigned char*)&dERP[id],STU_DTC_SIZE);	
}


//探测器匹配，返回0xFF匹配失败，探测器不存在,匹配成功返回探测器ID
unsigned char DtcMatching(unsigned char *pCode)
{
	unsigned char i=0;
	for(i=0; i<PARA_DTC_SUM; i++)
	{
		if(dERP[i].Mark &&    //已学习
		(dERP[i].node[0]==pCode[0]) &&
		(dERP[i].node[1]==pCode[1]))//判断探测器是否存在
		{	
			dERP[i].sleepTimes = 0;  ///离线时间计数变量	
		    //I2C_Read(STU_DEVICEPARA_OFFSET+i*STU_DTC_SIZE,(unsigned char*)&dERP[i],STU_DTC_SIZE);	
		 	return (dERP[i].ID);
		} 
	}
	return 0xff;
}


