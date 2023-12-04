#include "mt_Tftlcd.h"
#include "hal_tftlcd.h"
#include "lcdfont.h"

static void hal_tftlcd_Delay(unsigned int de);

unsigned char ColorBuf[640];

void mt_tftlcd_init(void)
{
  	hal_tftlcdConfig();//初始化GPIO
		hal_tftlcd_Delay(10000);
		hal_oled_RestL();//复位
		hal_tftlcd_Delay(10000);
		hal_oled_RestH();
		hal_tftlcd_Delay(100);

	//************* Start Initial Sequence **********//
		LCD_WR_REG(0x11);
		hal_tftlcd_Delay(10000);//delay_ms(100); //Delay 120ms
		LCD_WR_REG(0X36);// Memory Access Control
		if(USE_HORIZONTAL==0)LCD_WR_DATA8(0x00);
		else if(USE_HORIZONTAL==1)LCD_WR_DATA8(0xC0);
		else if(USE_HORIZONTAL==2)LCD_WR_DATA8(0x70);
		else LCD_WR_DATA8(0xA0);
		LCD_WR_REG(0X3A);
	 // LCD_WR_DATA8(0X03);   //12bit
		LCD_WR_DATA8(0X05);  
		//--------------------------------ST7789S Frame rate setting-------------------------

		LCD_WR_REG(0xb2);
		LCD_WR_DATA8(0x0c);
		LCD_WR_DATA8(0x0c);
		LCD_WR_DATA8(0x00);
		LCD_WR_DATA8(0x33);
		LCD_WR_DATA8(0x33);

		LCD_WR_REG(0xb7);
		LCD_WR_DATA8(0x35);
		//---------------------------------ST7789S Power setting-----------------------------

		LCD_WR_REG(0xbb);
		LCD_WR_DATA8(0x35);

		LCD_WR_REG(0xc0);
		LCD_WR_DATA8(0x2c);

		LCD_WR_REG(0xc2);
		LCD_WR_DATA8(0x01);

		LCD_WR_REG(0xc3);
		LCD_WR_DATA8(0x13);

		LCD_WR_REG(0xc4);
		LCD_WR_DATA8(0x20);

		LCD_WR_REG(0xc6);
		LCD_WR_DATA8(0x0f);

		LCD_WR_REG(0xca);
		LCD_WR_DATA8(0x0f);

		LCD_WR_REG(0xc8);
		LCD_WR_DATA8(0x08);

		LCD_WR_REG(0x55);
		LCD_WR_DATA8(0x90);

		LCD_WR_REG(0xd0);
		LCD_WR_DATA8(0xa4);
		LCD_WR_DATA8(0xa1);
		//--------------------------------ST7789S gamma setting------------------------------
		LCD_WR_REG(0xe0);
		LCD_WR_DATA8(0xd0);
		LCD_WR_DATA8(0x00);
		LCD_WR_DATA8(0x06);
		LCD_WR_DATA8(0x09);
		LCD_WR_DATA8(0x0b);
		LCD_WR_DATA8(0x2a);
		LCD_WR_DATA8(0x3c);
		LCD_WR_DATA8(0x55);
		LCD_WR_DATA8(0x4b);
		LCD_WR_DATA8(0x08);
		LCD_WR_DATA8(0x16);
		LCD_WR_DATA8(0x14);
		LCD_WR_DATA8(0x19);
		LCD_WR_DATA8(0x20);
		LCD_WR_REG(0xe1);
		LCD_WR_DATA8(0xd0);
		LCD_WR_DATA8(0x00);
		LCD_WR_DATA8(0x06);
		LCD_WR_DATA8(0x09);
		LCD_WR_DATA8(0x0b);
		LCD_WR_DATA8(0x29);
		LCD_WR_DATA8(0x36);
		LCD_WR_DATA8(0x54);
		LCD_WR_DATA8(0x4b);
		LCD_WR_DATA8(0x0d);
		LCD_WR_DATA8(0x16);
		LCD_WR_DATA8(0x14);
		LCD_WR_DATA8(0x21);
		LCD_WR_DATA8(0x20);
		LCD_WR_REG(0x29);
		hal_Oled_Display_on();//打开背光

		//LCD_Fill(0,0,LCD_W,LCD_H,YELLOW);
		hal_Tftlcd_Clear();
    	LCD_ShowPicture32PixFont(COOR_ICON_AC_X,COOR_ICON_AC_Y,ICON_32X32_ACLINK,HUE_LCD_FONT,HUE_LCD_BACK,0);
		LCD_ShowPicture32PixFont(COOR_ICON_BAT_X,COOR_ICON_BAT_Y,ICON_32X32_BAT_LEVEL5,HUE_LCD_FONT,HUE_LCD_BACK,0);
		LCD_ShowPicture32PixFont(COOR_ICON_WIFI_X,COOR_ICON_WIFI_Y,ICON_32X32_WIFI_S4,HUE_LCD_FONT,HUE_LCD_BACK,0);
		LCD_ShowPicture32PixFont(COOR_ICON_SIM_X,COOR_ICON_SIM_Y,ICON_32X32_GSM_NOCARD,HUE_LCD_FONT,HUE_LCD_BACK,0);
		LCD_ShowPicture32PixFont(COOR_ICON_SERVER_X,COOR_ICON_SERVER_Y,ICON_32X32_SERVER,HUE_LCD_FONT,HUE_LCD_BACK,0);
		LCD_ShowString(COOR_ICON_SYSTEMODE_X,COOR_ICON_SYSTEMODE_Y," DISARM ",HUE_LCD_FONT,HUE_LCD_BACK,48,0); 
} 

