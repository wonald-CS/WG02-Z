#ifndef ____HAL_TFTLCD_H_
#define ____HAL_TFTLCD_H_

void hal_tftlcdConfig(void);

void LCD_WR_REG(unsigned char dat);
void LCD_WR_DATA8(unsigned char dat);
void LCD_WR_DATA(unsigned short dat);
void DMA_SPI3_TX(unsigned char *buffer,unsigned short len);

void hal_Oled_Display_on(void);
void hal_Oled_Display_off(void);
void hal_oled_RestH(void);
void hal_oled_RestL(void);


#endif




