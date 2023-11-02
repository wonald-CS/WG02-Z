#include "hal_flash.h"
#include "mt_flash.h"

void mt_flashEraseSector(unsigned int Dst_Addr);
void mt_flashWaitBusy(void);
void mt_flashWriteEnable(void);
//void mt_flash_test(void);

//��ȡ������ID   FE16H
unsigned short mt_flashReadID(void)
{
	unsigned short Temp = 0;	  
	hal_spi2CSDrive(0); 			    
	hal_spi2ReadWriteByte(0x90);//????ID??	    
	hal_spi2ReadWriteByte(0x00); 	    
	hal_spi2ReadWriteByte(0x00); 	    
	hal_spi2ReadWriteByte(0x00); 	 			   
	Temp|=hal_spi2ReadWriteByte(0xFF)<<8;  //
	Temp|=hal_spi2ReadWriteByte(0xFF);	 
	hal_spi2CSDrive(1); 			    
	return Temp;
}  

void mt_flashInit(void)
{
 // static unsigned short produid;
	hal_spi2Init(); 
// 	produid = mt_flashReadID();
// 	mt_flash_test();
}


//pBuffer-��ȡ���ݴ洢��ַ,ReadAddr-Flash��ַ,NumByteToRead-��ȡ�ֽ���
void mt_flashRead(unsigned char *pBuffer,unsigned int ReadAddr,unsigned int NumByteToRead)   
{ 
	unsigned char  *pBuff;
	unsigned short i,num;  
	unsigned int RdAddr;
	RdAddr = ReadAddr;
	num = NumByteToRead;
	pBuff = pBuffer;
	hal_spi2CSDrive(0);                            //ʹ������   
	hal_spi2ReadWriteByte(0x03);         //���Ͷ�ȡ����   -
//      00 12 34 56H
	hal_spi2ReadWriteByte((unsigned char )((RdAddr)>>16));  //����24bit��ַ    
	hal_spi2ReadWriteByte((unsigned char )((RdAddr)>>8));   
	hal_spi2ReadWriteByte((unsigned char )RdAddr);   
	for(i=0;i<num;i++)
	{ 
		pBuff[i]=hal_spi2ReadWriteByte(0XFF);   //ѭ������  
	}
	hal_spi2CSDrive(1);                             //ȡ��Ƭѡ     	      
}  


//SPI��һҳ(0~65535)��д������256���ֽڵ�����
//��ָ����ַ��ʼд�����256�ֽڵ�����
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���256),������Ӧ�ó�����ҳ��ʣ���ֽ���!!!	 
void mt_flashWritePage(unsigned char * pBuffer,unsigned int WriteAddr,unsigned short NumByteToWrite)
{
	unsigned char  *pBuff;
	unsigned short i,num;  
	unsigned int wAddr;
	pBuff = pBuffer;
	wAddr = WriteAddr;
	num = NumByteToWrite;
	mt_flashWriteEnable();                  //SET WEL 

	hal_spi2CSDrive(0);                              //ʹ������   
	hal_spi2ReadWriteByte(W25X_PageProgram);      //����дҳ����   
	hal_spi2ReadWriteByte((unsigned char )((wAddr)>>16)); //����24bit��ַ    
	hal_spi2ReadWriteByte((unsigned char )((wAddr)>>8));   
	hal_spi2ReadWriteByte((unsigned char )wAddr);   
	for(i=0;i<num;i++)
	  hal_spi2ReadWriteByte(pBuff[i]);//ѭ��д��  
	hal_spi2CSDrive(1);   
	mt_flashWaitBusy();   //�ȴ�д�����
} 
//SPI_FLASHдʹ��	
//��WEL��λ   
void mt_flashWriteEnable(void)   
{
	hal_spi2CSDrive(0);                            //ʹ������   
	hal_spi2ReadWriteByte(W25X_WriteEnable);      //����дʹ��  0x06
	hal_spi2CSDrive(1);                           //ȡ��Ƭѡ     	      
} 

//��ȡSPI_FLASH��״̬�Ĵ���
//BIT7  6   5   4   3   2   1    0
//SPR   RV  TB BP2 BP1 BP0 WEL   
//SPR:Ĭ��0,״̬�Ĵ�������λ,���WPʹ��
//TB,BP2,BP1,BP0:FLASH����д��������
//WEL:дʹ������
//BUSY:æ���λ(1,æ;0,����)
//Ĭ��:0x00
unsigned char  mt_flashReadSR(void)   
{  
	unsigned char  byte=0;   
	hal_spi2CSDrive(0);                            //ʹ������   
	hal_spi2ReadWriteByte(W25X_ReadStatusReg);    //���Ͷ�ȡ״̬�Ĵ�������    
	byte=hal_spi2ReadWriteByte(0Xff);             //��ȡһ���ֽ�  
	hal_spi2CSDrive(1);                             //ȡ��Ƭѡ     
	return byte;   
} 
//�ȴ�����
void mt_flashWaitBusy(void)   
{   
	while ((mt_flashReadSR()&0x01)==0x01);   // �ȴ�BUSYλ���
}  

