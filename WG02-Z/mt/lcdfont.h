#ifndef __LCDFONT_H
#define __LCDFONT_H 	   

extern const unsigned char ascii_1206[][12];
extern const unsigned char ascii_1608[][16];
extern const unsigned char ascii_2412[][48];
extern const unsigned char ascii_3216[95][64];
extern const unsigned char ascii_4824[][144];



enum
{
	ICON_32X32_WIFI_S1,
	ICON_32X32_WIFI_S2,
	ICON_32X32_WIFI_S3,
	ICON_32X32_WIFI_S4,
	
	ICON_32X32_GSM_NOCARD,

	ICON_32X32_GSM_S1,
	ICON_32X32_GSM_S2,
	ICON_32X32_GSM_S3,
	ICON_32X32_GSM_S4,

	ICON_32X32_BAT_LEVEL0,	
	ICON_32X32_BAT_LEVEL1,		
	ICON_32X32_BAT_LEVEL2,		
	ICON_32X32_BAT_LEVEL3,	
	ICON_32X32_BAT_LEVEL4,	
	ICON_32X32_BAT_LEVEL5,	

	ICON_32X32_ACLINK,	
	ICON_32X32_ACBREAK,	

	ICON_32X32_SERVER,	

	ICON_32X32_CLEAR,	

	ICON_32X32_SUM
};


extern const unsigned char gImage_wifiSigal[ICON_32X32_SUM][128];


#endif


