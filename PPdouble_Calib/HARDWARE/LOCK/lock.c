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
 * Engineer							: Ëï³ÉÎÄ
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
#include "lock.h"
#include "delay.h"
#include "commhead.h"

unsigned char LOCK_OFF = 0;
unsigned char LOCK_ON = 1;
void LOCK_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 
 	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;	
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 
 GPIO_Init(GPIOB, &GPIO_InitStructure);	
 
 GPIO_ResetBits(GPIOA,GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11);
}
void Lock_Control(unsigned char hole,unsigned char *data)
{
	if(hole==HOLE_A)
	{
		if(*data==LOCK_OFF)
		{
			LOCK1P =0;
			LOCK1N =1;
			delay_ms(100);
			LOCK1P =0;
			LOCK1N =0;		
		}
		else
		{
			LOCK1P =1;
			LOCK1N =0;
			delay_ms(100);
			LOCK1P =0;
			LOCK1N =0;		
		}
	}
	else if(hole==HOLE_B)
	{
		if(*data==LOCK_OFF)
		{
			LOCK2N =1;
			LOCK2P =0;
			delay_ms(100);
			LOCK2N =0;
			LOCK2P =0;			
		}
		else
		{
			LOCK2N =0;
			LOCK2P =1;
			delay_ms(100);
			LOCK2N =0;
			LOCK2P =0;		
		}
	}
	else
	{
	}
}
