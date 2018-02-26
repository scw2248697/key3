/*
 * --------------------
 * Company							: Tianjin techvan
 * --------------------
 * Project Name					: PICA
 * Description					: C file
 * --------------------
 * Tool Versions				: uVision V5.17.0.0
 * Target Device				: STM32F103C8
 * --------------------
 * Engineer							: �����
 * Revision							: 3.0
 * Created Date					: 2018.01.05
 * --------------------
 * Engineer							:
 * Revision							:
 * Modified Date				:
 * --------------------
 * Additional Comments	:
 *
 * --------------------
 */
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "FM1702.h"
#include "delay.h"
#include "usart.h"
#include "string.h"
#include "spi.h"
#include "led.h"
#include "DataFlash.h"
#include "can.h"
#include "commhead.h"
#define uchar unsigned char

//uchar       cardtype;
uchar     	tagtype[2];	        /* ��Ƭ��ʶ�ַ� */
unsigned char buffer_rfid[16]; 
unsigned char buffer_temp[16];
/* FM1702�������� */
volatile unsigned char     	buf[10];            /* FM1702����ͽ��ջ����� */
 uchar     	UID[5];             /* ���к� */
unsigned char DefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
//uchar       Secnr;			        /* ������ */
uchar RFID_status_2nd ;  /*����״̬��ʶ, 1--��Կ�� 0--��Կ��*/
volatile unsigned char Hole_Choose=0;

unsigned char Return_Fail=0;
unsigned char Return_Suc=1;
unsigned char Borrow_Fail=0;
unsigned char Borrow_Suc=1;
//Ƭѡ����
void Chip_select(unsigned char hole)
{
  if(hole==HOLE_A)
	{
		Hole_Choose=HOLE_A;
	}
  else if(hole==HOLE_B)
	{
		Hole_Choose=HOLE_B;
	}
	else
	{
	}
	
}
//SPI����
 unsigned char rev(uchar tem)
{
  return SPI1_ReadWriteByte(tem);
} 

//SPI����
 void Send(unsigned char var) 

{ 	
	SPI1_ReadWriteByte(var);
}                  

//���Ĵ���
 uchar read_reg(uchar SpiAddress)
{
  if(Hole_Choose == HOLE_A)
	{
		NSS_H_B;
		delay_us(30);	
		NSS_L_A;
		delay_us(30);	
	}
  else if(Hole_Choose == HOLE_B)
	{
		NSS_H_A;
		delay_us(30);	
		NSS_L_B;
		delay_us(30);		
	}
	else
	{
	}	
	uchar rdata;
	delay_us(1);	
	SpiAddress=SpiAddress<<1;
	SpiAddress=SpiAddress | 0x80;
	delay_us(30);
	Send(SpiAddress);
	rdata=rev(0);
	delay_us(1);	
	NSS_H_B;
	NSS_H_A;	
	return(rdata);

}

//д�Ĵ���
 void write_reg(uchar SpiAddress,uchar dat)
{
  if(Hole_Choose == HOLE_A)
	{
		NSS_H_B;
		delay_us(30);	
		NSS_L_A;
		delay_us(30);	
	}
  else if(Hole_Choose == HOLE_B)
	{
		NSS_H_A;
		delay_us(30);	
		NSS_L_B;
		delay_us(30);		
	}
	else
	{
	}	
	
	SpiAddress = SpiAddress << 1;
	SpiAddress = SpiAddress & 0x7f;
	delay_us(30);
	Send(SpiAddress);		
	Send(dat);
	delay_us(1);		
	NSS_H_B;
	NSS_H_A;	
}

