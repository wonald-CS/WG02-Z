#ifndef _HAL_USART_H_
#define _HAL_USART_H_

#define DEBUG_TX_PORT	GPIOA
#define DEBUF_TX_PIN	GPIO_Pin_9

#define DEBUG_RX_PORT	GPIOA
#define DEBUF_RX_PIN	GPIO_Pin_10

#define LORA_RX_PORT	GPIOD
#define LORA_RX_PIN	    GPIO_Pin_2

#define LORA_TX_PORT	GPIOC
#define LORA_TX_PIN	    GPIO_Pin_12

#define WIFI_TX_PORT	GPIOB
#define WIFI_TX_PIN	    GPIO_Pin_10

#define WIFI_RX_PORT	GPIOB
#define WIFI_RX_PIN	    GPIO_Pin_11

#define G4_TX_PORT			GPIOA
#define G4_TX_PIN			GPIO_Pin_2
#define G4_RX_PORT			GPIOA
#define G4_RX_PIN    	    GPIO_Pin_3



#define DEBUG_USART_PORT	USART1
#define G4_USART_PORT	    USART2
#define WIFI_USART_PORT     USART3
#define LORA_UART_PORT      UART5

#define DEBUG_TXBUFF_SIZE_MAX       512

typedef void (*Uart5DateRx)(unsigned char pdat);
typedef void (*Usart3DateRx)(unsigned char pdat);
typedef void (*Usart2DateRx)(unsigned char pdat);

void hal_UsartInit(void);
void hal_UsartProc(void);
void hal_Lora_Proc(void);

void hal_lora_DataSent(unsigned char *buf,unsigned char len);
void hal_wifi_DataSent(unsigned char  *buf,unsigned int  len);
void hal_4G_DataSent(unsigned char *buf,unsigned int len);
void hal_4G_StringSent(unsigned char *buf);

void USART1_PutInDebugString(unsigned char pData[],unsigned char len);
void USART1_PutInDebugInfo(const char pData[]);
void USART1_DebugDatPrintQueue(unsigned char dat);

void hal_usart_Uart5DateRxCBSRegister(Uart5DateRx pCBS);
void hal_usart_Usart3DateRxCBSRegister(Usart3DateRx pCBS);
void hal_usart_Usart2DateRxCBSRegister(Usart2DateRx pCBS);





#endif
