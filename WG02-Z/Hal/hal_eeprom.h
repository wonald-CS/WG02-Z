#ifndef ____HAL_EEPROM_H_
#define ____HAL_EEPROM_H_

#define EEPROM_PAGE_SIZE 64

#define I2C_SCL_PORT	GPIOB
#define I2C_SCL_PIN		GPIO_Pin_8

#define I2C_SDA_PORT	GPIOB
#define I2C_SDA_PIN		GPIO_Pin_9


#define I2C_SCL_1()  	GPIO_SetBits(I2C_SCL_PORT, I2C_SCL_PIN)			    // SCL = 1
#define I2C_SCL_0()  	GPIO_ResetBits(I2C_SCL_PORT, I2C_SCL_PIN)			// SCL = 0
#define I2C_SDA_1()  	GPIO_SetBits(I2C_SDA_PORT, I2C_SDA_PIN)			    // SDA = 1
#define I2C_SDA_0()  	GPIO_ResetBits(I2C_SDA_PORT, I2C_SDA_PIN)			// SDA = 0

#define I2C_SCL_READ()  GPIO_ReadInputDataBit(I2C_SCL_PORT, I2C_SCL_PIN)				// 读SCL口线状态
#define I2C_SDA_READ()  GPIO_ReadInputDataBit(I2C_SDA_PORT, I2C_SDA_PIN)				// 读SDA口线状态




#endif 
