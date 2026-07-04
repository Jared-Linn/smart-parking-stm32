#include "RC522.h" 
#include <stdio.h>

#define MAXRLEN 18

void Sent_Byte(unsigned char Sdata);

/**************************************************************
*功  能：		初始化端口
*参  数：		无
*返回值：		无 
**************************************************************/
void RC522IO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	//使能PORTA时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);			
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_7;
	//设置成推挽输出
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; 				
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	//初始化
 	GPIO_Init(GPIOA, &GPIO_InitStructure);							
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4;
	//设置成上拉输入
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
	//初始化
 	GPIO_Init(GPIOA, &GPIO_InitStructure);							
	
}

/**************************************************************
*功  能：		RC522 芯片初始化
*参  数：		无
*返回值：		无 
**************************************************************/
void InitRc522(void)
{
	  PcdReset();
	  PcdAntennaOff();
	  Delay_ms(2);  
	  PcdAntennaOn();
	  M500PcdConfigISOType( 'A' );
}

/**************************************************************
*功  能：		RC522的工作方式
*参  数：		工作方式
*返回值：		无 
**************************************************************/
char M500PcdConfigISOType(unsigned char type)
{
	//ISO14443_A
	if (type == 'A')                     
	{ 

		ClearBitMask(Status2Reg,0x08);
		//3F
		WriteRawRC(ModeReg,0x3D);
		//84
		WriteRawRC(RxSelReg,0x86);
		//4F
		WriteRawRC(RFCfgReg,0x7F);
		//tmoLength);// TReloadVal = 'h6a =tmoLength(dec) 
		WriteRawRC(TReloadRegL,30);
		WriteRawRC(TReloadRegH,0);
		WriteRawRC(TModeReg,0x8D);
		WriteRawRC(TPrescalerReg,0x3E);
		Delay_ms(10);
		PcdAntennaOn();
		WriteRawRC(SerialSpeedReg,0x7A);

	}
	else{ return -1; }
   
   return MI_OK;
}

/**************************************************************
*功  能：		寻卡
*参  数：		req_code[IN]:寻卡方式  
*				0x52 = 寻感应区内所有符合14443A标准的卡
*		 		0x26 = 寻未进入休眠状态的卡
*		 		pTagType[OUT]：卡片类型代码
*				0x4400 = Mifare_UltraLight
*		 		0x0400 = Mifare_One(S50)
*		 		0x0200 = Mifare_One(S70)
*		 		0x0800 = Mifare_Pro(X)
*		 		0x4403 = Mifare_DESFire
*返回值: 		成功返回MI_OK 
**************************************************************/
char PcdRequest(unsigned char req_code,unsigned char *pTagType)
{
   char status;  
   unsigned int  unLen;
   unsigned char ucComMF522Buf[MAXRLEN]; 

   ClearBitMask(Status2Reg,0x08);
   WriteRawRC(BitFramingReg,0x07);
   SetBitMask(TxControlReg,0x03);
 
   ucComMF522Buf[0] = req_code;

   status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen);
   
   if ((status == MI_OK) && (unLen == 0x10))
   {    
       *pTagType     = ucComMF522Buf[0];
       *(pTagType+1) = ucComMF522Buf[1];
   }
   else
   {   status = MI_ERR;   }
   
   return status;
}
  
/**************************************************************
*功  能：		防冲撞
*参  数：		pSnr[OUT]:卡片序列号，4字节
*返回值：		成功返回MI_OK 
**************************************************************/
char PcdAnticoll(unsigned char *pSnr)
{
    char status;
    unsigned char i,snr_check=0;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    

    ClearBitMask(Status2Reg,0x08);
    WriteRawRC(BitFramingReg,0x00);
    ClearBitMask(CollReg,0x80);
 
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,2,ucComMF522Buf,&unLen);
	//已经给了命令，传送防冲撞数据过去，再接受4次，这个时候ucComMF522Buf已经有数据
    if (status == MI_OK)
    {
    	 for (i=0; i<4; i++)
         {   
			 //利用指针依次放入4位卡的序列号
             *(pSnr+i)  = ucComMF522Buf[i];
             snr_check ^= ucComMF522Buf[i];
         }
         if (snr_check != ucComMF522Buf[i])
         {   status = MI_ERR;    }
    }
    
    SetBitMask(CollReg,0x80);
    return status;
}