//FLASH ��д����
//����ȷ����д�ĵ�ַ��Χ�ڵ�����ȫ��Ϊ0XFF,�����ڷ�0XFF��д������ݽ�ʧ��!
//�����Զ���ҳ���� 
//��ָ����ַ��ʼд��ָ�����ȵ�����,����Ҫȷ����ַ��Խ��!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���65535)
//CHECK OK
void mt_flashWrite_Secor(unsigned char * pBuffer,unsigned int WriteAddr,unsigned short NumByteToWrite)   
{ 			 		 
	unsigned char  *pBuff;  ////���ݵ�ַָ��
	unsigned short num;  
	unsigned int wAddr;  ///д����ʼ��ַ
	unsigned short pageremain;	
	pBuff = pBuffer;
	num = NumByteToWrite;
	wAddr = WriteAddr;
	pageremain=256-wAddr%256; //��ҳʣ����ֽ���		 	    
	if(num<=pageremain)
		pageremain=num;//������256���ֽ�
	while(1)
	{	   
		        mt_flashWritePage(pBuff,wAddr,pageremain);
			if(num==pageremain)
				break;//д�������
			else //NumByteToWrite>pageremain
			{
				pBuff+=pageremain;
				wAddr+=pageremain;	//200  56   100
				num-=pageremain;			  //��ȥ�Ѿ�д���˵��ֽ���
				if(num>256)
					pageremain=256; //һ�ο���д��256���ֽ�
				else 
					pageremain=num; 	  //����256���ֽ���
			}		
	}	    
} 

//дSPI FLASH  
//��ָ����ַ��ʼд��ָ�����ȵ�����
//�ú�������������!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���256)  
void mt_flashWrite(unsigned char * pBuffer,unsigned int WriteAddr,unsigned short NumByteToWrite)   
 { 
		 unsigned char  SPI_FLASH_BUF[4096];
//	   unsigned char testaa[200];
		 unsigned char  *pBuff;
	          unsigned int secpos;        ///��Ҫд����ʼ������
	           unsigned short secoff;      ///д�뵽����ʼ������ ƫ�Ƶ�ַ
	           unsigned short secremain;	 ///��һ��д��������Ҫд������ݵĸ���     
		  unsigned short i,num;  

		 unsigned int wAddr;
		 pBuff = pBuffer;
		 wAddr = WriteAddr;
		 num = NumByteToWrite;  ////
		 secpos=wAddr/4096;//������ַ        
		 secoff=wAddr%4096;//�������ڵ�ƫ��
		 secremain=4096-secoff;//����ʣ��ռ��С   
	   if(num<=secremain)  ///num  ����Ҫд�����ݵĸ�ʽ   �����Ҫд������ݵĸ���С�ڱ�����ʣ��ĸ���
			 secremain=num;//������4096���ֽ�   ��ͬһ��������д
		 while(1) 
		 {	
				 mt_flashRead(SPI_FLASH_BUF,secpos*4096,4096);//������������������
				 mt_flashEraseSector(secpos);//�����������
				 
				 for(i=0;i<secremain;i++)	   //����
				 {
				        SPI_FLASH_BUF[i+secoff]=pBuff[i];
				 }
				mt_flashWrite_Secor(SPI_FLASH_BUF,secpos*4096,4096);//д���������� д�Ѿ������˵�,ֱ��д������ʣ������. 				 

				 if(num==secremain)   ///��Ҫд������ݳ��Ⱥ� ���ݳ���һ�µĻ���
					 break;//д�������
				 
				 else//д��δ����
				 {
					 secpos++;//������ַ��1
					 secoff=0;//ƫ��λ��Ϊ0 	 

					 pBuff+=secremain;  //ָ��ƫ��
					 wAddr+=secremain;//д��ַƫ��	   
					 num-=secremain;				//�ֽ����ݼ�
					 if(num>4096)
						 secremain=4096;	//��һ����������д����
					 else 
						 secremain=num;			//��һ����������д����
				 }	 
		 }	 	 
 } 
 //����һ������
//Dst_Addr:������ַ 0~511 for w25x16
//����һ������������ʱ��:45ms,���300ms
void mt_flashEraseSector(unsigned int Dst_Addr)   
{   
	unsigned int DstAddr;
	DstAddr = Dst_Addr;
	DstAddr*=4096;
	mt_flashWriteEnable();                  //SET WEL 	 
	mt_flashWaitBusy();   
	hal_spi2CSDrive(0);                              //ʹ������   
	hal_spi2ReadWriteByte(W25X_SectorErase);      //������������ָ�� 
	hal_spi2ReadWriteByte((unsigned char )((DstAddr)>>16));  //����24bit��ַ    
	hal_spi2ReadWriteByte((unsigned char )((DstAddr)>>8));   
	hal_spi2ReadWriteByte((unsigned char )DstAddr);  
	hal_spi2CSDrive(1);                             //ȡ��Ƭѡ     	      
	mt_flashWaitBusy();   				   //�ȴ��������
}

/*unsigned char falshtest[6000];
void mt_flash_test(void)
{
	unsigned int i;
	unsigned int falshdadrx;
	falshdadrx = 4000;
	for(i=0;i< 6000;i++)
	{
		falshtest[i] = i;
  }		
	mt_flashWrite(&falshtest[0],falshdadrx,6000);
         for(i=0;i< 6000;i++)
	{
		falshtest[i] = 0;
       }	
  mt_flashRead(&falshtest[0],falshdadrx,6000);
}
*/

