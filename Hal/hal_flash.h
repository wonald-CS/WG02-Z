#ifndef _HAL_FLASH_H
#define _HAL_FLASH_H

#define SPI2_SCK_PORT       GPIOB
#define SPI2_SCK_PIN        GPIO_Pin_13

#define SPI2_MOSI_PORT       GPIOB
#define SPI2_MOSI_PIN        GPIO_Pin_15

#define SPI2_MISO_PORT       GPIOB
#define SPI2_MISO_PIN        GPIO_Pin_14

#define SPI2_NSS_PORT       GPIOB
#define SPI2_NSS_PIN        GPIO_Pin_12
 

void hal_spi2Init(void);
void hal_spi2CSDrive(unsigned char sta);
unsigned char  hal_spi2ReadWriteByte(unsigned char  TxData);

#endif
