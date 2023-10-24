#ifndef _HAL_AL6630_H_
#define _HAL_AL6630_H_

#define Det_TemHum_Time 300

enum{
    TemHum_Step0_Idle,
    TemHum_Step1_Start,             //low(1-20ms)  输出                      10ms
    TemHum_Step2_McuRelease,        //high(10-200us)                         20us
    TemHum_Step3_Respond,           //low(75-85us) -- high(75-85us)          80us
    TemHum_Step4_Data,              //0:low(50us) -- high(26-28us)     1:low(50us) -- high(70us)
    TemHum_Step5_AL6630Release,     //low(50us)
    TemHum_Step6_Done,              //high         输出
};


typedef struct
{
    unsigned char TemHum_Step;      //获取温湿度步骤
    unsigned short Temperature;      //温度
    unsigned short Humidity;         //湿度
    unsigned int  Down_GetPos;      //捕获下降沿位置
    unsigned char Data_Len;         //数据长度
    unsigned char Data_GetFlag;     //数据获取完成标志位
	unsigned char TemHum_Rec[5];    //40位数据记录
}Str_TemHum;


unsigned short Get_Temperature_Data(void);
unsigned short Get_Humidity_Data(void);
void hal_GetTemHum_Proc(void);
void hal_Al6630_Init(void);

#endif 
