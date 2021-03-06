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
 * Engineer							: 孙成文
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

#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "can.h"
#include "beep.h"
#include "exti.h"
#include "timer.h"
#include "lock.h"
#include "commhead.h"
#include "string.h"
#include "stdlib.h"
#include "math.h"
#include "stdio.h"
#include "can.h"
#include "FM1702.h"
#include "spi.h"
#include "DataFlash.h"
//#pragma pack(2)

u8 res;
u16 length;
u8 rfid_status;
u8 rfid_statusB;
unsigned char out_main_A[16]={0};//检测函数的返回值，里面包含了FM1702中RxThreshold的帧.其中out[15]为最终需要的值。out[2]=0xFF为没有检测，out[2]=0x11为检测完毕，out[2]=0x55为正在检测。
unsigned char out_main_B[16]={0};//检测函数的返回值，里面包含了FM1702中RxThreshold的帧.其中out[15]为最终需要的值。out[2]=0xFF为没有检测，out[2]=0x11为检测完毕，out[2]=0x55为正在检测�
unsigned char rfid_match_A[16]={0};
unsigned char rfid_match_B[16]={0};
unsigned char chipID[16];
unsigned char flag_return = 0;
unsigned char flag_borrow = 0;
unsigned char play_ff[16]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
unsigned char test_A[16]={0xff,0xff,0x11,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x6f};
unsigned char test_B[16]={0xff,0xff,0x11,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x6f};
unsigned char test_ID[3]={0x02,0x00,0x00};
void Return_Process(void)
{
	if(flag_return==HOLE_RETURN_A)
	{
		rfid_status = Read_tag(HOLE_A);
		if(rfid_status && (key_A == PP_IN))
		{
			memcpy(relayBuf,buffer_rfid,16);
			if((memcmp(relayBuf,rfid_match_A,Array_Len(rfid_match_A)))==0)
			{
				Lock_Control(HOLE_A,&LOCK_OFF);
				SendPacket(CAN_RETURN_RFID,HOLE_A,&Return_Suc,1);
				key_A = PP_NONE;
				flag_return=0;
			}
			else
			{
				SendPacket(CAN_ERROR_RFID,HOLE_A,relayBuf,Array_Len(relayBuf));
				key_A = PP_NONE;
			}
			
		}
	}
	else if(flag_return==HOLE_RETURN_B)
	{
		rfid_status = Read_tag(HOLE_B);
		if(rfid_status && (key_B == PP_IN))
		{
			memcpy(relayBuf,buffer_rfid,16);
			if((memcmp(relayBuf,rfid_match_B,Array_Len(rfid_match_B)))==0)
			{
				Lock_Control(HOLE_B,&LOCK_OFF);
				SendPacket(CAN_RETURN_RFID,HOLE_B,&Return_Suc,1);
				key_B = PP_NONE;
				flag_return=0;
			}
			else
			{
				SendPacket(CAN_ERROR_RFID,HOLE_B,relayBuf,Array_Len(relayBuf));
				key_A = PP_NONE;
			}
		}		
	}
	else
	{
	}
}

void Borrow_Process(void)
{
	if(flag_borrow==HOLE_BORROW_A)
	{
		rfid_status = Read_tag(HOLE_A);
		if((rfid_status==0) && (key_A == PP_OUT))
		{
			SendPacket(CAN_BORROW_RFID,HOLE_A,&Borrow_Suc,1);
			Lock_Control(HOLE_A,&LOCK_OFF);
			key_A = PP_NONE;
			flag_borrow=0;		
		}		
	}
	else if(flag_borrow==HOLE_BORROW_B)
	{
		rfid_status = Read_tag(HOLE_B);
		if((rfid_status==0) && (key_B == PP_OUT))
		{
			SendPacket(CAN_BORROW_RFID,HOLE_B,&Borrow_Suc,1);
			Lock_Control(HOLE_B,&LOCK_OFF);
			key_B = PP_NONE;
			flag_borrow=0;		
		}				
	}
	else
	{
	}
}

