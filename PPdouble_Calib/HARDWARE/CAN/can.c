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
	  volatile unsigned int EXT_value_shield = 0x00FFFF00;//Ϊ�˹�����ݮ�ɵ�ԴID��Ŀ��ID
		EXT_value_1 = ((destID<<8)+chipID_hole)<<8;
    GPIO_InitTypeDef 		GPIO_InitStructure;
    CAN_InitTypeDef        	CAN_InitStructure;
    CAN_FilterInitTypeDef  	CAN_FilterInitStructure;
#if CAN_RX0_INT_ENABLE
    NVIC_InitTypeDef  		NVIC_InitStructure;
#endif

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//ʹ��PORTAʱ��



    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);//ʹ��CAN1ʱ��

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);			//��ʼ��IO

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);			//��ʼ��IO

    //CAN��Ԫ����
    CAN_InitStructure.CAN_TTCM=DISABLE;			//��ʱ�䴥��ͨ��ģʽ
    CAN_InitStructure.CAN_ABOM=DISABLE;			//����Զ����߹���
    CAN_InitStructure.CAN_AWUM=DISABLE;			//˯��ģʽͨ���������(���CAN->MCR��SLEEPλ)
    CAN_InitStructure.CAN_NART=ENABLE;			//��ֹ�����Զ�����
    CAN_InitStructure.CAN_RFLM=DISABLE;		 	//���Ĳ�����,�µĸ��Ǿɵ�
    CAN_InitStructure.CAN_TXFP=DISABLE;			//���ȼ��ɱ��ı�ʶ������
    CAN_InitStructure.CAN_Mode= mode;	        //ģʽ���ã� mode:0,��ͨģʽ;1,�ػ�ģʽ;
    //���ò�����
    CAN_InitStructure.CAN_SJW=tsjw;				//����ͬ����Ծ���(Tsjw)Ϊtsjw+1��ʱ�䵥λ  CAN_SJW_1tq	 CAN_SJW_2tq CAN_SJW_3tq CAN_SJW_4tq
    CAN_InitStructure.CAN_BS1=tbs1; 			//Tbs1=tbs1+1��ʱ�䵥λCAN_BS1_1tq ~CAN_BS1_16tq
    CAN_InitStructure.CAN_BS2=tbs2;				//Tbs2=tbs2+1��ʱ�䵥λCAN_BS2_1tq ~	CAN_BS2_8tq
    CAN_InitStructure.CAN_Prescaler=brp;        //��Ƶϵ��(Fdiv)Ϊbrp+1
    CAN_Init(CAN1, &CAN_InitStructure);        	//��ʼ��CAN1

    CAN_FilterInitStructure.CAN_FilterNumber=0;	//������0
    CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask ;//CAN_FilterMode_IdList; 	//����λģʽ
    CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit; 	//32λ��
    CAN_FilterInitStructure.CAN_FilterIdHigh=((EXT_value_1<<3) >>16) &0xffff;//0x0001;//EXT_value>>16;//<<5;	//32λID
    CAN_FilterInitStructure.CAN_FilterIdLow=(u16)(EXT_value_1<<3) | CAN_ID_EXT;//0x0000;//(((EXT_value<<16)>>24)<<8)|0x02;

    CAN_FilterInitStructure.CAN_FilterMaskIdHigh=((EXT_value_shield<<3) >>16) &0xffff;;//EXT_value>>16;//EXT_shield>>16;//0x0000;//32λMASK
    CAN_FilterInitStructure.CAN_FilterMaskIdLow=(u16)(EXT_value_shield<<3) | CAN_ID_EXT;//(((EXT_value<<16)>>24)<<8)|0x02;//(((EXT_shield<<16)>>24)<<8)+0x04;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_Filter_FIFO0;//������0������FIFO0
    CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;//���������0

    CAN_FilterInit(&CAN_FilterInitStructure);			//�˲�����ʼ��

