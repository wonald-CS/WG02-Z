#ifndef ____PARA_H_
#define ____PARA_H_

#define STM32_UID_ADDR		0x1ffff7e8

extern unsigned char STM32_UID[12];
#define MT_GET_MCU_UID(i)	(STM32_UID[i])


typedef struct SYSTEM_TIME
{
	unsigned short year;
	unsigned char mon;
	unsigned char day;
	unsigned char week;
	unsigned char hour;
	unsigned char min;
	unsigned char sec;
}stu_system_time;

extern volatile stu_system_time stuSystemtime;

#define MT_SET_DATE_YEAR(x)			(stuSystemtime.year = x)
#define MT_SET_DATE_MON(x)			(stuSystemtime.mon = x)
#define MT_SET_DATE_DAY(x)			(stuSystemtime.day = x)
#define MT_SET_DATE_WEEK(x)			(stuSystemtime.week = x)
#define MT_SET_DATE_HOUR(x)			(stuSystemtime.hour = x)
#define MT_SET_DATE_MIN(x)			(stuSystemtime.min = x)
#define MT_SET_DATE_SEC(x)			(stuSystemtime.sec = x)


void ParaInit(void);
void SystemTime_local(void);


#endif
