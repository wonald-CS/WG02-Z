#include "stm32F10x.h"
#include "hal_usb.h"


static unsigned char Get_Usb_Ac(void);

void hal_UsbInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB,ENABLE); 						 
	GPIO_InitStructure.GPIO_Pin = USB_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(USB_PORT, &GPIO_InitStructure);
}

static unsigned char Get_Usb_Ac(void)
{
    return GPIO_ReadInputDataBit(USB_PORT,USB_PIN);
}


USB_AC_Val Usb_Ac_Det(void)
{
    unsigned char count = 0;
    USB_AC_Val new_value,value;
    value =(USB_AC_Val) Get_Usb_Ac();

    while (value != new_value)
    {
        count++;
        if(count >= 20)
        {
            return new_value;
        }
        new_value = (USB_AC_Val) Get_Usb_Ac();
    }
    
    return value;
}