/**************************************************************
*功  能：		选定卡片
*参  数：		pSnr[IN]:卡片序列号，4字节
*返回值：		成功返回MI_OK
**************************************************************/
char PcdSelect(unsigned char *pSnr)
{
    char status;
    unsigned char i;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i=0; i<4; i++)
    {
    	ucComMF522Buf[i+2] = *(pSnr+i);
		//0放入防冲撞命令，1写入0x70不明白,2~5写入4位卡的序列号，6是0
    	ucComMF522Buf[6]  ^= *(pSnr+i);	
    }
	//计算CRC密码
    CalulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);	
  
    ClearBitMask(Status2Reg,0x08);			

	//传送被CRC计算后的数据
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);
    
    if ((status == MI_OK) && (unLen == 0x18))
    {   status = MI_OK;  }
    else
    {   status = MI_ERR;    }

    return status;
}
           
/**************************************************************
*功  能：		验证卡片密码
*参  数：		auth_mode[IN]: 密码验证模式 0x60 = 验证A密钥 0x61 = 验证B密钥
*		        addr[IN]：块地址   pKey[IN]：密码  pSnr[IN]：卡片序列号，4字节
*返回值：		成功返回MI_OK
**************************************************************/
char PcdAuthState(unsigned char auth_mode,unsigned char addr,unsigned char *pKey,unsigned char *pSnr)
{
    char status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
    for (i=0; i<6; i++)
    {    ucComMF522Buf[i+2] = *(pKey+i);   }
    for (i=0; i<6; i++)
    {    ucComMF522Buf[i+8] = *(pSnr+i);   }
    
    status = PcdComMF522(PCD_AUTHENT,ucComMF522Buf,12,ucComMF522Buf,&unLen);
    if ((status != MI_OK) || (!(ReadRawRC(Status2Reg) & 0x08)))
    {   status = MI_ERR;   }   
    return status;
}

/**************************************************************
*功  能：		读取M1卡一块数据
*参  数：		addr[IN]：块地址 pData[OUT]：读出的数据，16字节
*返回值：		成功返回MI_OK
**************************************************************/
char PcdRead(unsigned char addr,unsigned char *pData)
{
    char status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
   
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
    if ((status == MI_OK) && (unLen == 0x90))
 //   {   memcpy(pData, ucComMF522Buf, 16);   }
    {
        for (i=0; i<16; i++)
        {    *(pData+i) = ucComMF522Buf[i];   }
    }
    else
    {   status = MI_ERR;   }
    
    return status;
}
  
/**************************************************************
*功  能：		写数据到M1卡一块
*参  数：		addr[IN]：块地址  pData[IN]：写入的数据，16字节
*返回值：		成功返回MI_OK
**************************************************************/
char PcdWrite(unsigned char addr,unsigned char *pData)
{
    char status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }
        
    if (status == MI_OK)
    {
        //memcpy(ucComMF522Buf, pData, 16);
        for (i=0; i<16; i++)
        {    ucComMF522Buf[i] = *(pData+i);   }
        CalulateCRC(ucComMF522Buf,16,&ucComMF522Buf[16]);

        status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,18,ucComMF522Buf,&unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {   status = MI_ERR;   }
    }
    
    return status;
}

/**************************************************************
*功  能：		命令卡片进入休眠状态
*参  数：		无
*返回值：		成功返回MI_OK
**************************************************************/
char PcdHalt(void)
{
    char status;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    return MI_OK;
}

