#ifndef _HAL_USB_H_
#define _HAL_USB_H_

#define USB_PORT			GPIOB
#define USB_PIN			GPIO_Pin_1


typedef enum{
    Sta_UnLink,
    Sta_Link,
}USB_AC_Val;


void hal_UsbInit(void);
USB_AC_Val Usb_Ac_Det(void);

#endif
