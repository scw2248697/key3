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
 #include "led.h"
 #include "commhead.h"
unsigned char Led_Red[3]={1,0,0};
unsigned char Led_Green[3]={0,1,0};
unsigned char	Led_Blue[3]={0,0,1};
unsigned char	Led_Magenta[3]={1,0,1};
unsigned char	Led_Cyan[3]={0,1,1};
unsigned char Led_Yellow[3]={1,1,0};
unsigned char Led_White[3]={1,1,1};

void LED_Init(void)
{
 
	 GPIO_InitTypeDef  GPIO_InitStructure;
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	 GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	 

		
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;				
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	 GPIO_Init(GPIOB, &GPIO_InitStructure);					 
	 GPIO_ResetBits(GPIOB,GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7);
		
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4;				
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	 GPIO_Init(GPIOB, &GPIO_InitStructure);					 
	 GPIO_ResetBits(GPIOB,GPIO_Pin_3|GPIO_Pin_4);
		
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;				
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	 GPIO_Init(GPIOA, &GPIO_InitStructure);					 
	 GPIO_ResetBits(GPIOA,GPIO_Pin_15);					
}
void LED_Control(unsigned char hole,unsigned char *data)
{
	if(hole==HOLE_A)
	{	
		LEDR_A = *data;
		LEDG_A = *(data+1);
		LEDB_A = *(data+2);
	}
	else if(hole==HOLE_B)
	{
		LEDR_B = *data;
		LEDG_B = *(data+1);
		LEDB_B = *(data+2);		
	}
	else
	{
		LEDR_A = 1;LEDG_A = 1;LEDB_A = 1;
		LEDR_B = 1;LEDG_B = 1;LEDB_B = 1;
	}
}