int main(void)
{
	  /*将晶振变为内部晶振，主要原因是射频电路对外部晶振产生了干扰*/
    RCC_ClocksTypeDef  RCC_Clocks; 
	  SystemInit(); 
	  RCC_GetClocksFreq(&RCC_Clocks); 
	  /*各类初始化*/
    u8 mode=CAN_Mode_Normal;//CAN工作模式;CAN_Mode_Normal(0)：普通模式，CAN_Mode_LoopBack(1)：环回模式
    delay_init();	    	 //延时函数初始化
		FLASH_ReadByte(ADDR_Check_A,out_main_A,16);
		FLASH_ReadByte(ADDR_Check_B,out_main_B,16);
		FLASH_ReadByte(ADDR_Check_ID,chipID,3);
		FLASH_ReadByte(RFID_MATCH_A,rfid_match_A,16);
		FLASH_ReadByte(RFID_MATCH_B,rfid_match_B,16);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
    LED_Init();		  		//初始化与LED连接的硬件接口
    LOCK_Init();        //初始化与锁连接的硬件接口
	  SPI1_Init(); //SPI初始化
	  SPI1_SetSpeed(SPI_BaudRatePrescaler_256);//SPI速度设置
		Init_FM1702(HOLE_A);
		Init_FM1702(HOLE_B);
		/*读取FLASH的值，并为孔ID进行赋值*/	
    chipID_hole = chipID[0]; 
		chipID_A		=	chipID[1];
		chipID_B		=	chipID[2];
	  CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_7tq,CAN_BS1_8tq,6,mode);//CAN初始化环回模式,波特率500Kbps 内部晶振时的CAN设置
		EXTIX_Init();

//			FLASH_Write_Integrate(ADDR_Check_B,play_ff);
//			FLASH_Write_Integrate(ADDR_Check_A,play_ff);
//			FLASH_Write_Integrate(ADDR_Check_ID,play_ff);
//			FLASH_Write_Integrate(RFID_MATCH_A,play_ff);
//			FLASH_Write_Integrate(RFID_MATCH_B,play_ff);

