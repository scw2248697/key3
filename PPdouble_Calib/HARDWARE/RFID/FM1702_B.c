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
uchar     	tagtype[2];	        /* ��Ƭ��ʶ�ַ� */
/* FM1702�������� */
volatile unsigned char     	buf_B[10];            /* FM1702����ͽ��ջ����� */
unsigned char buffer_B[16];
unsigned char buffer_temp_1st[16];
uchar     	UID[5];             /* ���к� */
unsigned char DefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
//uchar       Secnr;			        /* ������ */
uchar RFID_status_1st;  /*����״̬��ʶ, 1--��Կ�� 0--��Կ��*/


//SPI����
 unsigned char rev_B(uchar tem)
{
  return SPI1_ReadWriteByte(tem);
} 
//SPI����
void Send_B(unsigned char var) 

{ 
	SPI1_ReadWriteByte(var);
}                  

//���Ĵ���
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

//д�Ĵ���
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
/*����: Clear_FIFO_B                                              */
/*����: �ú���ʵ����FFIFO������                                */
/*												       			                          */
/*����:                                                         */
/*      N/A                                                     */
/*                                                              */
/*���:                                                         */
/*	    TRUE, FIFO�����                                        */
/*	    FALSE, FIFOδ�����  	                                  */
/****************************************************************/
 uchar Clear_FIFO_B(void)
{
	uchar temp;
	uint  i;
	
	temp =read_reg_B(Control);						//���FIFO
	temp = (temp | 0x01);
	write_reg_B(Control, temp);
	for(i = 0; i < RF_TimeOut; i++)			//���FIFO�Ƿ����
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
/*����: Write_FIFO_B                                              */
/*����: �ú���ʵ����RC531��FIFO��д��x bytes����                */
/*												       			                          */
/*����:                                                         */
/*      count, ��д���ֽڵĳ���                                 */
/*	    buff, ָ���д�����ݵ�ָ��                              */
/*                                                              */
/*���:                                                         */
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
/*����: Read_FIFO_B                                               */
/*����: �ú���ʵ�ִ�RC531��FIFO�ж���x bytes����                */
/*												        		                          */
/*����:                                                         */
/*       buff, ָ��������ݵ�ָ��                               */
/*                                                              */
/*���:                                                         */
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
	if (temp >= 24)						//temp=255ʱ,�������ѭ��
	{									        //�������FIFO_LengthԽ���ж�
		temp = 24;						 
	}
	for(i = 0;i < temp; i++)
	{
 		*(buff + i) =read_reg_B(FIFO);
	}
	return temp;
 }

/****************************************************************/
/*����: Judge_Req_B                                               */
/*����: �ú���ʵ�ֶԿ�Ƭ��λӦ���źŵ��ж�                      */
/*												       			                          */
/*����:                                                         */
/*       *buff, ָ��Ӧ�����ݵ�ָ��                              */
/*                                                              */
/*���:                                                         */
/*	     TRUE, ��ƬӦ���ź���ȷ                                 */
/*       FALSE, ��ƬӦ���źŴ���                                */
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
/*����: Check_UID_B                                               */
/*����: �ú���ʵ�ֶ��յ��Ŀ�Ƭ�����кŵ��ж�                    */
/*����: N/A                                                     */
/*���: TRUE: ���к���ȷ                                        */
/* FALSE: ���кŴ���                                            */
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
/*����: Save_UID_B                                                */
/*����: �ú���ʵ�ֱ��濨Ƭ�յ������к�                          */
/*����: row: ������ͻ����                                       */
/* col: ������ͻ����                                            */
/* length: �Ӆ�����UID���ݳ���                                  */
/*���: N/A                                                     */
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
/*����: Set_BitFraming_B                                          */
/*����: �ú������ô��������ݵ��ֽ���                            */
/*����: row: ������ͻ����                                       */
/*      col: ������ͻ����                                       */
/*���: N/A                                                     */
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
/*����: FM1702_Bus_Sel_B                                          */
/*����: �ú���ʵ�ֶ�FM1702���������߷�ʽ(��������,SPI)ѡ��      */ 
/*												       			                          */
/*����:                                                         */
/*      N/A                                                     */
/*                                                              */
/*���:                                                         */
/*	    TRUE,  ����ѡ��ɹ�                                     */
/*	    FALSE, ����ѡ��ʧ��  	                                  */
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
/*����: Init_FM1702_1st                                          */
/*����: �ú���ʵ�ֶ�FM1702��ʼ��                                */ 
/*												       			                          */
/*����:                                                         */
/*      N/A                                                     */
/*                                                              */
/*���:                                                         */
/*	    TRUE,  ����ѡ��ɹ�                                     */
/*	    FALSE, ����ѡ��ʧ��  	                                  */
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
	RST_H;				        //��λ�
	delay_ms(50);
	RST_L;
	delay_ms(50);