/**************************************************************
*功  能：		用MF522计算CRC16函数
*参  数：		无
*返回值：		无 
**************************************************************/
void CalulateCRC(unsigned char *pIndata,unsigned char len,unsigned char *pOutData)
{
    unsigned char i,n;
    ClearBitMask(DivIrqReg,0x04);
    WriteRawRC(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);
    for (i=0; i<len; i++)
    {   WriteRawRC(FIFODataReg, *(pIndata+i));   }
    WriteRawRC(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do 
    {
        n = ReadRawRC(DivIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x04));
    pOutData[0] = ReadRawRC(CRCResultRegL);
    pOutData[1] = ReadRawRC(CRCResultRegM);
}

/**************************************************************
*功  能：		复位RC522
*参  数：		无
*返回值：		成功返回MI_OK
**************************************************************/
char PcdReset(void)
{
    MF522_RST(1);
    Delay_us(3);
    MF522_RST(0);
    Delay_us(3);
    MF522_RST(1);
    Delay_us(3);
    WriteRawRC(CommandReg,PCD_RESETPHASE);
    Delay_us(3);
    
    WriteRawRC(ModeReg,0x3D);
	//和Mifare卡通讯，CRC初始值0x6363
    WriteRawRC(TReloadRegL,30);           
    WriteRawRC(TReloadRegH,0);
    WriteRawRC(TModeReg,0x8D);
    WriteRawRC(TPrescalerReg,0x3E);
    WriteRawRC(TxAskReg,0x40);
   
    return MI_OK;
}

/**************************************************************
*功  能：		读RC632寄存器
*参  数：		Address[IN]:寄存器地址
*返回值：		读出的值
**************************************************************/
unsigned char ReadRawRC(unsigned char Address)
{
     unsigned char i, ucAddr;
     unsigned char ucResult=0;

     MF522_SCK(0);
     MF522_NSS(0);
     ucAddr = ((Address<<1)&0x7E)|0x80;

     for(i=8;i>0;i--)
     {
         MF522_SI(((ucAddr&0x80)==0x80));
		 Delay_us(1);
         MF522_SCK(1);
		 Delay_us(1);
         ucAddr <<= 1;
		 Delay_us(1);
         MF522_SCK(0);
		 Delay_us(1);
     }

     for(i=8;i>0;i--)
     {
         MF522_SCK(1);
		 Delay_us(1);
         ucResult <<= 1;
		 Delay_us(1);
         ucResult |= (MF522_SO() & 0x01);
		 Delay_us(1);
         MF522_SCK(0);
		 Delay_us(1);
     }

     MF522_NSS(1);
     MF522_SCK(1);
     return ucResult;
}

/**************************************************************
*功  能：		写RC632寄存器
*参  数：		Address[IN]:寄存器地址  value[IN]:写入的值
*返回值：		无 
**************************************************************/
void WriteRawRC(unsigned char Address, unsigned char value)
{  
    unsigned char i, ucAddr;

    MF522_SCK(0);
    MF522_NSS(0);
    ucAddr = ((Address<<1)&0x7E);

    for(i=8;i>0;i--)
    {
        MF522_SI (((ucAddr&0x80)==0x80));
		Delay_us(1);
        MF522_SCK(1);
		Delay_us(1);
        ucAddr <<= 1;
		Delay_us(1);
        MF522_SCK(0);
		Delay_us(1);
    }

    for(i=8;i>0;i--)
    {
        MF522_SI(((value&0x80)==0x80));
		Delay_us(1);
        MF522_SCK(1);
		Delay_us(1);
        value <<= 1;
		Delay_us(1);
        MF522_SCK(0);
		Delay_us(1);
    }
    MF522_NSS(1);
    MF522_SCK(1);
}

/**************************************************************
*功  能：		置RC522寄存器位
*参  数：		reg[IN]:寄存器地址  mask[IN]:置位值
*返回值：		无 
**************************************************************/
void SetBitMask(unsigned char reg,unsigned char mask)  
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);
	// set bit mask
    WriteRawRC(reg,tmp | mask);  
}

/**************************************************************
*功  能：		清RC522寄存器位
*参  数：		reg[IN]:寄存器地址  mask[IN]:清位值
*返回值：		无 
**************************************************************/
void ClearBitMask(unsigned char reg,unsigned char mask)  
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);
	// clear bit mask
    WriteRawRC(reg, tmp & ~mask);  
} 

/**************************************************************
*功  能：		通过RC522和ISO14443卡通讯
*参  数：		Command[IN]:RC522命令字
*		        pInData[IN]:通过RC522发送到卡片的数据
*		        InLenByte[IN]:发送数据的字节长度
*		        pOutData[OUT]:接收到的卡片返回数据
*		        *pOutLenBit[OUT]:返回数据的位长度
*返回值：		无 
**************************************************************/
char PcdComMF522(unsigned char Command, 
                 unsigned char *pInData, 
                 unsigned char InLenByte,
                 unsigned char *pOutData, 
                 unsigned int  *pOutLenBit)
{
    char Rc522_status = MI_ERR;
    unsigned char irqEn   = 0x00;
    unsigned char waitFor = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;
    switch (Command)
    {
		//验证秘钥
       case PCD_AUTHENT:		
          irqEn   = 0x12;
          waitFor = 0x10;
          break;
	   //复位并且发送并且接受数据
       case PCD_TRANSCEIVE:		
          irqEn   = 0x77;
          waitFor = 0x30;
          break;
       default:
         break;
    }
   
    WriteRawRC(ComIEnReg,irqEn|0x80);
    ClearBitMask(ComIrqReg,0x80);
    WriteRawRC(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);
    
    for (i=0; i<InLenByte; i++)
    {   WriteRawRC(FIFODataReg, pInData[i]);    }
    WriteRawRC(CommandReg, Command);
   
   //================= 
    if (Command == PCD_TRANSCEIVE)
    {    SetBitMask(BitFramingReg,0x80);  }
    
	//根据时钟频率调整，操作M1卡最大等待时间25ms
    i = 600;					
    do 
    {
         n = ReadRawRC(ComIrqReg);
         i--;
    }
    while ((i!=0) && !(n&0x01) && !(n&waitFor));
    ClearBitMask(BitFramingReg,0x80);
	      
    if (i!=0)
    {    
         if(!(ReadRawRC(ErrorReg)&0x1B))
         {
             Rc522_status = MI_OK;
             if (n & irqEn & 0x01)
             {   Rc522_status = MI_NOTAGERR;   }
             if (Command == PCD_TRANSCEIVE)
             {
               	n = ReadRawRC(FIFOLevelReg);
              	lastBits = ReadRawRC(ControlReg) & 0x07;

			
                if (lastBits)
                {   *pOutLenBit = (n-1)*8 + lastBits;   }
                else
                {   *pOutLenBit = n*8;   }
                if (n == 0)
                {   n = 1;    }
                if (n > MAXRLEN)
                {   n = MAXRLEN;   }
                for (i=0; i<n; i++)
                {   pOutData[i] = ReadRawRC(FIFODataReg);    }
            }
         }
         else
         {   Rc522_status = MI_ERR;   }
        
   }
   SetBitMask(ControlReg,0x80);           
   WriteRawRC(CommandReg,PCD_IDLE); 
   return Rc522_status;
}

