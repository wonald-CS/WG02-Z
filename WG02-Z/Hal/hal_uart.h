#ifndef _HAL_USART_H
#define _HAL_USART_H

//#define DEBUG_HAL_WIFI   1
#define DEBUG_HAL_GSM    1

typedef void (*Uart2DateRx)(unsigned char pdat);
typedef void (*Uart3DateRx)(unsigned char pdat);
typedef void (*Uart5DateRx)(unsigned char pdat); 

void hal_UsartInit(void);
void hal_UsartProc(void);

void USART1_PutInDebugString(unsigned char pData[],unsigned char len);
void USART1_PutInDebugInfo(const char pData[]);

void hal_usart_Uart2DateRxCBSRegister(Uart3DateRx pCBS);
void hal_Usart2_SentString(unsigned char *buf);

void Hal_Uart3_Send_Data(unsigned char  *buf,unsigned int  len);
void hal_usart_Uart3DateRxCBSRegister(Uart3DateRx pCBS);

void hal_usart_Uart5DateTx(unsigned char *buf,unsigned char len);
void hal_usart_Uart5DateRxCBSRegister(Uart5DateRx pCBS);

#endif