/*�µĳ�ʼ��*/	
	temp = read_reg_B(0x05);
	delay_ms(50);
	temp = FM1702_Bus_Sel_B();		 	//����ѡ��
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
/*����: Command_Send_B                                            */
/*����: �ú���ʵ����RC531��������Ĺ���                       */
/*												        		                          */
/*����:                                                         */
/*       count, ����������ĳ���                              */
/*	     buff, ָ����������ݵ�ָ��                             */
/*       Comm_Set, ������                                       */
/*												       			                          */
/*���:                                                         */
/*	     TRUE, �����ȷִ��                                   */
/*	     FALSE, ����ִ�д���  	                                */
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
	write_reg_B(Command, Comm_Set);					//����ִ��
	
	for(j = 0; j< RF_TimeOut; j++)				//�������ִ�з�
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
/*����: MIF_Halt                                                */
/*����: �ú���ʵ����ͣMIFARE��                                  */
/*����: N/A                                                     */
/*���: FM1702_OK: Ӧ����ȷ                                     */
/* FM1702_PARITYERR: ��żУ���                                 */
/* FM1702_CRCERR: CRCУ���                                     */
/* FM1702_NOTAGERR: �޿�                                        */
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
// ת����Կ��ʽ
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
/*����: Load_keyE2                                              */
/*����: �ú���ʵ�ְ�E2���������FM1702��keybuf��             */
/*����: Secnr: EE��ʼ��ַ                                       */
/*���: True: ��Կװ�سɹ�                                      */
/* False: ��Կװ��ʧ��                                          */
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
/*����: Request                                                 */
/*����: �ú���ʵ�ֶԷ���RC531������Χ֮�ڵĿ�Ƭ��Request����    */ 
/*												       			                          */
/*����:                                                         */
/*      mode: ALL(�������RC531������Χ֮�ڵĿ�Ƭ)			   	    */
/*	    STD(�����RC531������Χ֮�ڴ���HALT״̬�Ŀ�Ƭ)          */
/*                                                              */
/*���:                                                         */
/*	    FM222_NOTAGERR: �޿�                                    */
/*      FM222_OK: Ӧ����ȷ                                      */
/*	    FM222_REQERR: Ӧ�����										              */
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
    buf_B[0] = mode;					             //Requestģʽѡ��
    write_reg_B(Bit_Frame,0x07);			       //����7bit
    write_reg_B(ChannelRedundancy,0x03);	   //�ر�CRC
    write_reg_B(TxControl,0x5b); 
    write_reg_B(Control,0x01);          		 //����CRYPTO1λ
    temp = Command_Send_B(1, buf_B, Transceive);
    if(temp == FALSE)
    {

	    return FM1702_NOTAGERR;
    }	
	
    Read_FIFO_B(buf_B);					           //��FIFO�ж�ȡӦ����Ϣ
    temp = Judge_Req_B(buf_B);			         //�ж�Ӧ���ź��Ƿ���ȷ
    if (temp == TRUE)
    {
      tagtype[0] = buf_B[0];
      tagtype[1] = buf_B[1];
      return FM1702_OK;
    }

		return FM1702_REQERR;
		
}

