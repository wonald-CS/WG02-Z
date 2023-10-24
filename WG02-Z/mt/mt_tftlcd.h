#ifndef ____MT_TFTLCD_H_
#define ____MT_TFTLCD_H_

#define USE_HORIZONTAL 3  //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏

#if USE_HORIZONTAL==0||USE_HORIZONTAL==1

#define LCD_W 240
#define LCD_H 320

#else
#define LCD_W 320
#define LCD_H 240


#endif

///RGB565  
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE           	 0x001F  
#define BRED             0XF81F
#define GRED 		   	 0XFFE0
#define GBLUE	         0X07FF
#define RED              0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色
#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define GRAYBLUE       	 0X5458 //灰蓝色
#define LIGHTGREEN     	 0X841F //浅绿色
#define LGRAY 			 0XC618 //浅灰色(PANNEL),窗体背景色
#define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)

enum
{
	FORTSIZE_12 = 12,
	FORTSIZE_16 = 16,	
	FORTSIZE_24 = 24,
	FORTSIZE_32 = 32,	
	FORTSIZE_48 = 48,	
};



#define HUE_LCD_FONT     WHITE			//字体颜色
#define HUE_LCD_BACK     BLACK			//液晶背光颜色
#define HUE_FONT_BACK    LIGHTGREEN		//字体背光颜色


#define COOR_ICON_AC_X   1
#define COOR_ICON_AC_Y   1

#define COOR_ICON_BAT_X   33
#define COOR_ICON_BAT_Y   1

#define  COOR_ICON_SERVER_X   210
#define  COOR_ICON_SERVER_Y    1

#define  COOR_ICON_WIFI_X   250
#define  COOR_ICON_WIFI_Y    1

#define  COOR_ICON_SIM_X   282
#define  COOR_ICON_SIM_Y    1

#define  COOR_ICON_SYSTEMODE_X   70
#define  COOR_ICON_SYSTEMODE_Y   100



void mt_tftlcd_init(void);
void hal_Tftlcd_Clear(void);
void LCD_Fill(unsigned short xsta,unsigned short ysta,unsigned short xend,unsigned short yend,unsigned short color);
void LCD_ShowString(unsigned short x,unsigned short y,const unsigned char *p,unsigned short fc,unsigned short bc,unsigned char sizey,unsigned char mode);
void LCD_ShowPicture32PixFont(unsigned short x,unsigned short y,unsigned char num,unsigned short fc,unsigned short bc,unsigned char mode);
#endif

