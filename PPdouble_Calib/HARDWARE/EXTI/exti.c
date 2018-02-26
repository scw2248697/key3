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
#include "exti.h"
#include "led.h"
#include "key.h"
#include "delay.h"
#include "usart.h"
#include "beep.h"
#include "FM1702.h"
#include "can.h"
#include "commhead.h"

u8 key_test;
unsigned char key_A=0;
unsigned char key_B=0;
void EXTIX_Init(void)
{
 
 	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
    KEY_Init();	 //	按键端口初始化


		NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure);
        
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
    NVIC_Init(&NVIC_InitStructure);		
		
    EXTI_ClearITPendingBit(EXTI_Line9);
    EXTI_ClearITPendingBit(EXTI_Line10);	
	
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource9); 
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource10); 
  	EXTI_InitStructure.EXTI_Line=EXTI_Line10|EXTI_Line9;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;//EXTI_Trigger_Rising_Falling;//
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	
		
//		 NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;			//使能按键WK_UP所在的外部中断通道
//  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2， 
//  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;					//子优先级3
//  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
//  	NVIC_Init(&NVIC_InitStructure); 

 
}
void EXTI9_5_IRQHandler(void)//A孔
{
  delay_ms(50);//消抖
	if(KEY_A==0)	  //按键按下
	{
		LED_Control(HOLE_A,Led_Red);
		key_A = PP_IN;
	}
	else	  //按键抬起
	{
	 LED_Control(HOLE_A,Led_Green);
	 key_A = PP_OUT;
	}
	if((flag_return==0) && (flag_borrow==0))
	{
		SendPacket(CAN_RETURN_RFID,HOLE_A,&Return_Fail,1);
	}
	EXTI_ClearITPendingBit(EXTI_Line9);  //清除LINE2上的中断标志位  
}
 
void EXTI15_10_IRQHandler(void)//B孔
{
	delay_ms(50);//消抖//delay_ms(20);
	if(KEY_B==0)	  //按键按下
	{
		LED_Control(HOLE_B,Led_Red);
		key_B = PP_IN;
	}
	else	  //按键抬起
	{
		LED_Control(HOLE_B,Led_Green);
		key_B = PP_OUT;		
	}
	if((flag_return==0) && (flag_borrow==0))
	{
		SendPacket(CAN_RETURN_RFID,HOLE_B,&Return_Fail,1);
	}
	EXTI_ClearITPendingBit(EXTI_Line10);  //清除LINE2上的中断标志位  
}

