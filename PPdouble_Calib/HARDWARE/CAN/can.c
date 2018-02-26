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
#include "can.h"
#include "led.h"
#include "delay.h"
#include "usart.h"
#include "lock.h"
#include "string.h"
#include "commhead.h"
#include "FM1702.h"

u8 recvBuffer[1024];
u8 relayBuffer[128];
u8 sendBuffer[32];
u16 recvBufferCount=0;
u8 relayBuf[16];
u8 *WRPtr=recvBuffer;
u8 *RDPtr=recvBuffer;

u8 choiceFlagA=0;
u8 choiceFlagB=0;
u8 chipID_hole;
u8 chipID_A;
u8 chipID_B;//chipID_B is "B", chipID_A is "A"


u16 std_id =0x0100;
u8 flag_pollingA=0; 
u8 flag_pollingB=0; 




u8 CAN_Mode_Init(u8 tsjw,u8 tbs2,u8 tbs1,u16 brp,u8 mode)
{
		volatile unsigned int EXT_value_1 = 0;
		volatile unsigned int EXT_value_2 = 0;
	  volatile unsigned int EXT_value_shield = 0x00FFFF00;//为了过滤树莓派的源ID与目标ID
		EXT_value_1 = ((destID<<8)+chipID_hole)<<8;
    GPIO_InitTypeDef 		GPIO_InitStructure;
    CAN_InitTypeDef        	CAN_InitStructure;
    CAN_FilterInitTypeDef  	CAN_FilterInitStructure;
#if CAN_RX0_INT_ENABLE
    NVIC_InitTypeDef  		NVIC_InitStructure;
#endif

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//使能PORTA时钟



    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);//使能CAN1时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽
    GPIO_Init(GPIOA, &GPIO_InitStructure);			//初始化IO

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//上拉输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);			//初始化IO

    //CAN单元设置
    CAN_InitStructure.CAN_TTCM=DISABLE;			//非时间触发通信模式
    CAN_InitStructure.CAN_ABOM=DISABLE;			//软件自动离线管理
    CAN_InitStructure.CAN_AWUM=DISABLE;			//睡眠模式通过软件唤醒(清除CAN->MCR的SLEEP位)
    CAN_InitStructure.CAN_NART=ENABLE;			//禁止报文自动传送
    CAN_InitStructure.CAN_RFLM=DISABLE;		 	//报文不锁定,新的覆盖旧的
    CAN_InitStructure.CAN_TXFP=DISABLE;			//优先级由报文标识符决定
    CAN_InitStructure.CAN_Mode= mode;	        //模式设置： mode:0,普通模式;1,回环模式;
    //设置波特率
    CAN_InitStructure.CAN_SJW=tsjw;				//重新同步跳跃宽度(Tsjw)为tsjw+1个时间单位  CAN_SJW_1tq	 CAN_SJW_2tq CAN_SJW_3tq CAN_SJW_4tq
    CAN_InitStructure.CAN_BS1=tbs1; 			//Tbs1=tbs1+1个时间单位CAN_BS1_1tq ~CAN_BS1_16tq
    CAN_InitStructure.CAN_BS2=tbs2;				//Tbs2=tbs2+1个时间单位CAN_BS2_1tq ~	CAN_BS2_8tq
    CAN_InitStructure.CAN_Prescaler=brp;        //分频系数(Fdiv)为brp+1
    CAN_Init(CAN1, &CAN_InitStructure);        	//初始化CAN1

    CAN_FilterInitStructure.CAN_FilterNumber=0;	//过滤器0
    CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask ;//CAN_FilterMode_IdList; 	//屏蔽位模式
    CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit; 	//32位宽
    CAN_FilterInitStructure.CAN_FilterIdHigh=((EXT_value_1<<3) >>16) &0xffff;//0x0001;//EXT_value>>16;//<<5;	//32位ID
    CAN_FilterInitStructure.CAN_FilterIdLow=(u16)(EXT_value_1<<3) | CAN_ID_EXT;//0x0000;//(((EXT_value<<16)>>24)<<8)|0x02;

    CAN_FilterInitStructure.CAN_FilterMaskIdHigh=((EXT_value_shield<<3) >>16) &0xffff;;//EXT_value>>16;//EXT_shield>>16;//0x0000;//32位MASK
    CAN_FilterInitStructure.CAN_FilterMaskIdLow=(u16)(EXT_value_shield<<3) | CAN_ID_EXT;//(((EXT_value<<16)>>24)<<8)|0x02;//(((EXT_shield<<16)>>24)<<8)+0x04;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_Filter_FIFO0;//过滤器0关联到FIFO0
    CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;//激活过滤器0

    CAN_FilterInit(&CAN_FilterInitStructure);			//滤波器初始化