/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1,x2 设置列的起始和结束地址
                y1,y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
void LCD_Address_Set(unsigned short x1,unsigned short y1,unsigned short x2,unsigned short y2)
{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1+2);
		LCD_WR_DATA(x2+2);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1+1);
		LCD_WR_DATA(y2+1);
		LCD_WR_REG(0x2c);//储存器写
}

static void hal_tftlcd_Delay(unsigned int de)
{
	while(de--);
}
/******************************************************************************
      函数说明：在指定区域填充颜色
      入口数据：xsta,ysta   起始坐标
                xend,yend   终止坐标
								color       要填充的颜色
      返回值：  无
******************************************************************************/

void LCD_Fill(unsigned short xsta,unsigned short ysta,unsigned short xend,unsigned short yend,unsigned short color)
{          
	unsigned short i; 
	LCD_Address_Set(xsta,ysta,xend-1,yend-1);//设置显示范围
	for(i=0;i<xend;i++)
  {
		ColorBuf[i++] = color>>8;
		ColorBuf[i] = color;
	}
	for(i=ysta;i<yend*2;i++)
	{		
		 DMA_SPI3_TX(ColorBuf,xend);
	}	
}


void hal_Tftlcd_Clear(void)
{
	LCD_Fill(0,0,LCD_W,LCD_H,HUE_LCD_BACK);
}


/******************************************************************************
      函数说明：在指定位置画点
      入口数据：x,y 画点坐标
                color 点的颜色
      返回值：  无
******************************************************************************/
static void LCD_DrawPoint(unsigned short x,unsigned short y,unsigned short color)
{
	LCD_Address_Set(x,y,x,y);//设置光标位置 
	LCD_WR_DATA(color);
} 


/******************************************************************************
      函数说明：显示单个字符   A
      入口数据：x,y显示坐标
                num 要显示的字符
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowChar(unsigned short x,unsigned short y,unsigned char num,unsigned short fc,unsigned short bc,unsigned char sizey,unsigned char mode)
{
	unsigned char temp,sizex,t,m=0;
	unsigned short i,TypefaceNum;//一个字符所占字节大小
	unsigned short x0=x;
	sizex=sizey/2;
	TypefaceNum=(sizex/8+((sizex%8)?1:0))*sizey;
	num=num-' ';    //得到偏移后的值   67-32 =  35
	LCD_Address_Set(x,y,x+sizex-1,y+sizey-1);  //设置光标位置 
	for(i=0;i<TypefaceNum;i++)
	{ 
		if(sizey==12)temp=ascii_1206[num][i];		       //调用6x12字体
		else if(sizey==16)temp=ascii_1608[num][i];		 //调用8x16字体
		else if(sizey==24)temp=ascii_2412[num][i];		 //调用12x24字体
		else if(sizey==32)temp=ascii_3216[num][i];		 //调用16x32字体
		else if(sizey==48)temp=ascii_4824[num][i];		 //调用16x32字体
		else return;
		for(t=0;t<8;t++)
		{
			if(!mode)//非叠加模式
			{
				if(temp&(0x01<<t))LCD_WR_DATA(fc);
				else LCD_WR_DATA(bc);
				m++;
				if(m%sizex==0)
				{
					m=0;
					break;
				}
			}
			else//叠加模式
			{
				if(temp&(0x01<<t))LCD_DrawPoint(x,y,fc);//画一个点
				x++;
				if((x-x0)==sizex)
				{
					x=x0;
					y++;
					break;
				}
			}
		}
	}   	 	  
}


/******************************************************************************
      函数说明：显示字符串    
      入口数据：x,y显示坐标       320*320
                *p 要显示的字符串
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0带字节背景  1透明模式
      返回值：  无
******************************************************************************/
void LCD_ShowString(unsigned short x,unsigned short y,const unsigned char *p,unsigned short fc,unsigned short bc,unsigned char sizey,unsigned char mode)
{         
	while(*p!='\0')
	{       
		LCD_ShowChar(x,y,*p,fc,bc,sizey,mode);
		x+=sizey/2;
		p++;
	}  
}

/******************************************************************************
      函数说明：小图标显示   
      入口数据：x,y显示坐标       
                num 要显示的图标
                fc 图标的颜色
                bc 图标背景色
                sizey 字号
      返回值：  无
******************************************************************************/
void LCD_ShowPicture32PixFont(unsigned short x,unsigned short y,unsigned char num,unsigned short fc,unsigned short bc,unsigned char mode)
{
	unsigned char temp,sizex,t,m=0;
	unsigned short i,TypefaceNum;////一个字符所占字节大小
	unsigned short x0=x;
	TypefaceNum=128;
	LCD_Address_Set(x,y,x+31,y+31);   //设置光标位置 
	for(i=0;i<TypefaceNum;i++)
	{ 
		temp=gImage_wifiSigal[num][i];
		for(t=0;t<8;t++)
		{
			if(!mode)//非叠加模式
			{
				if(temp&(0x01<<t))LCD_WR_DATA(fc);
				else LCD_WR_DATA(bc);
				m++;
				if(m%sizex==0)
				{
					m=0;
					break;
				}
			}
			else//叠加模式
			{
				if(temp&(0x01<<t))
					LCD_DrawPoint(x,y,fc);//画一个点
				x++;
				if((x-x0)==sizex)
				{
					x=x0;
					y++;
					break;
				}
			}
		}
	}   	 	  
}
