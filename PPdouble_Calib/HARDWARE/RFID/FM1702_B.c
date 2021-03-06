#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "FM1702.h"
#include "usart.h"
#include "delay.h"
#include "string.h"
#include "spi.h"
#include "DataFlash.h"
#include "can.h"
#define uchar unsigned char

//uchar       cardtype;
uchar     	tagtype[2];	        /* 卡片标识字符 */
/* FM1702变量定义 */
volatile unsigned char     	buf_B[10];            /* FM1702命令发送接收缓冲区 */
unsigned char buffer_B[16];
unsigned char buffer_temp_1st[16];
uchar     	UID[5];             /* 序列号 */
unsigned char DefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
//uchar       Secnr;			        /* 扇区号 */
uchar RFID_status_1st;  /*锁孔状态标识, 1--有钥匙 0--无钥匙*/


//SPI接收
 unsigned char rev_B(uchar tem)
{
  return SPI1_ReadWriteByte(tem);
} 
//SPI发送
void Send_B(unsigned char var) 

{ 
	SPI1_ReadWriteByte(var);
}                  

//读寄存器
 uchar read_reg_B(uchar SpiAddress)
{
	uchar rdata;
	SpiAddress=SpiAddress<<1;
	SpiAddress=SpiAddress | 0x80; 
	delay_us(30);
	NSS_L;
	NSS_H_2nd;
  delay_us(30);
	Send_B(SpiAddress);
	rdata=rev_B(0);
	NSS_H;
  NSS_H_2nd;	
	return(rdata);
}

//写寄存器
 void write_reg_B(uchar SpiAddress,uchar dat)
{
	SpiAddress = SpiAddress << 1;
	SpiAddress = SpiAddress & 0x7f;
	delay_us(30);
	NSS_L;
  NSS_H_2nd;	
	delay_us(30);	
	Send_B(SpiAddress);		
	Send_B(dat);	
	NSS_H;
	NSS_H_2nd;
}