#if CAN_RX0_INT_ENABLE
    CAN_ITConfig(CAN1,CAN_IT_FMP0,ENABLE);				//FIFO0消息挂号中断允许.

    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;     // 主优先级为1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;            // 次优先级为0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif
    return 0;
}
unsigned char data0;
unsigned char flag_light;
unsigned char count;
unsigned char flag_headjudge;
#if CAN_RX0_INT_ENABLE	//使能RX0中断
//中断服务函数
void USB_LP_CAN1_RX0_IRQHandler(void)
{
		struct pica_can_cmd body;
    CanRxMsg rx_msg;
    CAN_Receive(CAN1, 0, &rx_msg);
	
		body.cmd_type = rx_msg.ExtId>>24;
		body.hole = ((unsigned char)(rx_msg.ExtId))>>5;
		body.frame_idx = rx_msg.ExtId & 0x0000001F;	
	  body.data_length = rx_msg.DLC;
    if(RDPtr==WRPtr)                        //指针相等意味着buffer读完了，把读写指针指向buffer初始位置
    {
        WRPtr=recvBuffer;           
        RDPtr=recvBuffer;           
		}
		*WRPtr = body.cmd_type;
		*(WRPtr+1) = body.hole;
		*(WRPtr+2) = body.frame_idx;
		*(WRPtr+3) = body.data_length;
		memcpy(recvBuffer + (WRPtr-recvBuffer+CAN_IDENTIFIER_LEN), rx_msg.Data,rx_msg.DLC);  //读指针和写指针都指向下一个
		WRPtr = WRPtr + rx_msg.DLC+CAN_IDENTIFIER_LEN;		

		
//        }	
//    if((RxMessage.Data[0]==chipID_B)||(RxMessage.Data[0]==chipID_A)||(RxMessage.Data[0]==0x00))
//   {
//        if(RDPtr==WRPtr)                        //每次来到新的数据的时候，如果读指针不在首地址，则同时将读指针和写指针向首地址方向移动(RDPtr-recvBuffer>0)
//        {
//            WRPtr=recvBuffer;//+(WRPtr-RDPtr);            //move the pointer backwards
//            RDPtr=recvBuffer;                          //the read pointer is fixed to the begining after the new data arrival
//        }
//        if((RxMessage.Data[1]&0x80) == 0x80 )          // the MSB of the source ID is set to 1 if this frame is the first one of the packet
//        {
//            if(RxMessage.Data[0] == 0)
//            {
//                if(RxMessage.Data[6] > 8)                  //flag_headjudge 这个信号目的是避免丢帧
//								{ flag_headjudge = 1;}
//								memcpy(recvBuffer + (WRPtr-recvBuffer), RxMessage.Data,RxMessage.DLC);  //读指针和写指针都指向下一个
//                WRPtr = WRPtr + RxMessage.DLC;
//            }
//						else if ((RxMessage.Data[3]==2) && (RxMessage.Data[0]==(chipID_A)))  //操作锁
//						{
//							if(RxMessage.Data[4])    //开锁
//              {

//             memcpy(recvBuffer + (WRPtr-recvBuffer), RxMessage.Data,RxMessage.DLC);  //读指针和写指针都指向下一个
//             WRPtr = WRPtr + RxMessage.DLC;	
//						
//              }
//              else
//              {   
//						  flag_pollingA=0;
//						 memcpy(recvBuffer + (WRPtr-recvBuffer), RxMessage.Data,RxMessage.DLC);  //读指针和写指针都指向下一个
//             WRPtr = WRPtr + RxMessage.DLC;			
//              }
//						}
//						else if ((RxMessage.Data[3]==2) && (RxMessage.Data[0]==chipID_B))  //操作锁
//						{
//							if(RxMessage.Data[4])    //开锁
//              {
//						  memcpy(recvBuffer + (WRPtr-recvBuffer), RxMessage.Data,RxMessage.DLC);  //读指针和写指针都指向下一个
//              WRPtr = WRPtr + RxMessage.DLC;
//												
//              }
//              else
//              {
//						  flag_pollingB=0;
//							memcpy(recvBuffer + (WRPtr-recvBuffer), RxMessage.Data,RxMessage.DLC);  //读指针和写指针都指向下一个
//              WRPtr = WRPtr + RxMessage.DLC;					
//              }
//						}

//						else
//            {							
//             memcpy(recvBuffer + (WRPtr-recvBuffer), RxMessage.Data,RxMessage.DLC);  //读指针和写指针都指向下一个
//             WRPtr = WRPtr + RxMessage.DLC;
//												
//						}
//	
//        }
//        else
//        {
//            if(flag_headjudge)
//            {
//                flag_headjudge =0;
//                memcpy(recvBuffer + (WRPtr-recvBuffer), &RxMessage.Data[2],RxMessage.DLC-2);// 非第一帧数据
//                WRPtr = WRPtr + RxMessage.DLC-2;
//											
//            }
//        }
//    }

}