/****************************************************************/
/*����: Clear_FIFO                                              */
/*����: �ú���ʵ����FFIFO������                                */
/*												       			                          */
/*����:                                                         */
/*      N/A                                                     */
/*                                                              */
/*���:                                                         */
/*	    TRUE, FIFO�����                                        */
/*	    FALSE, FIFOδ�����  	                                  */
/****************************************************************/
 uchar Clear_FIFO(void)
{
	uchar temp;
	uint  i;
	
	temp =read_reg(Control);						//���FIFO
	temp = (temp | 0x01);
	write_reg(Control, temp);
	for(i = 0; i < RF_TimeOut; i++)			//���FIFO�Ƿ����
	{
		temp = read_reg(FIFO_Length);
		if(temp == 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}

/****************************************************************/
/*����: Write_FIFO                                              */
/*����: �ú���ʵ����RC531��FIFO��д��x bytes����                */
/*												       			                          */
/*����:                                                         */
/*      count, ��д���ֽڵĳ���                                 */
/*	    buff, ָ���д�����ݵ�ָ��                              */
/*                                                              */
/*���:                                                         */
/*	    N/A                                                 		*/
/****************************************************************/
 void Write_FIFO(uchar count,volatile unsigned char *buff)
{
	uchar i;
	
	for(i = 0; i < count; i++)
	{
		write_reg(FIFO,*(buff + i));
	}
}

/****************************************************************/
/*����: Read_FIFO                                               */
/*����: �ú���ʵ�ִ�RC531��FIFO�ж���x bytes����                */
/*												        		                          */
/*����:                                                         */
/*       buff, ָ��������ݵ�ָ��                               */
/*                                                              */
/*���:                                                         */
/*	     N/A                                                 		*/
/****************************************************************/
 uchar Read_FIFO(volatile unsigned char *buff)
{
	uchar temp;
	uchar i;
	
	temp =read_reg(FIFO_Length);
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
 		*(buff + i) =read_reg(FIFO);
		}
	return temp;
 }

/****************************************************************/
/*����: Judge_Req                                               */
/*����: �ú���ʵ�ֶԿ�Ƭ��λӦ���źŵ��ж�                      */
/*												       			                          */
/*����:                                                         */
/*       *buff, ָ��Ӧ�����ݵ�ָ��                              */
/*                                                              */
/*���:                                                         */
/*	     TRUE, ��ƬӦ���ź���ȷ                                 */
/*       FALSE, ��ƬӦ���źŴ���                                */
/****************************************************************/
 uchar Judge_Req(volatile unsigned char *buff)
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
/*����: Check_UID                                               */
/*����: �ú���ʵ�ֶ��յ��Ŀ�Ƭ�����кŵ��ж�                    */
/*����: N/A                                                     */
/*���: TRUE: ���к���ȷ                                        */
/* FALSE: ���кŴ���                                            */
/****************************************************************/
 uchar Check_UID(void)
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

		return TRUE;
	}

	return FALSE;
}

/****************************************************************/
/*����: Save_UID                                                */
/*����: �ú���ʵ�ֱ��濨Ƭ�յ������к�                          */
/*����: row: ������ͻ����                                       */
/* col: ������ͻ����                                            */
/* length: �Ӆ�����UID���ݳ���                                  */
/*���: N/A                                                     */
/****************************************************************/
 void Save_UID(uchar row, uchar col, uchar length)
{
	uchar	i;
	uchar	temp;
	uchar	temp1;

	if((row == 0x00) && (col == 0x00))
	{
		for(i = 0; i < length; i++)
		{
			UID[i] = buf[i];
		}
	}
	else
	{
		temp = buf[0];
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

		buf[0] = temp;
		UID[row - 1] = temp1 | temp;
		for(i = 1; i < length; i++)
		{
			UID[row - 1 + i] = buf[i];
		}
	}
}