//			FLASH_Write_Integrate(ADDR_Check_B,test_A);
//			FLASH_Write_Integrate(ADDR_Check_A,test_B);
//			FLASH_Write_Integrate(ADDR_Check_ID,test_ID);


    while(1)
    {
			struct pica_can_cmd body;
			
			if(out_main_A[2]==CLB_ING)//A孔进入校准模式
			{
				LED_Control(HOLE_A,Led_Yellow);
				calibration(HOLE_A);	
			}
			else if(out_main_B[2]==CLB_ING)//B孔进入校准模式
			{
				LED_Control(HOLE_B,Led_Yellow);
				calibration(HOLE_B);
			}
			else
			{
				if(WRPtr>RDPtr)
				{
					body.cmd_type = *RDPtr;
					body.hole = *(RDPtr+1);
					body.frame_idx = *(RDPtr+2);
					body.data_length = *(RDPtr+3);
					memcpy(body.data,(RDPtr+4),body.data_length);
					switch(body.cmd_type)
					{					
							case CAN_SET_LED:
							{	
								LED_Control(body.hole,body.data);
							}
							break; 
							case CAN_SET_LOCK:
							{
								Lock_Control(body.hole,body.data);								
							}
							break;
							case CAN_SET_LEDLOCK:
							{	
								LED_Control(body.hole,body.data);
								Lock_Control(body.hole,body.data+3);		
							}
							break;
							case CAN_GET_RFID:
							{
								rfid_status = Read_tag(body.hole);																
								if(rfid_status)
								{
									memcpy(relayBuf,buffer_rfid,16);
									SendPacket(CAN_GET_RFID,body.hole,relayBuf,Array_Len(relayBuf));
								}
								else
								{
									memset(relayBuf,0x00,16);
									SendPacket(CAN_GET_RFID,body.hole,relayBuf,Array_Len(relayBuf));
								}								
							}
							break;
							case CAN_SET_RFID:
							{					
								if(body.frame_idx !=0)
								{
									for(int i=0;i<8;i++)
									{
										relayBuf[i] = body.data[i];
									}
								}
								else
								{
									for(int i=0;i<8;i++)
									{
										relayBuf[i+8] = body.data[i];
									}
									Write_tag(body.hole,relayBuf);
									SendPacket(CAN_SET_RFID,body.hole,relayBuf,Array_Len(relayBuf));								
								}
							}
							break;
							case CAN_RETURN_RFID:
							{	
								if(body.data[0])
								{
									Lock_Control(body.hole,&LOCK_ON);	
									if(body.hole==HOLE_A)
									{
										flag_return = HOLE_RETURN_A;
									}
									else if(body.hole==HOLE_B)
									{
										flag_return = HOLE_RETURN_B;
									}
									else
									{
									}
								}
								else
								{
									Lock_Control(body.hole,&LOCK_OFF);
									flag_return = 0;									
								}
							}
							break;
							case CAN_BORROW_RFID:
							{
								if(body.data[0])
								{
									Lock_Control(body.hole,&LOCK_ON);	
									if(body.hole==HOLE_A)
									{
										flag_borrow = HOLE_BORROW_A;
									}
									else if(body.hole==HOLE_B)
									{
										flag_borrow = HOLE_BORROW_B;
									}
									else
									{
									}
								}
								else
								{
									Lock_Control(body.hole,&LOCK_OFF);
									flag_borrow = 0;									
								}								
							}
							break;
							case CAN_ERROR_RFID:
							{	
							}
							break;
							case CAN_CALIB:
							{
								if(body.hole == HOLE_A)
								{
									out_main_A[0]=0x3F;out_main_A[2]=CLB_ING;														
									FLASH_Write_Integrate(ADDR_Check_A,out_main_A);										
									Stm32_restart();
								}
								else if(body.hole == HOLE_B)
								{
									out_main_B[0]=0x3F;out_main_B[2]=CLB_ING;														
									FLASH_Write_Integrate(ADDR_Check_B,out_main_B);										
									Stm32_restart();
								}	
								else
								{								
								}									
							}
							break;
							case CAN_MATCH:
							{
									rfid_status = Read_tag(body.hole);	
									if(rfid_status)
									{
										memcpy(relayBuf,buffer_rfid,16);
										if(body.hole == HOLE_A)
										{
											FLASH_Write_Integrate(RFID_MATCH_A,relayBuf);
										}
										else if(body.hole == HOLE_B)
										{
											FLASH_Write_Integrate(RFID_MATCH_B,relayBuf);
										}
										else
										{
										}
										SendPacket(CAN_MATCH,body.hole,relayBuf,Array_Len(relayBuf));
										Stm32_restart();
									}
									else
									{
									}
							}
							break;
							case CAN_GET_HOLE_INFO:
							{
							}
							break;
							default:
							{
							}
							break;
					}
					RDPtr = RDPtr + body.data_length+CAN_IDENTIFIER_LEN;
				}
			}
			Return_Process();
			Borrow_Process();

//	/*如果没有校准过的孔,指示灯亮起。并且打开A孔的天线等待写ID*/
						if((out_main_A[2]==NO_ID)&(out_main_B[2]==NO_ID))
						{
							 rfid_status = Read_tag(HOLE_A);
							 LEDR_A = 0;LEDG_A = 1;LEDB_A = 0;LEDR_B = 0;LEDG_B = 0;LEDB_B = 0;
							 delay_ms(100);
							 
						}
//						if((out_main_A[2]==CLB_WAIT)&(out_main_B[2]==CLB_WAIT))
//						{
//							 LEDR_A = 0;LEDG_A = 0;LEDB_A = 1;LEDR_B = 0;LEDG_B = 0;LEDB_B = 1;
//							 delay_ms(1000);
//							 
//						}
						if(out_main_A[2]==CLB_ERROR)
						{
							 LEDR_A = 1;LEDG_A = 1;LEDB_A = 0;				
							 delay_ms(1000); 
						}
						if(out_main_B[2]==CLB_ERROR)
						{
							 LEDR_B = 1;LEDG_B = 1;LEDB_B = 0;							
							 delay_ms(1000);
						}
    }
}