#endif

//can发送一组数据(固定格式:ID为0X12,标准帧,数据帧)
//len:数据长度(最大为8)
//msg:数据指针,最大为8个字节.
//返回值:0,成功;
//		 其他,失败;
u8 Can_Send_Msg(u8 cmd_type,unsigned char hole,unsigned char frame_idx,u8* msg,u8 len)
{
    u8 mbox;
    u16 i=0;
	  volatile unsigned int EXT_value = 0;
	  EXT_value = cmd_type;
	  EXT_value = (EXT_value<<8)+chipID_hole;
	  EXT_value = (EXT_value<<8)+destID;
	  EXT_value = (EXT_value<<8)+((hole<<5)+frame_idx);
    CanTxMsg TxMessage;
    TxMessage.ExtId=EXT_value;//0x00;			// 设置扩展标示符
    TxMessage.IDE=CAN_ID_EXT;//CAN_Id_Standard; 	// 标准帧
    TxMessage.RTR=CAN_RTR_Data;		// 数据帧
    TxMessage.DLC=len;				// 要发送的数据长度
    for(i=0; i<len; i++)
        TxMessage.Data[i]=msg[i];
    mbox= CAN_Transmit(CAN1, &TxMessage);

	
    i=0;
    while((CAN_TransmitStatus(CAN1, mbox)==CAN_TxStatus_Failed)&&(i<0XFFF))i++;	//等待发送结束
    if(i>=0XFFF)
        return 1;
    return 0;
}
//can口接收数据查询
//buf:数据缓存区;
//返回值:0,无数据被收到;
//		 其他,接收的数据长度;
u8 Can_Receive_Msg(u8 *buf)
{
    u32 i;
    CanRxMsg RxMessage;
    if( CAN_MessagePending(CAN1,CAN_FIFO0)==0)
        return 0;		//没有接收到数据,直接退出
    CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);//读取数据
    for(i=0; i<8; i++)
        buf[i]=RxMessage.Data[i];
    return RxMessage.DLC;
}


//void SendPacket(u8 SouID,u8 ACKorNOTI)
void SendPacket(u8 cmd_type,unsigned char hole,u8* msg,unsigned char data_len_all)
{
		int data_len_last,frame_idx;
		frame_idx = ((data_len_all / 8) + ((data_len_all % 8) != 0 ? 1 : 0))-1;
		data_len_last =(data_len_all % 8) != 0 ? (data_len_all % 8) : 8;
		for(int i=frame_idx;i>=0;i--)
		{
			if(i==0)
			{
			Can_Send_Msg(cmd_type,hole,i,msg+frame_idx*8,data_len_last);
			}
			else
			{
			Can_Send_Msg(cmd_type,hole,i,msg+(frame_idx-i)*8,8);
			}
		}
	

//    unsigned char size;
//		unsigned char size_quotient;
//		unsigned char size_remainder;
//		int i;
//   // unsigned char flag;
//    unsigned char CanSend[8];
//	  size = sizeof(relayBuf);
//	  size_quotient = size/8;
//		size_remainder = size%8;
//		if(size_quotient==0|size_quotient==1)
//		{
//		Can_Send_Msg(command_send,SouID,0,relayBuf,size);
//		}
//		else
//		{
//			for(i=0;i<size_quotient;i++)
//			{
//				if(i==size_quotient-1)
//				{
//					if(size_remainder==0)
//					{size_remainder=8;}
//					memcpy(&(CanSend),relayBuf+i*8,size_remainder);
//					Can_Send_Msg(command_send,SouID,size_quotient-i-1,CanSend,size_remainder);
//				}
//				else	
//				{					
//					memcpy(&(CanSend),relayBuf+i*8,8);
//					Can_Send_Msg(command_send,SouID,size_quotient-i-1,CanSend,8);
//				}
//			}
//		}


}