/****************************************************************/
/*����: Set_BitFraming                                          */
/*����: �ú������ô��������ݵ��ֽ���                            */
/*����: row: ������ͻ����                                       */
/*      col: ������ͻ����                                       */
/*���: N/A                                                     */
/****************************************************************/
 void Set_BitFraming(uchar row, uchar col)
{
	switch(row)
	{
	case 0:		buf[1] = 0x20; break;
	case 1:		buf[1] = 0x30; break;
	case 2:		buf[1] = 0x40; break;
	case 3:		buf[1] = 0x50; break;
	case 4:		buf[1] = 0x60; break;
	default:	break;
	}

	switch(col)
	{
	case 0:		write_reg(0x0F,0x00);  break;
	case 1:		write_reg(0x0F,0x11); buf[1] = (buf[1] | 0x01); break;
	case 2:		write_reg(0x0F,0x22); buf[1] = (buf[1] | 0x02); break;
	case 3:		write_reg(0x0F,0x33); buf[1] = (buf[1] | 0x03); break;
	case 4:		write_reg(0x0F,0x44); buf[1] = (buf[1] | 0x04); break;
	case 5:		write_reg(0x0F,0x55); buf[1] = (buf[1] | 0x05); break;
	case 6:		write_reg(0x0F,0x66); buf[1] = (buf[1] | 0x06); break;
	case 7:		write_reg(0x0F,0x77); buf[1] = (buf[1] | 0x07); break;
	default:	break;
	}
}

/****************************************************************/
/*����: FM1702_Bus_Sel                                          */
/*����: �ú���ʵ�ֶ�FM1702���������߷�ʽ(��������,SPI)ѡ��      */ 
/*												       			                          */
/*����:                                                         */
/*      N/A                                                     */
/*                                                              */
/*���:                                                         */
/*	    TRUE,  ����ѡ��ɹ�                                     */
/*	    FALSE, ����ѡ��ʧ��  	                                  */
/****************************************************************/
 uchar FM1702_Bus_Sel(void)
{
	uchar i,temp;
	
	write_reg(Page_Sel,0x80);
	write_reg(0x08,0x80);
	write_reg(0x10,0x80);
	write_reg(0x18,0x80);
	write_reg(0x20,0x80);
	write_reg(0x28,0x80);
	write_reg(0x30,0x80);
	write_reg(0x38,0x80);
	for(i = 0; i < RF_TimeOut; i++)
	{
		temp=read_reg(Command);
		if(temp == 0x00)
		{
			write_reg(Page_Sel,0x00);
			write_reg(0x08,0x00);
			write_reg(0x10,0x00);
			write_reg(0x18,0x00);
			write_reg(0x20,0x00);
			write_reg(0x28,0x00);
			write_reg(0x30,0x00);
			write_reg(0x38,0x00);			
			return TRUE;
		}
	}
	return FALSE;
}

/****************************************************************/
/*����: Init_FM1702                                          */
/*����: �ú���ʵ�ֶ�FM1702��ʼ��                                */ 
/*												       			                          */
/*����:                                                         */
/*      N/A                                                     */
/*                                                              */
/*���:                                                         */
/*	    TRUE,  ����ѡ��ɹ�                                     */
/*	    FALSE, ����ѡ��ʧ��  	                                  */
/****************************************************************/
uchar Init_FM1702(unsigned char hole)
{	
  unsigned char temp;
	unsigned char out[16];
	//Chip_select(hole);
	if(hole == HOLE_A)
	{
		Chip_select(HOLE_A);
		FLASH_ReadByte(ADDR_Check_B,out,16);
		RST_H_A;				        //��λ
		delay_ms(50);
		RST_L_A;
		delay_ms(50);			
	}
	if(hole == HOLE_B)
	{
		Chip_select(HOLE_B);
		FLASH_ReadByte(ADDR_Check_A,out,16);
		RST_H_B;				        //��λ
		delay_ms(50);
		RST_L_B;
		delay_ms(50);					
	}

	if(out[15]==0xFF)
	{
	 out[15] = 0x8f;
	}
/*�µĳ�ʼ��*/	
	temp = read_reg(0x05);
	delay_ms(50);
	temp = FM1702_Bus_Sel();		 	//����ѡ��
  if (temp == TRUE)
	{
		write_reg(CWConductance, 0x3F);
		write_reg(RxControl1, 0x73);
		write_reg(Rxcontrol2, 0x01);
		write_reg(RxThreshold, out[15]);
		RFID_RFclose();
		return TRUE;
	}	
	RFID_RFclose();
	return FALSE;	

}

