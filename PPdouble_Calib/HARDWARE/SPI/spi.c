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
#include "spi.h"
#include "FM1702.h"
#include "stm32f10x.h"
#include "delay.h"
//SPIx 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
u8 SPI1_ReadWriteByte(u8 TxData)
{		
	u8 retry=0;				 	
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位
		{
		retry++;
		if(retry>200)return 0;
		}			  
	SPI_I2S_SendData(SPI1, TxData); //通过外设SPIx发送一个数据
	retry=0;

	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) //检查指定的SPI标志位设置与否:接受缓存非空标志位
		{
		retry++;
		if(retry>200)return 0;
		}	  						    
	return SPI_I2S_ReceiveData(SPI1); //返回通过SPIx最近接收的数据					    
}



//以下是SPI模块的初始化代码，配置成主机模式，					  
//SPI口初始化
//这里针是对SPI1的初始化



void SPI1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;


	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA, ENABLE );//PORTA时钟使能 
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_SPI1,  ENABLE );//SPI1时钟使能 	
//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO|RCC_APB2Periph_SPI1,ENABLE);
  GPIO_InitStructure.GPIO_Pin = MF522_RST_PIN | MF522_RST_PIN_2nd | MF522_NSS_PIN | MF522_NSS_PIN_2nd; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA,MF522_RST_PIN | MF522_RST_PIN_2nd | MF522_NSS_PIN | MF522_NSS_PIN_2nd);

//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 ;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 ; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //PB13/14/15复用推挽输出 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA

// 	GPIO_SetBits(GPIOA,GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7);  //PA5/6/7上拉

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//串行同步时钟的空闲状态为高电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//串行同步时钟的第二个跳变沿（上升或下降）数据被采样
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;		//定义波特率预分频的值:波特率预分频值为256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	SPI_Init(SPI1, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
//  SPI_SSOutputCmd(SPI1, ENABLE);

	SPI_Cmd(SPI1, ENABLE); //使能SPI外设
//	SPI1_SetSpeed(SPI_BaudRatePrescaler_256);
//	SPI1_ReadWriteByte(0xff);//启动传输		 
 

}   
//SPI 速度设置函数
//SpeedSet:
//SPI_BaudRatePrescaler_2   2分频   
//SPI_BaudRatePrescaler_8   8分频   
//SPI_BaudRatePrescaler_16  16分频  
//SPI_BaudRatePrescaler_256 256分频 
void SPI1_Close(void)
{
	
//	GPIO_InitTypeDef GPIO_InitStructure;
////  SPI_InitTypeDef  SPI_InitStructure;
//	
//  GPIO_InitStructure.GPIO_Pin = MF522_RST_PIN | MF522_RST_PIN_2nd | MF522_NSS_PIN | MF522_NSS_PIN_2nd; 
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);

//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 ; 
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //PB13/14/15复用推挽输出 
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA	
  SPI_Cmd(SPI1,DISABLE); 
	SPI_I2S_DeInit(SPI1);
//	  RCC_APB2PeriphClockCmd(	RCC_APB2RSTR_SPI1RST,  ENABLE );
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_SPI1,  DISABLE );//SPI1时钟使能 
	
	

//	  RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA, DISABLE );//PORTA时钟使能 
//	  RCC_APB2PeriphClockCmd(	RCC_APB2RSTR_SPI1RST,  ENABLE );//SPI1时钟使能 
} 
void SPI1_Open(void)
{
	
//	GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;
//	
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA, ENABLE );//PORTA时钟使能 
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_SPI1,  ENABLE );//SPI1时钟使能 	

//  GPIO_InitStructure.GPIO_Pin = MF522_RST_PIN | MF522_RST_PIN_2nd | MF522_NSS_PIN | MF522_NSS_PIN_2nd; 
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);
//	GPIO_SetBits(GPIOA,MF522_RST_PIN | MF522_RST_PIN_2nd | MF522_NSS_PIN | MF522_NSS_PIN_2nd);

//	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 ; 
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //PB13/14/15复用推挽输出 
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA

// 	GPIO_SetBits(GPIOA,GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7);  //PA5/6/7上拉
//	RCC_APB2PeriphClockCmd(	RCC_APB2RSTR_SPI1RST,  DISABLE );
//	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_SPI1,  ENABLE );//SPI1时钟使能
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//串行同步时钟的空闲状态为高电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//串行同步时钟的第二个跳变沿（上升或下降）数据被采样
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;		//定义波特率预分频的值:波特率预分频值为256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	SPI_Init(SPI1, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器  	
	SPI_Cmd(SPI1,ENABLE); 
//	  RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA, ENABLE );//PORTA时钟使能 
//	  RCC_APB2PeriphClockCmd(	RCC_APB2RSTR_SPI1RST,  DISABLE );//SPI1时钟使能 
	delay_ms(50);
} 

void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler)
{
  assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
	SPI1->CR1&=0XFFC7;
	SPI1->CR1|=SPI_BaudRatePrescaler;	//设置SPI1速度 
	SPI_Cmd(SPI1,ENABLE); 

} 

































