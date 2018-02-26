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
    KEY_Init();	 //	�����˿ڳ�ʼ��


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
		
//		 NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;			//ʹ�ܰ���WK_UP���ڵ��ⲿ�ж�ͨ��
//  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//��ռ���ȼ�2�� 
//  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;					//�����ȼ�3
//  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
//  	NVIC_Init(&NVIC_InitStructure); 

 
}
void EXTI9_5_IRQHandler(void)//A��
{
  delay_ms(50);//����
	if(KEY_A==0)	  //��������
	{
		LED_Control(HOLE_A,Led_Red);
		key_A = PP_IN;
	}
	else	  //����̧��
	{
	 LED_Control(HOLE_A,Led_Green);
	 key_A = PP_OUT;
	}
	if((flag_return==0) && (flag_borrow==0))
	{
		SendPacket(CAN_RETURN_RFID,HOLE_A,&Return_Fail,1);
	}
	EXTI_ClearITPendingBit(EXTI_Line9);  //���LINE2�ϵ��жϱ�־λ  
}
 
void EXTI15_10_IRQHandler(void)//B��
{
	delay_ms(50);//����//delay_ms(20);
	if(KEY_B==0)	  //��������
	{
		LED_Control(HOLE_B,Led_Red);
		key_B = PP_IN;
	}
	else	  //����̧��
	{
		LED_Control(HOLE_B,Led_Green);
		key_B = PP_OUT;		
	}
	if((flag_return==0) && (flag_borrow==0))
	{
		SendPacket(CAN_RETURN_RFID,HOLE_B,&Return_Fail,1);
	}
	EXTI_ClearITPendingBit(EXTI_Line10);  //���LINE2�ϵ��жϱ�־λ  
}