/****************************************************************/
/*����: Command_Send                                            */
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
 uchar Command_Send(uchar count,volatile unsigned char * buff,uchar Comm_Set)
{
	uint  j;
	uchar temp;
	
	write_reg(Command, 0x00);
	Clear_FIFO();
  if (count != 0)
  {
	  Write_FIFO(count, buff);
  }	 
	write_reg(Command, Comm_Set);					//����ִ��
	
	for(j = 0; j< RF_TimeOut; j++)				//�������ִ�з�
	{
		temp =read_reg(Command);
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

	write_reg(0x23,0x63);
	write_reg(0x12,0x3f);
	write_reg(0x22,0x07);
	*buf = RF_CMD_HALT;
	*(buf + 1) = 0x00;
	temp = Command_Send(2, buf, Transmit);
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
		temp = read_reg(0x0A);
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
 char M500HostCodeKey(unsigned char *uncoded, unsigned char *coded)   
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
    
  M500HostCodeKey(uncoded_keys, coded_keys);
	temp = Command_Send(12, coded_keys, LoadKey);
	temp = read_reg(0x0A) & 0x40; //check ErrorFlag bit6, KeyErr
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
		write_reg(Command, 0x00);
		Clear_FIFO();
    uchar  temp;
    write_reg(TxControl,0x58);
	  delay_us(10);
    write_reg(TxControl,0x5b);		 
    write_reg(CRCPresetLSB,0x63);
    write_reg(CWConductance,0x3f);
    buf[0] = mode;					             //Requestģʽѡ��
    write_reg(Bit_Frame,0x07);			       //����7bit
    write_reg(ChannelRedundancy,0x03);	   //�ر�CRC
    write_reg(TxControl,0x5b); 
    write_reg(Control,0x01);          		 //����CRYPTO1λ
    temp = Command_Send(1, buf, Transceive);
    if(temp == FALSE)
    {
	    return FM1702_NOTAGERR;
    }	
	
    Read_FIFO(buf);					           //��FIFO�ж�ȡӦ����Ϣ
    temp = Judge_Req(buf);			         //�ж�Ӧ���ź��Ƿ���ȷ
    if (temp == TRUE)
    {
        tagtype[0] = buf[0];
        tagtype[1] = buf[1];
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
	write_reg(0x23,0x63);
	write_reg(0x12,0x3f);
	write_reg(0x13,0x3f);
	buf[0] = RF_CMD_ANTICOL;
	buf[1] = 0x20;
	write_reg(0x22,0x03);	                   // �ر�CRC,����żУ��
	temp = Command_Send(2, buf, Transceive);
	while(1)
	{
		if(temp == FALSE)
		{
			return(FM1702_NOTAGERR);
		}

		temp = read_reg(0x04);
		if(temp == 0)
		{
			return FM1702_BYTECOUNTERR;
		}

		Read_FIFO(buf);
		Save_UID(row, col, temp);			        // ���յ���UID����UID������
	
		temp = read_reg(0x0A);				        // �жϽӅ������Ƿ����
		temp = temp & 0x01;
		if(temp == 0x00)
		{
			temp = Check_UID();			            // У���յ���UID
			if(temp == FALSE)
			{
				return(FM1702_SERNRERR);
			}
			return(FM1702_OK);
		}
		else
		{
			temp = read_reg(0x0B);             // ��ȡ��ͻ���Ĵ��� 
			row = temp / 8;
			col = temp % 8;
			buf[0] = RF_CMD_ANTICOL;
			Set_BitFraming(row + pre_row, col);	// ���ô��������ݵ��ֽ��� 
			pre_row = pre_row + row;
			for(i = 0; i < pre_row + 1; i++)
			{
				buf[i + 2] = UID[i];
			}

			if(col != 0x00)
			{
				row = pre_row + 1;
			}
			else
			{
				row = pre_row;
			}
			temp = Command_Send(row + 2, buf, Transceive);
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

	write_reg(0x23,0x63);
	write_reg(0x12,0x3f);
	buf[0] = RF_CMD_SELECT;
	buf[1] = 0x70;
	for(i = 0; i < 5; i++)
	{
		buf[i + 2] = UID[i];
	}

	write_reg(0x22,0x0f);	                       // ����CRC,��żУ��У�� 
	temp = Command_Send(7, buf, Transceive);
	if(temp == FALSE)
	{
		return(FM1702_NOTAGERR);
	}
	else
	{
		temp = read_reg(0x0A);
		if((temp & 0x02) == 0x02) return(FM1702_PARITYERR);
		if((temp & 0x04) == 0x04) return(FM1702_FRAMINGERR);
		if((temp & 0x08) == 0x08) return(FM1702_CRCERR);
		temp = read_reg(0x04);
		if(temp != 1) return(FM1702_BYTECOUNTERR);
		Read_FIFO(buf);	                      // ��FIFO�ж�ȡӦ����Ϣ 
		temp = *buf;
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

	write_reg(0x23,0x63);
	write_reg(0x12,0x3f);
	write_reg(0x13,0x3f);
	temp1 = read_reg(0x09);
	temp1 = temp1 & 0xf7;
	write_reg(0x09,temp1);
	if(mode == RF_CMD_AUTH_LB)			            // AUTHENT1 
		buf[0] = RF_CMD_AUTH_LB;
	else
		buf[0] = RF_CMD_AUTH_LA;
	buf[1] = SecNR * 4 + 3;
	for(i = 0; i < 4; i++)
	{
		buf[2 + i] = UID[i];
	}

	write_reg(0x22,0x0f);	                     // ����CRC,��żУ��У�� 
	temp = Command_Send(6, buf, Authent1);
	if(temp == FALSE)
	{
		return FM1702_NOTAGERR;
	}

	temp = read_reg(0x0A);
	if((temp & 0x02) == 0x02) return FM1702_PARITYERR;
	if((temp & 0x04) == 0x04) return FM1702_FRAMINGERR;
	if((temp & 0x08) == 0x08) return FM1702_CRCERR;
	temp = Command_Send(0, buf, Authent2);	 // AUTHENT2 
	if(temp == FALSE)
	{ 
		return FM1702_NOTAGERR;
	}

	temp = read_reg(0x0A);
	if((temp & 0x02) == 0x02) return FM1702_PARITYERR;
	if((temp & 0x04) == 0x04) return FM1702_FRAMINGERR;
	if((temp & 0x08) == 0x08) return FM1702_CRCERR;
	temp1 = read_reg(0x09);
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

	write_reg(0x23,0x63);
	write_reg(0x12,0x3f);
	write_reg(0x13,0x3f);
	write_reg(0x22,0x0f);

	buff[0] = RF_CMD_READ;
	buff[1] = Block_Adr;
	temp = Command_Send(2, buff, Transceive);
	if(temp == 0)
	{
		return FM1702_NOTAGERR;
	}

	temp = read_reg(0x0A);
	if((temp & 0x02) == 0x02) return FM1702_PARITYERR;
	if((temp & 0x04) == 0x04) return FM1702_FRAMINGERR;
	if((temp & 0x08) == 0x08) return FM1702_CRCERR;
	temp = read_reg(0x04);
	if(temp == 0x10)	                      // 8K�������ݳ���Ϊ16
	{
		Read_FIFO(buff);
		return FM1702_OK;
	}
	else if(temp == 0x04)	                  // Token�������ݳ���Ϊ16
	{
		Read_FIFO(buff);
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

	write_reg(0x23,0x63);
	write_reg(0x12,0x3f);
//	F_buff = temp1 + 0x10;
	write_reg(0x22,0x07);
	F_buff[0] = RF_CMD_WRITE;
	F_buff[1] = Block_Adr;
	temp = Command_Send(2, F_buff, Transceive);
	if(temp == FALSE)
	{
		return(FM1702_NOTAGERR);
	}

	temp = read_reg(0x04);
	if(temp == 0)
	{
		return(FM1702_BYTECOUNTERR);
	}

	Read_FIFO(F_buff);
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

	temp = Command_Send(16, buff, Transceive);
	if(temp == TRUE)
	{
		return(FM1702_OK);
	}
	else
	{
		temp = read_reg(0x0A);
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
uchar ReadID_FM1702(void)
{
	uchar	temp;
	buf[0] = 0x00;
	buf[1] = 0x00;
	buf[2] = 0x10;
	temp = Command_Send(3, buf, ReadE2);
	delay_us(100);
	if(temp==TRUE)
	{
		Read_FIFO(buf);	                      // ��FIFO�ж�ȡӦ����Ϣ 
		delay_us(100);	
		return TRUE;	
	}
 return FALSE;
}

/****************************************************************/
/*����: Standby_2nd                                            */
/*����: �ú���ʵ�ֶ�FM1702 standby                               */
/*����: N/A                                                      */
/*���: TRUE ��ȡ�ɹ�����ӡ                                      */
/*      FALSE��ȡʧ�ܷ���FALSE                                   */
/*****************************************************************/
void RFID_RFclose(void)
{
	uchar	temp;
	temp = read_reg(TxControl);
	temp = temp & 0xFC;
	write_reg(TxControl,temp);
	delay_ms(1);	
}
void RFID_Powerdown(void)
{
	/*�ص�Դ*/
	uchar	temp;
	temp = read_reg(Control);
	temp = temp | 0x10;
	write_reg(Control,temp);
	delay_ms(1);
}
/****************************************************************/
/*����: Wakeup_2nd                                            */
/*����: �ú���ʵ�ֶ�FM1702 Wakeup                               */
/*����: N/A                                                      */
/*���: TRUE ��ȡ�ɹ�����ӡ                                      */
/*      FALSE��ȡʧ�ܷ���FALSE                                   */
/*****************************************************************/
void RFID_RFopen(void)
{
	/*������*/
	uchar	temp;
	temp = read_reg(TxControl);
	temp = temp | 0x03;
	write_reg(TxControl,temp);
	delay_ms(1);	
}
void RFID_Poweron(void)
{
	/*����Դ*/
	uchar	temp;
	temp = read_reg(Control);
	temp = temp & 0xef;
	write_reg(Control,temp);
	delay_ms(1);
}

uchar Read_tag(unsigned char hole)
{
	unsigned char status,loop = 0,s=0;
	unsigned char out_A[16],out_B[16],chipID_in[16];
	Chip_select(hole);
	RFID_RFopen();
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
							status=MIF_READ(buffer_temp,4);							       //��������ȡ1����0�����ݵ�buffer_temp[0]-buffer_temp[15]
							if (status == FM1702_OK)
							{
								if ((buffer_temp[0]==0xaa) && (buffer_temp[1]==0xaa)) //�����ʶ��ȷ0xaaaa�����Ƶ�buffer������TRUE
								{
									memcpy(buffer_rfid,buffer_temp,16);
									s++;
									RFID_RFclose();
									return TRUE;
								}
								//˫�װ�ID����
								if ((buffer_temp[0]==0xbb) && (buffer_temp[1]==0xbb)) //�����ʶ��ȷ0xBBBB��Ϊ˫�װ����ID��ֵ
								{
									//����Կ�׿׵�״̬����ת���ȴ�����
									FLASH_ReadByte(ADDR_Check_A,out_A,16);
									FLASH_ReadByte(ADDR_Check_B,out_B,16);
									out_A[2]=CLB_WAIT;
									FLASH_Write_Integrate(ADDR_Check_A,out_A);	
									out_B[2]=CLB_WAIT;
									FLASH_Write_Integrate(ADDR_Check_B,out_B);	
									//ΪԿ�׿�д��ID
									chipID_in[0]=buffer_temp[2];
									chipID_in[1]=buffer_temp[3];
									chipID_in[2]=buffer_temp[4];
									FLASH_Write_Integrate(ADDR_Check_ID,chipID_in);	
									
									Stm32_restart();	
								}
								//���װ�ID����
								if ((buffer_temp[0]==0xcc) && (buffer_temp[1]==0xcc)) //�����ʶ��ȷ0xCCCC��Ϊ���װ弰ǰ������ID��ֵ
								{
									//����Կ�׿׵�״̬����ת���ȴ�����
									FLASH_ReadByte(ADDR_Check_A,out_A,16);
									FLASH_ReadByte(ADDR_Check_B,out_B,16);
									out_A[2]=CLB_WAIT;
									FLASH_Write_Integrate(ADDR_Check_A,out_A);
									out_B[2]=CLB_WAIT;
									FLASH_Write_Integrate(ADDR_Check_B,out_B);
									//ΪԿ�׿�д��ID
									chipID_in[0]=buffer_temp[2];
									chipID_in[1]=buffer_temp[3];
									chipID_in[2]=0;
									FLASH_Write_Integrate(ADDR_Check_ID,chipID_in);	 
									flag_pollingA=0;						
									
									Stm32_restart();		
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
		RFID_RFclose();
		return TRUE;
	}
	RFID_RFclose();
	return FALSE;
}	

/****************************************************************/
/*����: Write_check_B                                            */
/*����: �ú���ʵ�������Ķ�������                                */ 
/*												       			                          */
/*����:                                                         */
/*      N/A                                                     */
/*                                                              */
/*���: TRUE or FALSE                                           */
/****************************************************************/
uchar Write_tag(unsigned char hole,unsigned char *buffer_temp_write)
{
	unsigned char status,loop = 0,s=0;
	Chip_select(hole);
	RFID_RFopen();
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
							status=MIF_Write(buffer_temp_write,4);							       //д����д1����0�����ݵ�buffer_temp[0]-buffer_temp[15]
							if (status == FM1702_OK)
							{
									s++;
									RFID_RFclose();
									return TRUE;
							}										
						}
					}						
				}
			}
		}
	}
	if (s>0)
	{
		RFID_RFclose();
		return TRUE;
	}
	RFID_RFclose();
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
uchar Read_check(unsigned char hole)
{
	unsigned char status ;
	Chip_select(hole);
	RFID_RFopen();
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
						status=MIF_READ(buffer_temp,4);							       //��������ȡ1����0�����ݵ�buffer_temp[0]-buffer_temp[15]			
						if (status == FM1702_OK)
						{
							if ((buffer_temp[0]==0xaa) && (buffer_temp[1]==0xaa)) //�����ʶ��ȷ0xaaaa�����Ƶ�buffer������TRUE
							{
								RFID_RFclose();
								return TRUE; 	
							}
						}
					}
				}						
			}
		}
	}
	 RFID_RFclose();
	 return FALSE;		
}