#if CAN_RX0_INT_ENABLE
    CAN_ITConfig(CAN1,CAN_IT_FMP0,ENABLE);				//FIFO0��Ϣ�Һ��ж�����.

    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;     // �����ȼ�Ϊ1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;            // �����ȼ�Ϊ0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif
    return 0;
}
unsigned char data0;
unsigned char flag_light;
unsigned char count;
unsigned char flag_headjudge;
#if CAN_RX0_INT_ENABLE	//ʹ��RX0�ж�
//�жϷ�����
void USB_LP_CAN1_RX0_IRQHandler(void)
{
		struct pica_can_cmd body;
    CanRxMsg rx_msg;
    CAN_Receive(CAN1, 0, &rx_msg);
	
		body.cmd_type = rx_msg.ExtId>>24;
		body.hole = ((unsigned char)(rx_msg.ExtId))>>5;
		body.frame_idx = rx_msg.ExtId & 0x0000001F;	
	  body.data_length = rx_msg.DLC;
    if(RDPtr==WRPtr)                        //ָ�������ζ��buffer�����ˣ��Ѷ�дָ��ָ��buffer��ʼλ��
    {
        WRPtr=recvBuffer;           
        RDPtr=recvBuffer;           
		}
		*WRPtr = body.cmd_type;
		*(WRPtr+1) = body.hole;
		*(WRPtr+2) = body.frame_idx;
		*(WRPtr+3) = body.data_length;
		memcpy(recvBuffer + (WRPtr-recvBuffer+CAN_IDENTIFIER_LEN), rx_msg.Data,rx_msg.DLC);  //��ָ���дָ�붼ָ����һ��
		WRPtr = WRPtr + rx_msg.DLC+CAN_IDENTIFIER_LEN;		

		
//        }	
//    if((RxMessage.Data[0]==chipID_B)||(RxMessage.Data[0]==chipID_A)||(RxMessage.Data[0]==0x00))
//   {
//        if(RDPtr==WRPtr)                        //ÿ�������µ����ݵ�ʱ�������ָ�벻���׵�ַ����ͬʱ����ָ���дָ�����׵�ַ�����ƶ�(RDPtr-recvBuffer>0)
//        {
//            WRPtr=recvBuffer;//+(WRPtr-RDPtr);            //move the pointer backwards
//            RDPtr=recvBuffer;                          //the read pointer is fixed to the begining after the new data arrival
//        }
//        if((RxMessage.Data[1]&0x80) == 0x80 )          // the MSB of the source ID is set to 1 if this frame is the first one of the packet
//        {
//            if(RxMessage.Data[0] == 0)
//            {
//                if(RxMessage.Data[6] > 8)                  //flag_headjudge ����ź�Ŀ���Ǳ��ⶪ֡
//								{ flag_headjudge = 1;}
//								memcpy(recvBuffer + (WRPtr-recvBuffer), RxMessage.Data,RxMessage.DLC);  //��ָ���дָ�붼ָ����һ��
//                WRPtr = WRPtr + RxMessage.DLC;
//            }
//						else if ((RxMessage.Data[3]==2) && (RxMessage.Data[0]==(chipID_A)))  //������
//						{
//							if(RxMessage.Data[4])    //����
//              {

//             memcpy(recvBuffer + (WRPtr-recvBuffer), RxMessage.Data,RxMessage.DLC);  //��ָ���дָ�붼ָ����һ��
//             WRPtr = WRPtr + RxMessage.DLC;	
//						
//              }
//              else
//              {   
//						  flag_pollingA=0;
//						 memcpy(recvBuffer + (WRPtr-recvBuffer), RxMessage.Data,RxMessage.DLC);  //��ָ���дָ�붼ָ����һ��
//             WRPtr = WRPtr + RxMessage.DLC;			
//              }
//						}
//						else if ((RxMessage.Data[3]==2) && (RxMessage.Data[0]==chipID_B))  //������
//						{
//							if(RxMessage.Data[4])    //����
//              {
//						  memcpy(recvBuffer + (WRPtr-recvBuffer), RxMessage.Data,RxMessage.DLC);  //��ָ���дָ�붼ָ����һ��
//              WRPtr = WRPtr + RxMessage.DLC;
//												
//              }
//              else
//              {
//						  flag_pollingB=0;
//							memcpy(recvBuffer + (WRPtr-recvBuffer), RxMessage.Data,RxMessage.DLC);  //��ָ���дָ�붼ָ����һ��
//              WRPtr = WRPtr + RxMessage.DLC;					
//              }
//						}

//						else
//            {							
//             memcpy(recvBuffer + (WRPtr-recvBuffer), RxMessage.Data,RxMessage.DLC);  //��ָ���дָ�붼ָ����һ��
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
//                memcpy(recvBuffer + (WRPtr-recvBuffer), &RxMessage.Data[2],RxMessage.DLC-2);// �ǵ�һ֡����
//                WRPtr = WRPtr + RxMessage.DLC-2;
//											
//            }
//        }
//    }

}


#endif

//can����һ������(�̶���ʽ:IDΪ0X12,��׼֡,����֡)
//len:���ݳ���(���Ϊ8)
//msg:����ָ��,���Ϊ8���ֽ�.
//����ֵ:0,�ɹ�;
//		 ����,ʧ��;
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
    TxMessage.ExtId=EXT_value;//0x00;			// ������չ��ʾ��
    TxMessage.IDE=CAN_ID_EXT;//CAN_Id_Standard; 	// ��׼֡
    TxMessage.RTR=CAN_RTR_Data;		// ����֡
    TxMessage.DLC=len;				// Ҫ���͵����ݳ���
    for(i=0; i<len; i++)
        TxMessage.Data[i]=msg[i];
    mbox= CAN_Transmit(CAN1, &TxMessage);

	
    i=0;
    while((CAN_TransmitStatus(CAN1, mbox)==CAN_TxStatus_Failed)&&(i<0XFFF))i++;	//�ȴ����ͽ���
    if(i>=0XFFF)
        return 1;
    return 0;
}
//can�ڽ������ݲ�ѯ
//buf:���ݻ�����;
//����ֵ:0,�����ݱ��յ�;
//		 ����,���յ����ݳ���;
u8 Can_Receive_Msg(u8 *buf)
{
    u32 i;
    CanRxMsg RxMessage;
    if( CAN_MessagePending(CAN1,CAN_FIFO0)==0)
        return 0;		//û�н��յ�����,ֱ���˳�
    CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);//��ȡ����
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