/**************************************************************
*功  能：		开启天线（每次启动或关闭天险发射之间应至少有1ms的间隔）
*参  数：		无
*返回值：		无 
**************************************************************/
void PcdAntennaOn()
{
    unsigned char i;
    i = ReadRawRC(TxControlReg);
    if (!(i & 0x03))
    {
        SetBitMask(TxControlReg, 0x03);
    }
}

/**************************************************************
*功  能：		关闭天线
*参  数：		无
*返回值：		无 
**************************************************************/
void PcdAntennaOff()
{
    ClearBitMask(TxControlReg, 0x03);
}

/**************************************************************
*功  能：		初始化串口、读卡器
*参  数：		无
*返回值：		无 
**************************************************************/
void Readcard_daivce_Init()
{
	//串口初始化
	RC522IO_Init();		
	Delay_ms(100);
	//读卡器初始化
	InitRc522();		
	Delay_ms(1000);
	RC522IO_Init();
	Delay_ms(100);
}
/**************************************************************
*功  能：		读卡函数
*参  数：		无
*返回值：		无 
**************************************************************/
void Read_Card(void)
{
	char str[50];
    uint8_t ucArray_ID [4]; 
	char Rc522_status = 0;
	//卡类型
	uint8_t CT[2];
	//卡号
	uint8_t SN[4];
	//密钥
	uint8_t KEY[6]={0xff,0xff,0xff,0xff,0xff,0xff};
	//扇区0  存有卡号ID	
	uint8_t s = 0x00; 
	//卡号高位
	uint8_t RXRFIDH[8];								
	uint8_t i = 0;
	uint8_t RXRFID[16];	

	//寻卡
	Rc522_status = PcdRequest(PICC_REQALL,CT);
	//寻卡成功
	if(Rc522_status == MI_OK)								
	{
		Rc522_status=MI_ERR;
		CT[0] = CT[0] + 0x30;
		CT[1] = CT[1] + 0x30;

		//防冲撞
		Rc522_status = PcdAnticoll(SN);					
		if(Rc522_status == MI_OK)
		{
			Rc522_status=MI_ERR;
		
			//选定此卡
			Rc522_status =PcdSelect(SN);
			//选定成功
			if(Rc522_status == MI_OK)						
			{
				Rc522_status=MI_ERR;
				
				//验证密钥
				Rc522_status =PcdAuthState(0x60,s,KEY,SN);
				if(Rc522_status == MI_OK)
				{
					Rc522_status = MI_ERR;
																		
					//读卡
					Rc522_status=PcdRead(s,RXRFID);		
					if(Rc522_status == MI_OK)
					{
						Rc522_status = MI_ERR;
						OLED_Show_Str(16*0,4,"                ",16);
						OLED_Show_Str(16*2,4,"读卡成功",16);
						Delay_ms(500);
						Delay_ms(500);
						OLED_Show_Str(16*2,4,"ID号如下：",16);
						Delay_ms(500);
						Delay_ms(500);
						sprintf(str,"%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",RXRFID[0],RXRFID[1],RXRFID[2],RXRFID[3],RXRFID[4],
																									   RXRFID[5],RXRFID[6],RXRFID[7],RXRFID[8],RXRFID[9],
																									   RXRFID[10],RXRFID[11],RXRFID[12],RXRFID[13],RXRFID[14],
																									   RXRFID[15]
																									  );
						OLED_Show_Str(16*0,4,str,16);	
				
					}	
	                     Delay_ms(500);
                    	 Delay_ms(500);
					     Delay_ms(500);
						 OLED_Show_Str(16*0,4,"                ",16);
                         OLED_Show_Str(16*0,6,"                ",16);	
                         OLED_Show_Str(20,4,"等待ID识别",16);					
				}
			}
		}
	}	
}
//										endfile