/****************************************************************/
/*����: AntiColl                                                */
/*����: �ú���ʵ�ֶԷ���FM1702������Χ֮�ڵĿ�Ƭ�ķ���ͻ���    */
/*����: N/A                                                     */
/*���: FM1702_NOTAGERR: �޿�                                   */
/* FM1702_BYTECOUNTERR: �����ֽڴ���                            */
/* FM1702_SERNRERR: ��Ƭ���к�Ӧ�����                          */
/* FM1702_OK: ��ƬӦ����ȷ                                      */
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
	write_reg_B(0x22,0x03);	                   // �ر�CRC,����żУ��
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
		Save_UID_B(row, col, temp);			        // ���յ���UID����UID������
	
		temp = read_reg_B(0x0A);				        // �жϽӅ������Ƿ����
		temp = temp & 0x01;
		if(temp == 0x00)
		{
			temp = Check_UID_B();			            // У���յ���UID
			if(temp == FALSE)
			{
				return(FM1702_SERNRERR);
			}
			return(FM1702_OK);
		}
		else
		{
			temp = read_reg_B(0x0B);             // ��ȡ��ͻ���Ĵ��� 
			row = temp / 8;
			col = temp % 8;
			buf_B[0] = RF_CMD_ANTICOL;
			Set_BitFraming_B(row + pre_row, col);	// ���ô��������ݵ��ֽ��� 
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
/*����: Select_Card                                             */
/*����: �ú���ʵ�ֶԷ���FM1702������Χ֮�ڵ�ĳ�ſ�Ƭ����ѡ��    */
/*����: N/A                                                     */
/*���: FM1702_NOTAGERR: �޿�                                   */
/* FM1702_PARITYERR: ��żУ���                                 */
/* FM1702_CRCERR: CRCУ���                                     */
/* FM1702_BYTECOUNTERR: �����ֽڴ���                            */
/* FM1702_OK: Ӧ����ȷ                                          */
/* FM1702_SELERR: ѡ������                                      */
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

	write_reg_B(0x22,0x0f);	                       // ����CRC,��żУ��У�� 
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
		Read_FIFO_B(buf_B);	                      // ��FIFO�ж�ȡӦ����Ϣ 
		temp = *buf_B;
		//�ж�Ӧ���ź��Ƿ���ȷ 
		if((temp == 0x08) || (temp == 0x88) || (temp == 0x53) ||(temp == 0x18)) //S70 temp = 0x18	
		{
			return(FM1702_OK);
		}
		else
			return(FM1702_SELERR);
	}
}

/****************************************************************/
/*����: Authentication                                          */
/*����: �ú���ʵ��������֤�Ĺ���                                */
/*����: UID: ��Ƭ���кŵ�ַ                                     */
/* SecNR: ������                                                */
/* mode: ģʽ                                                   */
/*���: FM1702_NOTAGERR: �޿�                                   */
/* FM1702_PARITYERR: ��żУ���                                 */
/* FM1702_CRCERR: CRCУ���                                     */
/* FM1702_OK: Ӧ����ȷ                                          */
/* FM1702_AUTHERR: Ȩ����֤�д�                                 */
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

	write_reg_B(0x22,0x0f);	                     // ����CRC,��żУ��У�� 
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
	temp1 = temp1 & 0x08;	                     // Crypto1on=1��֤ͨ�� 
	if(temp1 == 0x08)
	{
		return FM1702_OK;
	}
	return FM1702_AUTHERR;
}