/****************************************************************/
/*����: Read_status_2nd                                         */
/*����: �ú���ʵ�����ζ���״̬�仯                              */ 
/*												       			                          */
/*����:                                                         */
/*      N/A                                                     */
/*                                                              */
/*���: TRUE or FALSE                                           */
/*      TRUE �б仯��FALSE �ޱ仯                               */    
/****************************************************************/
uchar Read_status(unsigned char hole)
{
	unsigned char status;
  status =  Read_tag(hole);
	if (status == TRUE && RFID_status_2nd==1)  //��Կ�����ޱ仯
	{
		return FALSE;
	}
	else if(status == FALSE && RFID_status_2nd==1) //��Կ�����б仯
	{
		memset(buffer_rfid,0,16*sizeof(char));          //��buffer
	
		RFID_status_2nd=0;
		return TRUE;
	}
	else if(status == TRUE && RFID_status_2nd==0) //��Կ�����б仯
	{
		RFID_status_2nd=1;
		return TRUE;
	}
	else if(status == FALSE && RFID_status_2nd==0) //��Կ�����ޱ仯
	{
		return FALSE;
	}
	else
	{
		return FALSE;
	}
}
//��������
void Stm32_restart(void)
{
		delay_ms(100);					
		__set_FAULTMASK(1);
		NVIC_SystemReset();	
}
//��⺯��
uchar calibration(unsigned char hole)
{

	unsigned char calibration_array[16]={0};	
	unsigned char RxThreshold_value=0;
	int i,j,max=0,maxb=0,xxx,count;
	Chip_select(hole);
	RFID_RFopen();
	max = calibration_array[1];
	if(hole == HOLE_A)
	{
	FLASH_ReadByte(ADDR_Check_A,calibration_array,16);	
	}
	if(hole == HOLE_B)
	{
	FLASH_ReadByte(ADDR_Check_B,calibration_array,16);	
	}
	RxThreshold_value =calibration_array[0];
	count = (RxThreshold_value>>4);
	if(RxThreshold_value==0xBF)//���״̬�ĸ�ֵ
	{
		calibration_array[count]=0;
		calibration_array[2] = CLB_FINISH;
		if(hole == HOLE_A)
		{
		FLASH_Write_Integrate(ADDR_Check_A,calibration_array);	
		}
		if(hole == HOLE_B)
		{
		FLASH_Write_Integrate(ADDR_Check_B,calibration_array);	
		}			
		LED_Control(hole,Led_Blue);
		delay_ms(1000);
		SendPacket(CAN_CALIB,hole,&calibration_array[15],1);
		Stm32_restart();		
		return TRUE;
	}
	else if(RxThreshold_value==0xAF)//��ȡ�����żĴ�����ֵ
	{
		calibration_array[count]=0;
		for(i=5;i<11;i++)
		{
			if(calibration_array[i]>max)
			{
				max=calibration_array[i];
				maxb=i;
			}
		}
		calibration_array[0] = RxThreshold_value+0x10;
		calibration_array[15] = maxb*16+0x0f;
		if(hole == HOLE_A)
		{
		FLASH_Write_Integrate(ADDR_Check_A,calibration_array);	
		}
		if(hole == HOLE_B)
		{
		FLASH_Write_Integrate(ADDR_Check_B,calibration_array);	
		}			
		LED_Control(hole,Led_Green);
		delay_ms(1000);
		if(max>200)
		{
			Stm32_restart();
		}
		if(max<=200)
		{
			calibration_array[0] = 0x77;
			calibration_array[count]=0;
			calibration_array[2] = CLB_ERROR;
			if(hole == HOLE_A)
			{
			FLASH_Write_Integrate(ADDR_Check_A,calibration_array);	
			}
			if(hole == HOLE_B)
			{
			FLASH_Write_Integrate(ADDR_Check_B,calibration_array);	
			}			
			LED_Control(hole,Led_White);
			delay_ms(10);
		  SendPacket(CAN_CALIB,hole,&calibration_array[0],1);
			Stm32_restart();				
		}
	}
	else//��ÿһ���Ĵ�������У׼
	{
		write_reg(RxThreshold,RxThreshold_value);// RxThreshold_value);
		calibration_array[count]=0;
		for(j=0;j<255;j++)
		{
			RFID_RFopen();				
			xxx =  Read_check(hole);
			RFID_RFclose();					
			if(xxx == TRUE) 
			{
				calibration_array[count]++;
			}
		}
		calibration_array[0] = RxThreshold_value+0x10;
		
		if(hole == HOLE_A)
		{
		FLASH_Write_Integrate(ADDR_Check_A,calibration_array);	
		}
		if(hole == HOLE_B)
		{
		FLASH_Write_Integrate(ADDR_Check_B,calibration_array);	
		}
		delay_ms(10);
		Stm32_restart();
	}
	return TRUE;
}