/****************************************************************/
/*名称: Clear_FIFO_B                                              */
/*功能: 该函数实现清縁FIFO的数据                                */
/*												       			                          */
/*输入:                                                         */
/*      N/A                                                     */
/*                                                              */
/*输出:                                                         */
/*	    TRUE, FIFO被清空                                        */
/*	    FALSE, FIFO未被清空  	                                  */
/****************************************************************/
 uchar Clear_FIFO_B(void)
{
	uchar temp;
	uint  i;
	
	temp =read_reg_B(Control);						//清空FIFO
	temp = (temp | 0x01);
	write_reg_B(Control, temp);
	for(i = 0; i < RF_TimeOut; i++)			//检查FIFO是否被清空
	{
		temp = read_reg_B(FIFO_Length);
		if(temp == 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}

/****************************************************************/
/*名称: Write_FIFO_B                                              */
/*功能: 该函数实现向RC531的FIFO中写入x bytes数据                */
/*												       			                          */
/*输入:                                                         */
/*      count, 待写入字节的长度                                 */
/*	    buff, 指向待写入数据的指针                              */
/*                                                              */
/*输出:                                                         */
/*	    N/A                                                 		*/
/****************************************************************/
 void Write_FIFO_B(uchar count,volatile unsigned char *buff)
{
	uchar i;
	
	for(i = 0; i < count; i++)
	{
		write_reg_B(FIFO,*(buff + i));
	}
}

/****************************************************************/
/*名称: Read_FIFO_B                                               */
/*功能: 该函数实现从RC531的FIFO中读出x bytes数据                */
/*												        		                          */
/*输入:                                                         */
/*       buff, 指向读出数据的指针                               */
/*                                                              */
/*输出:                                                         */
/*	     N/A                                                 		*/
/****************************************************************/
uchar Read_FIFO_B(volatile unsigned char *buff)
{
	uchar temp;
	uchar i;
	
	temp =read_reg_B(FIFO_Length);
	if (temp == 0)
	{
		return 0;
	}
	if (temp >= 24)						//temp=255时,会进入死循环
	{									        //因此增加FIFO_Length越限判断
		temp = 24;						 
	}
	for(i = 0;i < temp; i++)
	{
 		*(buff + i) =read_reg_B(FIFO);
	}
	return temp;
 }

/****************************************************************/
/*名称: Judge_Req_B                                               */
/*功能: 该函数实现对卡片复位应答信号的判断                      */
/*												       			                          */
/*输入:                                                         */
/*       *buff, 指向应答数据的指针                              */
/*                                                              */
/*输出:                                                         */
/*	     TRUE, 卡片应答信号正确                                 */
/*       FALSE, 卡片应答信号错误                                */
/****************************************************************/
 uchar Judge_Req_B(volatile unsigned char *buff)
{
	uchar temp1,temp2;
	
	temp1 = *buff;
	temp2 = *(buff + 1);

	if((temp1 != 0x00) && (temp2 == 0x00))
	{
		return TRUE;
	}
	return FALSE;
}

/****************************************************************/
/*名称: Check_UID_B                                               */
/*功能: 该函数实现对收到的卡片的序列号的判断                    */
/*输入: N/A                                                     */
/*输出: TRUE: 序列号正确                                        */
/* FALSE: 序列号错误                                            */
/****************************************************************/
 uchar Check_UID_B(void)
{
	uchar	temp;
	uchar	i;

	temp = 0x00;
	for(i = 0; i < 5; i++)
	{
		temp = temp ^ UID[i];
	}

	if(temp == 0)
	{
		PR("1st Check UID_OK\r\n");
		return TRUE;
	}

	return FALSE;
}

/****************************************************************/
/*名称: Save_UID_B                                                */
/*功能: 该函数实现保存卡片收到的序列号                          */
/*输入: row: 产生冲突的行                                       */
/* col: 产生冲突的列                                            */
/* length: 接収到的UID数据长度                                  */
/*输出: N/A                                                     */
/****************************************************************/
 void Save_UID_B(uchar row, uchar col, uchar length)
{
	uchar	i;
	uchar	temp;
	uchar	temp1;

	if((row == 0x00) && (col == 0x00))
	{
		for(i = 0; i < length; i++)
		{
			UID[i] = buf_B[i];
		}
	}
	else
	{
		temp = buf_B[0];
		temp1 = UID[row - 1];
		switch(col)
		{
		case 0:		temp1 = 0x00; row = row + 1; break;
		case 1:		temp = temp & 0xFE; temp1 = temp1 & 0x01; break;
		case 2:		temp = temp & 0xFC; temp1 = temp1 & 0x03; break;
		case 3:		temp = temp & 0xF8; temp1 = temp1 & 0x07; break;
		case 4:		temp = temp & 0xF0; temp1 = temp1 & 0x0F; break;
		case 5:		temp = temp & 0xE0; temp1 = temp1 & 0x1F; break;
		case 6:		temp = temp & 0xC0; temp1 = temp1 & 0x3F; break;
		case 7:		temp = temp & 0x80; temp1 = temp1 & 0x7F; break;
		default:	break;
		}

		buf_B[0] = temp;
		UID[row - 1] = temp1 | temp;
		for(i = 1; i < length; i++)
		{
			UID[row - 1 + i] = buf_B[i];
		}
	}
}

/****************************************************************/
/*名称: Set_BitFraming_B                                          */
/*功能: 该函数设置待发送数据的字节数                            */
/*输入: row: 产生冲突的行                                       */
/*      col: 产生冲突的列                                       */
/*输出: N/A                                                     */
/****************************************************************/
 void Set_BitFraming_B(uchar row, uchar col)
{
	switch(row)
	{
	case 0:		buf_B[1] = 0x20; break;
	case 1:		buf_B[1] = 0x30; break;
	case 2:		buf_B[1] = 0x40; break;
	case 3:		buf_B[1] = 0x50; break;
	case 4:		buf_B[1] = 0x60; break;
	default:	break;
	}

	switch(col)
	{
	case 0:		write_reg_B(0x0F,0x00);  break;
	case 1:		write_reg_B(0x0F,0x11); buf_B[1] = (buf_B[1] | 0x01); break;
	case 2:		write_reg_B(0x0F,0x22); buf_B[1] = (buf_B[1] | 0x02); break;
	case 3:		write_reg_B(0x0F,0x33); buf_B[1] = (buf_B[1] | 0x03); break;
	case 4:		write_reg_B(0x0F,0x44); buf_B[1] = (buf_B[1] | 0x04); break;
	case 5:		write_reg_B(0x0F,0x55); buf_B[1] = (buf_B[1] | 0x05); break;
	case 6:		write_reg_B(0x0F,0x66); buf_B[1] = (buf_B[1] | 0x06); break;
	case 7:		write_reg_B(0x0F,0x77); buf_B[1] = (buf_B[1] | 0x07); break;
	default:	break;
	}
}

/****************************************************************/
/*名称: FM1702_Bus_Sel_B                                          */
/*功能: 该函数实现对FM1702操作的总线方式(并行总线,SPI)选择      */ 
/*												       			                          */
/*输入:                                                         */
/*      N/A                                                     */
/*                                                              */
/*输出:                                                         */
/*	    TRUE,  总线选择成功                                     */
/*	    FALSE, 总线选择失败  	                                  */
/****************************************************************/
uchar FM1702_Bus_Sel_B(void)
{
	uchar i,temp;
	
	write_reg_B(Page_Sel,0x80);
	write_reg_B(0x08,0x80);
	write_reg_B(0x10,0x80);
	write_reg_B(0x18,0x80);
	write_reg_B(0x20,0x80);
	write_reg_B(0x28,0x80);
	write_reg_B(0x30,0x80);
	write_reg_B(0x38,0x80);	
	for(i = 0; i < RF_TimeOut; i++)
	{
		temp=read_reg_B(Command);
		if(temp == 0x00)
		{
			write_reg_B(Page_Sel,0x00);
			write_reg_B(0x08,0x00);
			write_reg_B(0x10,0x00);
			write_reg_B(0x18,0x00);
			write_reg_B(0x20,0x00);
			write_reg_B(0x28,0x00);
			write_reg_B(0x30,0x00);
			write_reg_B(0x38,0x00);				
			return TRUE;
		}
	}
	return FALSE;
}


/****************************************************************/
/*名称: Init_FM1702_1st                                          */
/*功能: 该函数实现对FM1702初始化                                */ 
/*												       			                          */
/*输入:                                                         */
/*      N/A                                                     */
/*                                                              */
/*输出:                                                         */
/*	    TRUE,  总线选择成功                                     */
/*	    FALSE, 总线选择失败  	                                  */
/****************************************************************/
uchar Init_FM1702_1st(void)
{	
  uchar temp ;
	unsigned char out[16];
	FLASH_ReadByte(ADDR_Check_B,out,16);
	delay_ms(10);
  if(out[2]==0xff)
	{
		out[15] = 0x6f;
	}
	NSS_L;
	RST_H;				        //复位�
	delay_ms(50);
	RST_L;
	delay_ms(50);
/*新的初始化*/	
	temp = read_reg_B(0x05);
	delay_ms(50);
	temp = FM1702_Bus_Sel_B();		 	//总线选择
  if (temp == TRUE)
	{
		write_reg_B(CWConductance, 0x3F);
		write_reg_B(RxControl1, 0x73);
		write_reg_B(Rxcontrol2, 0x01);
	//	write_reg_B(ModWidth, 0x13);	
		write_reg_B(RxThreshold, out[15]);
//		write_reg_B(RxWait, 0x06);
		return TRUE;
	}	
	return FALSE;
}

/****************************************************************/
/*名称: Command_Send_B                                            */
/*功能: 该函数实现向RC531发送命令集的功能                       */
/*												        		                          */
/*输入:                                                         */
/*       count, 待发送命令集的长度                              */
/*	     buff, 指向待发送数据的指针                             */
/*       Comm_Set, 命令码                                       */
/*												       			                          */
/*输出:                                                         */
/*	     TRUE, 命令被正确执行                                   */
/*	     FALSE, 命令执行错误  	                                */
/****************************************************************/
uchar Command_Send_B(uchar count,volatile unsigned char * buff,uchar Comm_Set)
{
	uint  j;
	uchar temp;
	
	write_reg_B(Command, 0x00);
	Clear_FIFO_B();
  if (count != 0)
  {
	  Write_FIFO_B(count, buff);
  }	 
	write_reg_B(Command, Comm_Set);					//命令执行
	
	for(j = 0; j< RF_TimeOut; j++)				//检查命令执行否
	{
		temp =read_reg_B(Command);
		if(temp == 0x00)  
		{
			return TRUE;
		}
	}
	return FALSE;	
}

/****************************************************************/
/*名称: MIF_Halt                                                */
/*功能: 该函数实现暂停MIFARE卡                                  */
/*输入: N/A                                                     */
/*输出: FM1702_OK: 应答正确                                     */
/* FM1702_PARITYERR: 奇偶校验错                                 */
/* FM1702_CRCERR: CRC校验错                                     */
/* FM1702_NOTAGERR: 无卡                                        */
/****************************************************************/
/*
uchar MIF_Halt(void)
{
	uchar	temp;
	uint	i;

	write_reg_B(0x23,0x63);
	write_reg_B(0x12,0x3f);
	write_reg_B(0x22,0x07);
	*buf_B = RF_CMD_HALT;
	*(buf_B + 1) = 0x00;
	temp = Command_Send_B(2, buf_B, Transmit);
	if(temp == TRUE)
	{
		for(i = 0; i < 0x50; i++)
		{
			_nop_();
		}

		return FM1702_OK;
	}
	else
	{
		temp = read_reg_B(0x0A);
		if((temp & 0x02) == 0x02)
		{
			return(FM1702_PARITYERR);
		}

		if((temp & 0x04) == 0x04)
		{
			return(FM1702_FRAMINGERR);
		}

		return(FM1702_NOTAGERR);
	}
}
*/
///////////////////////////////////////////////////////////////////////
// 转换密钥格式
///////////////////////////////////////////////////////////////////////
char M500HostCodeKey_B(unsigned char *uncoded, unsigned char *coded)   
{
    unsigned char cnt = 0;
    unsigned char ln  = 0;     
    unsigned char hn  = 0;      
    
    for (cnt = 0; cnt < 6; cnt++)
    {
        ln = uncoded[cnt] & 0x0F;
        hn = uncoded[cnt] >> 4;
        coded[cnt * 2 + 1] = (~ln << 4) | ln;
        coded[cnt * 2 ] = (~hn << 4) | hn;
    }
    return FM1702_OK;
}

/****************************************************************/
/*名称: Load_keyE2                                              */
/*功能: 该函数实现把E2中密码存入FM1702的keybuf中             */
/*输入: Secnr: EE起始地址                                       */
/*输出: True: 密钥装载成功                                      */
/* False: 密钥装载失败                                          */
/****************************************************************/
uchar Load_keyE2_CPY(uchar *uncoded_keys)
{
  uchar temp;
  uchar coded_keys[13];
    
  M500HostCodeKey_B(uncoded_keys, coded_keys);
	temp = Command_Send_B(12, coded_keys, LoadKey);
	temp = read_reg_B(0x0A) & 0x40;
	if (temp == 0x40)
	{
		return FALSE;
	}
	return TRUE;
}

/****************************************************************/
/*名称: Request                                                 */
/*功能: 该函数实现对放入RC531操作范围之内的卡片的Request操作    */ 
/*												       			                          */
/*输入:                                                         */
/*      mode: ALL(监测所以RC531操作范围之内的卡片)			   	    */
/*	    STD(监测在RC531操作范围之内处于HALT状态的卡片)          */
/*                                                              */
/*输出:                                                         */
/*	    FM222_NOTAGERR: 无卡                                    */
/*      FM222_OK: 应答正确                                      */
/*	    FM222_REQERR: 应答错误										              */
/****************************************************************/
uchar Request(uchar mode)
{
		write_reg_B(Command, 0x00);
		Clear_FIFO_B();	  
    uchar  temp;
    write_reg_B(TxControl,0x58);
    delay_us(10);
    write_reg_B(TxControl,0x5b);		 
    write_reg_B(CRCPresetLSB,0x63);
    write_reg_B(CWConductance,0x3f);
    buf_B[0] = mode;					             //Request模式选择
    write_reg_B(Bit_Frame,0x07);			       //发送7bit
    write_reg_B(ChannelRedundancy,0x03);	   //关闭CRC
    write_reg_B(TxControl,0x5b); 
    write_reg_B(Control,0x01);          		 //屏蔽CRYPTO1位
    temp = Command_Send_B(1, buf_B, Transceive);
    if(temp == FALSE)
    {

	    return FM1702_NOTAGERR;
    }	
	
    Read_FIFO_B(buf_B);					           //从FIFO中读取应答信息
    temp = Judge_Req_B(buf_B);			         //判断应答信号是否正确
    if (temp == TRUE)
    {
      tagtype[0] = buf_B[0];
      tagtype[1] = buf_B[1];
      return FM1702_OK;
    }

		return FM1702_REQERR;
		
}

/****************************************************************/
/*名称: AntiColl                                                */
/*功能: 该函数实现对放入FM1702操作范围之内的卡片的防冲突检测    */
/*输入: N/A                                                     */
/*输出: FM1702_NOTAGERR: 无卡                                   */
/* FM1702_BYTECOUNTERR: 接收字节错误                            */
/* FM1702_SERNRERR: 卡片序列号应答错误                          */
/* FM1702_OK: 卡片应答正确                                      */
/****************************************************************/
uchar AntiColl(void)
{
	uchar	temp;
	uchar	i;
	uchar	row, col;
	uchar	pre_row;

	row = 0;
	col = 0;
	pre_row = 0;
	write_reg_B(0x23,0x63);
	write_reg_B(0x12,0x3f);
	write_reg_B(0x13,0x3f);
	buf_B[0] = RF_CMD_ANTICOL;
	buf_B[1] = 0x20;
	write_reg_B(0x22,0x03);	                   // 关闭CRC,打开奇偶校验
	temp = Command_Send_B(2, buf_B, Transceive);
	while(1)
	{
		if(temp == FALSE)
		{
			return(FM1702_NOTAGERR);
		}

		temp = read_reg_B(0x04);
		if(temp == 0)
		{
			return FM1702_BYTECOUNTERR;
		}

		Read_FIFO_B(buf_B);
		Save_UID_B(row, col, temp);			        // 将收到的UID放入UID数组中
	
		temp = read_reg_B(0x0A);				        // 判断接収数据是否出错
		temp = temp & 0x01;
		if(temp == 0x00)
		{
			temp = Check_UID_B();			            // 校验收到的UID
			if(temp == FALSE)
			{
				return(FM1702_SERNRERR);
			}
			return(FM1702_OK);
		}
		else
		{
			temp = read_reg_B(0x0B);             // 读取冲突检测寄存器 
			row = temp / 8;
			col = temp % 8;
			buf_B[0] = RF_CMD_ANTICOL;
			Set_BitFraming_B(row + pre_row, col);	// 设置待发送数据的字节数 
			pre_row = pre_row + row;
			for(i = 0; i < pre_row + 1; i++)
			{
				buf_B[i + 2] = UID[i];
			}

			if(col != 0x00)
			{
				row = pre_row + 1;
			}
			else
			{
				row = pre_row;
			}
			temp = Command_Send_B(row + 2, buf_B, Transceive);
		}
	}
}

/****************************************************************/
/*名称: Select_Card                                             */
/*功能: 该函数实现对放入FM1702操作范围之内的某张卡片进行选择    */
/*输入: N/A                                                     */
/*输出: FM1702_NOTAGERR: 无卡                                   */
/* FM1702_PARITYERR: 奇偶校验错                                 */
/* FM1702_CRCERR: CRC校验错                                     */
/* FM1702_BYTECOUNTERR: 接收字节错误                            */
/* FM1702_OK: 应答正确                                          */
/* FM1702_SELERR: 选卡出错                                      */
/****************************************************************/
uchar Select_Card(void)
{
	uchar	temp, i;

	write_reg_B(0x23,0x63);
	write_reg_B(0x12,0x3f);
	buf_B[0] = RF_CMD_SELECT;
	buf_B[1] = 0x70;
	for(i = 0; i < 5; i++)
	{
		buf_B[i + 2] = UID[i];
	}

	write_reg_B(0x22,0x0f);	                       // 开启CRC,奇偶校验校验 
	temp = Command_Send_B(7, buf_B, Transceive);
	if(temp == FALSE)
	{
		return(FM1702_NOTAGERR);
	}
	else
	{
		temp = read_reg_B(0x0A);
		if((temp & 0x02) == 0x02) return(FM1702_PARITYERR);
		if((temp & 0x04) == 0x04) return(FM1702_FRAMINGERR);
		if((temp & 0x08) == 0x08) return(FM1702_CRCERR);
		temp = read_reg_B(0x04);
		if(temp != 1) return(FM1702_BYTECOUNTERR);
		Read_FIFO_B(buf_B);	                      // 从FIFO中读取应答信息 
		temp = *buf_B;
		//判断应答信号是否正确 
		if((temp == 0x08) || (temp == 0x88) || (temp == 0x53) ||(temp == 0x18)) //S70 temp = 0x18	
		{
			return(FM1702_OK);
		}
		else
			return(FM1702_SELERR);
	}
}

/****************************************************************/
/*名称: Authentication                                          */
/*功能: 该函数实现密码认证的过程                                */
/*输入: UID: 卡片序列号地址                                     */
/* SecNR: 扇区号                                                */
/* mode: 模式                                                   */
/*输出: FM1702_NOTAGERR: 无卡                                   */
/* FM1702_PARITYERR: 奇偶校验错                                 */
/* FM1702_CRCERR: CRC校验错                                     */
/* FM1702_OK: 应答正确                                          */
/* FM1702_AUTHERR: 权威认证有错                                 */
/****************************************************************/
uchar Authentication(uchar *UID, uchar SecNR, uchar mode)
{
	uchar	i;
	uchar	temp, temp1;

	write_reg_B(0x23,0x63);
	write_reg_B(0x12,0x3f);
	write_reg_B(0x13,0x3f);
	temp1 = read_reg_B(0x09);
	temp1 = temp1 & 0xf7;
	write_reg_B(0x09,temp1);
	if(mode == RF_CMD_AUTH_LB)			            // AUTHENT1 
		buf_B[0] = RF_CMD_AUTH_LB;
	else
		buf_B[0] = RF_CMD_AUTH_LA;
	buf_B[1] = SecNR * 4 + 3;
	for(i = 0; i < 4; i++)
	{
		buf_B[2 + i] = UID[i];
	}

	write_reg_B(0x22,0x0f);	                     // 开启CRC,奇偶校验校验 
	temp = Command_Send_B(6, buf_B, Authent1);
	if(temp == FALSE)
	{
		return FM1702_NOTAGERR;
	}

	temp = read_reg_B(0x0A);
	if((temp & 0x02) == 0x02) return FM1702_PARITYERR;
	if((temp & 0x04) == 0x04) return FM1702_FRAMINGERR;
	if((temp & 0x08) == 0x08) return FM1702_CRCERR;
	temp = Command_Send_B(0, buf_B, Authent2);	 // AUTHENT2 
	if(temp == FALSE)
	{
		return FM1702_NOTAGERR;
	}

	temp = read_reg_B(0x0A);
	if((temp & 0x02) == 0x02) return FM1702_PARITYERR;
	if((temp & 0x04) == 0x04) return FM1702_FRAMINGERR;
	if((temp & 0x08) == 0x08) return FM1702_CRCERR;
	temp1 = read_reg_B(0x09);
	temp1 = temp1 & 0x08;	                     // Crypto1on=1验证通过 
	if(temp1 == 0x08)
	{
		return FM1702_OK;
	}
	return FM1702_AUTHERR;
}

/****************************************************************/
/*名称: MIF_Read                                                */
/*功能: 该函数实现读MIFARE卡块的数值                            */
/*输入: buff: 缓冲区首地址                                      */
/* Block_Adr: 块地址                                            */
/*输出: FM1702_NOTAGERR: 无卡                                   */
/* FM1702_PARITYERR: 奇偶校验错                                 */
/* FM1702_CRCERR: CRC校验错                                     */
/* FM1702_BYTECOUNTERR: 接收字节错误                            */
/* FM1702_OK: 应答正确                                          */
/****************************************************************/
uchar MIF_READ(uchar *buff, uchar Block_Adr)
{
	uchar	temp;

	write_reg_B(0x23,0x63);
	write_reg_B(0x12,0x3f);
	write_reg_B(0x13,0x3f);
	write_reg_B(0x22,0x0f);

	buff[0] = RF_CMD_READ;
	buff[1] = Block_Adr;
	temp = Command_Send_B(2, buff, Transceive);
	if(temp == 0)
	{
		return FM1702_NOTAGERR;
	}

	temp = read_reg_B(0x0A);
	if((temp & 0x02) == 0x02) return FM1702_PARITYERR;
	if((temp & 0x04) == 0x04) return FM1702_FRAMINGERR;
	if((temp & 0x08) == 0x08) return FM1702_CRCERR;
	temp = read_reg_B(0x04);
	if(temp == 0x10)	                      // 8K卡读数据长度为16
	{
		Read_FIFO_B(buff);
		PR("1st_RFID_MIF_READ OK!\r\n");
		return FM1702_OK;
	}
	else if(temp == 0x04)	                  // Token卡读数据长度为16
	{
		Read_FIFO_B(buff);
		return FM1702_OK;
	}
	else
	{
		return FM1702_BYTECOUNTERR;
	}
}

/****************************************************************/
/*名称: MIF_Write                                               */
/*功能: 该函数实现写MIFARE卡块的数值                            */
/*输入: buff: 缓冲区首地址                                      */
/* Block_Adr: 块地址                                            */
/*输出: FM1702_NOTAGERR: 无卡                                   */
/* FM1702_BYTECOUNTERR: 接收字节错误                            */
/* FM1702_NOTAUTHERR: 未经权威认证                              */
/* FM1702_EMPTY: 数据溢出错误                                   */
/* FM1702_CRCERR: CRC校验错                                     */
/* FM1702_PARITYERR: 奇偶校验错                                 */
/* FM1702_WRITEERR: 写卡块数据出错                              */
/* FM1702_OK: 应答正确                                          */
/****************************************************************/
uchar MIF_Write(uchar *buff, uchar Block_Adr)
{
	uchar	temp;
	uchar	F_buff[2];

	write_reg_B(0x23,0x63);
	write_reg_B(0x12,0x3f);
//	F_buff = temp1 + 0x10;
	write_reg_B(0x22,0x07);
	F_buff[0] = RF_CMD_WRITE;
	F_buff[1] = Block_Adr;
	temp = Command_Send_B(2, F_buff, Transceive);
	if(temp == FALSE)
	{
		return(FM1702_NOTAGERR);
	}

	temp = read_reg_B(0x04);
	if(temp == 0)
	{
		return(FM1702_BYTECOUNTERR);
	}

	Read_FIFO_B(F_buff);
	temp = *F_buff;
	switch(temp)
	{
		case 0x00:	return(FM1702_NOTAUTHERR);	     // 暂时屏蔽掉写错误
		case 0x04:	return(FM1702_EMPTY);
		case 0x0a:	break;
		case 0x01:	return(FM1702_CRCERR);
		case 0x05:	return(FM1702_PARITYERR);
		default:	return(FM1702_WRITEERR);
	}

	temp = Command_Send_B(16, buff, Transceive);
	if(temp == TRUE)
	{
		return(FM1702_OK);
	}
	else
	{
		temp = read_reg_B(0x0A);
		if((temp & 0x02) == 0x02)
			return(FM1702_PARITYERR);
		else if((temp & 0x04) == 0x04)
			return(FM1702_FRAMINGERR);
		else if((temp & 0x08) == 0x08)
			return(FM1702_CRCERR);
		else
			return(FM1702_WRITEERR);
	}
}

/****************************************************************/
/*名称: ReadID_FM1702                                            */
/*功能: 该函数实现对FM1702 ID读取  */
/*输入: N/A                                                      */
/*输出: TRUE 读取成功并打印                                      */
/*      FALSE读取失败返回FALSE                                   */
/*id1: 0x3088fe0303000001fef53a4395575e1b                        */
/*id2: 0x3088fe0303000001e8e63a4395575e82                        */
/*id3: 0x3088fe0303000001a4da3a4395575e78                        */
/*id4: 0x3088fe03030000014dd43a4395575e27                        */
/*id5: 0x3088fe030300000193e63a4395575e61                        */
/*id6: 0x3088fe0303000001baf93a4395575e8c                        */
/*****************************************************************/
uchar ReadID_1st_FM1702(void)
{
	uchar	temp;
	buf_B[0] = 0x00;
	buf_B[1] = 0x00;
	buf_B[2] = 0x10;
	temp = Command_Send_B(3, buf_B, ReadE2);
	delay_us(100);
	if(temp==TRUE)
	{
		Read_FIFO_B(buf_B);
		delay_us(100);		// 从FIFO中读取应答信息 
		return TRUE;	
	}
  return FALSE;

}
/****************************************************************/
/*名称: Standby_1st                                            */
/*功能: 该函数实现对FM1702 standby                               */
/*输入: N/A                                                      */
/*输出: N/A                                                      */
/*****************************************************************/
void RFID_Powerdown_B(void)
{
//	/*关电源*/
//	uchar	temp;
//	temp = read_reg_B(Control);
//	temp = temp | 0x10;
//	write_reg_B(Control,temp);
//	delay_us(100);
	/*关天线*/
	uchar	temp;
	temp = read_reg_B(TxControl);
	temp = temp & 0xFC;
	write_reg_B(TxControl,temp);
	delay_us(100);				
}
void RFID_Off_B(void)
{
	/*关电源*/
	uchar	temp;
	temp = read_reg_B(Control);
	temp = temp | 0x10;
	write_reg_B(Control,temp);
	delay_us(100);
//	/*关天线*/
//	uchar	temp;
//	temp = read_reg_B(TxControl);
//	temp = temp & 0xFC;
//	write_reg_B(TxControl,temp);
//	delay_us(100);				
}
/****************************************************************/
/*名称: Wakeup_1st                                            */
/*功能: 该函数实现对FM1702 Wakeup                               */
/*输入: N/A                                                      */
/*输出: N/A                                                      */
/*****************************************************************/
void RFID_Wakeup_B(void)
{
	/*开电源*/
//	uchar	temp;
//	temp = read_reg_B(Control);
//	temp = temp & 0xef;
//	write_reg_B(Control,temp);
//	delay_us(100);
	/*开天线*/
		uchar	temp;
	temp = read_reg_B(TxControl);
	temp = temp | 0x03;
	write_reg_B(TxControl,temp);
	delay_us(100);	
}
void RFID_On_B(void)
{
	/*开电源*/
	uchar	temp;
	temp = read_reg_B(Control);
	temp = temp & 0xef;
	write_reg_B(Control,temp);
	delay_us(100);
//	/*开天线*/
//		uchar	temp;
//	temp = read_reg_B(TxControl);
//	temp = temp | 0x03;
//	write_reg_B(TxControl,temp);
//	delay_us(100);	
}
/****************************************************************/
/*名称: Read_tag_1st                                            */
/*功能: 该函数实现完整的读卡操作                                */ 
/*												       			                          */
/*输入:                                                         */
/*      N/A                                                     */
/*                                                              */
/*输出: TRUE or FALSE                                           */
/****************************************************************/
uchar Read_tag_B(void)
{
	unsigned char status ,loop = 0, s=0;
  for(loop=0;loop<10;++loop)
	{	
		status = Request(RF_CMD_REQUEST_ALL);		    //寻卡
		if(status == FM1702_OK)   
		{
			status = AntiColl();                     //冲突检测
			if(status == FM1702_OK)
			{
				status=Select_Card();                  //选卡
				if(status == FM1702_OK)
				{
					status = Load_keyE2_CPY(DefaultKey);          //加载密码
					if(status == TRUE)
					{
						status = Authentication(UID, 1, RF_CMD_AUTH_LA);	  //验证1扇区keyA
						if(status == FM1702_OK)
						{
							status=MIF_READ(buffer_temp_1st,4);							       //读卡，读取1扇区0块数据到buffer_temp_2nd[0]-buffer_temp_2nd[15]			
							if (status == FM1702_OK)
							{
								if ((buffer_temp_1st[0]==0xaa) && (buffer_temp_1st[1]==0xaa)) //如果标识正确0xaaaa，则复制到buffer，返回TRUE
								{
									memcpy(buffer_B,buffer_temp_1st,16);
									s++;
									return TRUE;
								}
							} 
						}
					}						
				}
			}
		}
	}
	if (s>0)
	{
		return TRUE; 	
	}

	 return FALSE;		
}
/****************************************************************/
/*名称: Read_check_B                                            */
/*功能: 该函数实现完整的读卡操作                                */ 
/*												       			                          */
/*输入:                                                         */
/*      N/A                                                     */
/*                                                              */
/*输出: TRUE or FALSE                                           */
/****************************************************************/
uchar Read_check_B(void)
{
	unsigned char status ;
	status = Request(RF_CMD_REQUEST_ALL);		    //寻卡
	if(status == FM1702_OK)   
	{
		status = AntiColl();                     //冲突检测
		if(status == FM1702_OK)
		{
			status=Select_Card();                  //选卡
			if(status == FM1702_OK)
			{
				status = Load_keyE2_CPY(DefaultKey);          //加载密码
				if(status == TRUE)
				{
					status = Authentication(UID, 1, RF_CMD_AUTH_LA);	  //验证1扇区keyA
					if(status == FM1702_OK)
					{
						status=MIF_READ(buffer_temp_1st,4);							       //读卡，读取1扇区0块数据到buffer_temp_2nd[0]-buffer_temp_2nd[15]			
						if (status == FM1702_OK)
						{
							if ((buffer_temp_1st[0]==0xaa) && (buffer_temp_1st[1]==0xaa)) //如果标识正确0xaaaa，则复制到buffer，返回TRUE
							{
								return TRUE; 	
							}
						}
					}
				}						
			}
		}
	}

	return FALSE;		
}
/****************************************************************/
/*名称: STM32_FM1702_GPIO_Init                                  */
/*功能: 该函数实现对STM32 GPIO-》SPI初始化                      */ 
/*												       			                          */
/*输入:                                                         */
/*      N/A                                                     */
/*                                                              */
/*输出: N/A                                                     */
/****************************************************************/
void STM32_FM1702_GPIO_Init()
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = MF522_RST_PIN | MF522_RST_PIN_2nd | MF522_MOSI_PIN | MF522_SCK_PIN | MF522_NSS_PIN | MF522_NSS_PIN_2nd;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = MF522_MISO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(MF522_MISO_PORT, &GPIO_InitStructure);
}


/****************************************************************/
/*名称: Init_FM1702                                             */
/*功能: 该函数实现对2个FM1702初始化                             */ 
/*												       			                          */
/*输入:                                                         */
/*      N/A                                                     */
/*                                                              */
/*输出: N/A                                                     */
/****************************************************************/
uchar Init_FM1702()
{
	uchar	temp1,temp2;
	temp1=Init_FM1702_1st();
	temp2=Init_FM1702_2nd();
	if(temp1==TRUE && temp2==TRUE)
	{
		return TRUE;
	}
	return FALSE;
}

/****************************************************************/
/*名称: Read_status_1st                                         */
/*功能: 该函数实现两次读卡状态变化                              */ 
/*												       			                          */
/*输入:                                                         */
/*      N/A                                                     */
/*                                                              */
/*输出: TRUE or FALSE                                           */
/*      TRUE 有变化，FALSE 无变化                               */    
/****************************************************************/
uchar Read_status_B(void)
{
	unsigned char status;
  status =  Read_tag_B();
	if (status == TRUE && RFID_status_1st==1)  //有钥匙且无变化
	{
		return FALSE;
	}
	else if(status == FALSE && RFID_status_1st==1) //无钥匙且有变化
	{
		memset(buffer_B,0,16*sizeof(char));          //清buffer 

		RFID_status_1st=0;
		return TRUE;
	}
	else if(status == TRUE && RFID_status_1st==0) //有钥匙且有变化
	{
		RFID_status_1st=1;
		return TRUE;
	}
	else if(status == FALSE && RFID_status_1st==0) //无钥匙且无变化
	{
		return FALSE;
	}
	else
	{
		return FALSE;
	}
}
uchar calibration_B(void)
{

	unsigned char calibration_array_1[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};	
	unsigned char RxThreshold_value=0;
	int i,j,max=0,maxb=0,xxx,count;
	max = calibration_array_1[1];
	FLASH_ReadByte(ADDR_Check_B,calibration_array_1,16);
	delay_ms(1000);
	RxThreshold_value =calibration_array_1[0];
	delay_us(100);
	count = (RxThreshold_value>>4);
	if(RxThreshold_value==0xBF)
	{
		calibration_array_1[count]=0;
		calibration_array_1[2] = 0x11;
		FLASH_Write_Integrate(ADDR_Check_B,calibration_array_1);					
		LED1_G_OFF;LED1_R_ON;LED1_B_OFF;
		delay_ms(1000);
		relayBuf[0]=calibration_array_1[15];
		SendPacket(chipID_B,CALIB_ACK);	
		delay_ms(1000);
		__set_FAULTMASK(1);
		NVIC_SystemReset();				
		return TRUE;
	}
	else if(RxThreshold_value==0xAF)
	{
		calibration_array_1[count]=0;
		for(i=5;i<11;i++)
		{
			if(calibration_array_1[i]>max)
			{
				max=calibration_array_1[i];
				maxb=i;
			}
		}
		calibration_array_1[0] = RxThreshold_value+0x10;
		calibration_array_1[15] = maxb*16+0x0f;
		FLASH_Write_Integrate(ADDR_Check_B,calibration_array_1);					
		LED1_G_ON;LED1_R_OFF;LED1_B_OFF;
		delay_ms(1000);		
		if(max>200)
		{
			__set_FAULTMASK(1);
			NVIC_SystemReset();			
		}
		if(max<=200)
		{
			calibration_array_1[0] = 0x77;
			calibration_array_1[count]=0;
			calibration_array_1[2] = 0x22;
			FLASH_Write_Integrate(ADDR_Check_B,calibration_array_1);					
			delay_ms(1000);
			LED1_G_ON;LED1_R_ON;LED1_B_OFF;
			relayBuf[0]=0x77;
		  SendPacket(chipID_B,CALIB_ACK);
			delay_ms(1000);
			__set_FAULTMASK(1);
			NVIC_SystemReset();			
		}	
			
	}
	else
	{
		write_reg_B(RxThreshold,RxThreshold_value);// RxThreshold_value);
		calibration_array_1[count]=0;
		for(j=0;j<255;j++)
		{
			RFID_Wakeup_B();
			delay_ms(1);
			xxx =  Read_check_B();
			delay_ms(1);
			RFID_Powerdown_B();					
			if(xxx == TRUE) 
			{
				calibration_array_1[count]++;
			}
		}
		calibration_array_1[0] = RxThreshold_value+0x10;
		FLASH_Write_Integrate(ADDR_Check_B,calibration_array_1);			
		__set_FAULTMASK(1);
		NVIC_SystemReset();
		}
	return TRUE;
}