/****************************************************************/
/*����: MIF_Read                                                */
/*����: �ú���ʵ�ֶ�MIFARE�������ֵ                            */
/*����: buff: �������׵�ַ                                      */
/* Block_Adr: ���ַ                                            */
/*���: FM1702_NOTAGERR: �޿�                                   */
/* FM1702_PARITYERR: ��żУ���                                 */
/* FM1702_CRCERR: CRCУ���                                     */
/* FM1702_BYTECOUNTERR: �����ֽڴ���                            */
/* FM1702_OK: Ӧ����ȷ                                          */
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
	if(temp == 0x10)	                      // 8K�������ݳ���Ϊ16
	{
		Read_FIFO_B(buff);
		PR("1st_RFID_MIF_READ OK!\r\n");
		return FM1702_OK;
	}
	else if(temp == 0x04)	                  // Token�������ݳ���Ϊ16
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
/*����: MIF_Write                                               */
/*����: �ú���ʵ��дMIFARE�������ֵ                            */
/*����: buff: �������׵�ַ                                      */
/* Block_Adr: ���ַ                                            */
/*���: FM1702_NOTAGERR: �޿�                                   */
/* FM1702_BYTECOUNTERR: �����ֽڴ���                            */
/* FM1702_NOTAUTHERR: δ��Ȩ����֤                              */
/* FM1702_EMPTY: �����������                                   */
/* FM1702_CRCERR: CRCУ���                                     */
/* FM1702_PARITYERR: ��żУ���                                 */
/* FM1702_WRITEERR: д�������ݳ���                              */
/* FM1702_OK: Ӧ����ȷ                                          */
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
		case 0x00:	return(FM1702_NOTAUTHERR);	     // ��ʱ���ε�д����
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
/*����: ReadID_FM1702                                            */
/*����: �ú���ʵ�ֶ�FM1702 ID��ȡ  */
/*����: N/A                                                      */
/*���: TRUE ��ȡ�ɹ�����ӡ                                      */
/*      FALSE��ȡʧ�ܷ���FALSE                                   */
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
		delay_us(100);		// ��FIFO�ж�ȡӦ����Ϣ 
		return TRUE;	
	}
  return FALSE;

}
/****************************************************************/
/*����: Standby_1st                                            */
/*����: �ú���ʵ�ֶ�FM1702 standby                               */
/*����: N/A                                                      */
/*���: N/A                                                      */
/*****************************************************************/
void RFID_Powerdown_B(void)
{
//	/*�ص�Դ*/
//	uchar	temp;
//	temp = read_reg_B(Control);
//	temp = temp | 0x10;
//	write_reg_B(Control,temp);
//	delay_us(100);
	/*������*/
	uchar	temp;
	temp = read_reg_B(TxControl);
	temp = temp & 0xFC;
	write_reg_B(TxControl,temp);
	delay_us(100);				
}
void RFID_Off_B(void)
{
	/*�ص�Դ*/
	uchar	temp;
	temp = read_reg_B(Control);
	temp = temp | 0x10;
	write_reg_B(Control,temp);
	delay_us(100);
//	/*������*/
//	uchar	temp;
//	temp = read_reg_B(TxControl);
//	temp = temp & 0xFC;
//	write_reg_B(TxControl,temp);
//	delay_us(100);				
}
/****************************************************************/
/*����: Wakeup_1st                                            */
/*����: �ú���ʵ�ֶ�FM1702 Wakeup                               */
/*����: N/A                                                      */
/*���: N/A                                                      */
/*****************************************************************/
void RFID_Wakeup_B(void)
{
	/*����Դ*/
//	uchar	temp;
//	temp = read_reg_B(Control);
//	temp = temp & 0xef;
//	write_reg_B(Control,temp);
//	delay_us(100);
	/*������*/
		uchar	temp;
	temp = read_reg_B(TxControl);
	temp = temp | 0x03;
	write_reg_B(TxControl,temp);
	delay_us(100);	
}
void RFID_On_B(void)
{
	/*����Դ*/
	uchar	temp;
	temp = read_reg_B(Control);
	temp = temp & 0xef;
	write_reg_B(Control,temp);
	delay_us(100);
//	/*������*/
//		uchar	temp;
//	temp = read_reg_B(TxControl);
//	temp = temp | 0x03;
//	write_reg_B(TxControl,temp);
//	delay_us(100);	
}
/****************************************************************/
/*����: Read_tag_1st                                            */
/*����: �ú���ʵ�������Ķ�������                                */ 
/*												       			                          */
/*����:                                                         */
/*      N/A                                                     */
/*                                                              */
/*���: TRUE or FALSE                                           */
/****************************************************************/
uchar Read_tag_B(void)
{
	unsigned char status ,loop = 0, s=0;
  for(loop=0;loop<10;++loop)
	{	
		status = Request(RF_CMD_REQUEST_ALL);		    //Ѱ��
		if(status == FM1702_OK)   
		{
			status = AntiColl();                     //��ͻ���
			if(status == FM1702_OK)
			{
				status=Select_Card();                  //ѡ��
				if(status == FM1702_OK)
				{
					status = Load_keyE2_CPY(DefaultKey);          //��������
					if(status == TRUE)
					{
						status = Authentication(UID, 1, RF_CMD_AUTH_LA);	  //��֤1����keyA
						if(status == FM1702_OK)
						{
							status=MIF_READ(buffer_temp_1st,4);							       //��������ȡ1����0�����ݵ�buffer_temp_2nd[0]-buffer_temp_2nd[15]			
							if (status == FM1702_OK)
							{
								if ((buffer_temp_1st[0]==0xaa) && (buffer_temp_1st[1]==0xaa)) //�����ʶ��ȷ0xaaaa�����Ƶ�buffer������TRUE
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
/*����: Read_check_B                                            */
/*����: �ú���ʵ�������Ķ�������                                */ 
/*												       			                          */
/*����:                                                         */
/*      N/A                                                     */
/*                                                              */
/*���: TRUE or FALSE                                           */
/****************************************************************/
uchar Read_check_B(void)
{
	unsigned char status ;
	status = Request(RF_CMD_REQUEST_ALL);		    //Ѱ��
	if(status == FM1702_OK)   
	{
		status = AntiColl();                     //��ͻ���
		if(status == FM1702_OK)
		{
			status=Select_Card();                  //ѡ��
			if(status == FM1702_OK)
			{
				status = Load_keyE2_CPY(DefaultKey);          //��������
				if(status == TRUE)
				{
					status = Authentication(UID, 1, RF_CMD_AUTH_LA);	  //��֤1����keyA
					if(status == FM1702_OK)
					{
						status=MIF_READ(buffer_temp_1st,4);							       //��������ȡ1����0�����ݵ�buffer_temp_2nd[0]-buffer_temp_2nd[15]			
						if (status == FM1702_OK)
						{
							if ((buffer_temp_1st[0]==0xaa) && (buffer_temp_1st[1]==0xaa)) //�����ʶ��ȷ0xaaaa�����Ƶ�buffer������TRUE
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
/*����: STM32_FM1702_GPIO_Init                                  */
/*����: �ú���ʵ�ֶ�STM32 GPIO-��SPI��ʼ��                      */ 
/*												       			                          */
/*����:                                                         */
/*      N/A                                                     */
/*                                                              */
/*���: N/A                                                     */
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
/*����: Init_FM1702                                             */
/*����: �ú���ʵ�ֶ�2��FM1702��ʼ��                             */ 
/*												       			                          */
/*����:                                                         */
/*      N/A                                                     */
/*                                                              */
/*���: N/A                                                     */
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
/*����: Read_status_1st                                         */
/*����: �ú���ʵ�����ζ���״̬�仯                              */ 
/*												       			                          */
/*����:                                                         */
/*      N/A                                                     */
/*                                                              */
/*���: TRUE or FALSE                                           */
/*      TRUE �б仯��FALSE �ޱ仯                               */    
/****************************************************************/
uchar Read_status_B(void)
{
	unsigned char status;
  status =  Read_tag_B();
	if (status == TRUE && RFID_status_1st==1)  //��Կ�����ޱ仯
	{
		return FALSE;
	}
	else if(status == FALSE && RFID_status_1st==1) //��Կ�����б仯
	{
		memset(buffer_B,0,16*sizeof(char));          //��buffer 

		RFID_status_1st=0;
		return TRUE;
	}
	else if(status == TRUE && RFID_status_1st==0) //��Կ�����б仯
	{
		RFID_status_1st=1;
		return TRUE;
	}
	else if(status == FALSE && RFID_status_1st==0) //��Կ�����ޱ仯
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

