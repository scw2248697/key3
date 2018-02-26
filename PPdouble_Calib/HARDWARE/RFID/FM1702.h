
#ifndef _MAIN_INCLUDED_
#define _MAIN_INCLUDED_

/*常量定义*/
#define FALSE	0
#define TRUE	1

#define OSC_FREQ                11059200L

#define BAUD_115200             256 - (OSC_FREQ/192L)/115200L   // 255
#define BAUD_57600              256 - (OSC_FREQ/192L)/57600L    // 254
#define BAUD_38400              256 - (OSC_FREQ/192L)/38400L    // 253
#define BAUD_28800              256 - (OSC_FREQ/192L)/28800L    // 252
#define BAUD_19200              256 - (OSC_FREQ/192L)/19200L    // 250
#define BAUD_14400              256 - (OSC_FREQ/192L)/14400L    // 248
#define BAUD_9600               256 - (OSC_FREQ/192L)/9600L     // 244
#define RCAP2_50us              65536L - OSC_FREQ/240417L
#define RCAP2_1ms               65536L - OSC_FREQ/12021L
#define RCAP2_10ms              65536L - OSC_FREQ/1200L
#define RCAP2_1s                65536L - OSC_FREQ/12L

#define CALL_isr_UART()         TI = 1

#define mifare1			1
#define mifarepro		2
#define mifarelight		3
#define unkowncard		4
#define unknowncard     4

/* FM1702命令码 */
#define Transceive	0x1E			/* 发送接收命令 */
#define Transmit	0x1a			/* 发送命令 */
#define ReadE2		0x03			/* 读FM1702 EEPROM命令 */
#define WriteE2		0x01			/* 写FM1702 EEPROM命令 */
#define Authent1	0x0c			/* 验证命令认证过程第1步 */
#define Authent2	0x14			/* 验证命令认证过程第2步 */
#define LoadKeyE2	0x0b			/* 将密钥从EEPROM复制到KEY缓存 */
#define LoadKey		0x19			/* 将密钥从FIFO缓存复制到KEY缓存 */
//#define RF_TimeOut	0xfff			/* 发送命令延时时间 */
#define RF_TimeOut	0x3f
#define Req		    0x01
#define Sel		    0x02

/* 数据类型定义 */
#define uchar	unsigned char
#define uint	unsigned int

/* 卡片类型定义定义 */
#define TYPEA_MODE	    0			/* TypeA模式 */
#define TYPEB_MODE	    1			/* TypeB模式 */
#define SHANGHAI_MODE	2			/* 上海模式 */
#define TM0_HIGH	    0xf0		/* 定时器0高位,4MS定时 */
#define TM0_LOW		    0x60		/* 定时器0低位 */
#define TIMEOUT		    100			/* 超时计数器4MS×100=0.4秒 */

/* 射频卡通信命令码定义 */
#define RF_CMD_REQUEST_STD	0x26
#define RF_CMD_REQUEST_ALL	0x52
#define RF_CMD_ANTICOL		0x93
#define RF_CMD_SELECT		0x93
#define RF_CMD_AUTH_LA		0x60
#define RF_CMD_AUTH_LB		0x61
#define RF_CMD_READ		    0x30
#define RF_CMD_WRITE		0xa0
#define RF_CMD_INC		    0xc1
#define RF_CMD_DEC		    0xc0
#define RF_CMD_RESTORE		0xc2
#define RF_CMD_TRANSFER		0xb0
#define RF_CMD_HALT		    0x50

/* Status Values */
#define ALL	    0x01
#define KEYB	0x04
#define KEYA	0x00
#define _AB	    0x40
#define CRC_A	1
#define CRC_B	2
#define CRC_OK	0
#define CRC_ERR 1
#define BCC_OK	0
#define BCC_ERR 1

/* 卡类型定义 */
#define MIFARE_8K	    0			/* MIFARE系列8KB卡片 */
#define MIFARE_TOKEN	1			/* MIFARE系列1KB TOKEN卡片 */
#define SHANGHAI_8K	    2			/* 上海标准系列8KB卡片 */
#define SHANGHAI_TOKEN	3			/* 上海标准系列1KB TOKEN卡片 */

/* 函数错误代码定义 */
#define FM1702_OK_2nd		    0		/* 正确 */
#define FM1702_OK		    0		/* 正确 */
#define FM1702_NOTAGERR		1		/* 无卡 */
#define FM1702_CRCERR		2		/* 卡片CRC校验错误 */
#define FM1702_EMPTY		3		/* 数值溢出错误 */
#define FM1702_AUTHERR		4		/* 验证不成功 */
#define FM1702_PARITYERR	5		/* 卡片奇偶校验错误 */
#define FM1702_CODEERR		6		/* 通讯错误(BCC校验错) */
#define FM1702_SERNRERR		8		/* 卡片序列号错误(anti-collision 错误) */
#define FM1702_SELECTERR	9		/* 卡片数据长度字节错误(SELECT错误) */
#define FM1702_NOTAUTHERR	10		/* 卡片没有通过验证 */
#define FM1702_BITCOUNTERR	11		/* 从卡片接收到的位数错误 */
#define FM1702_BYTECOUNTERR	12		/* 从卡片接收到的字节数错误仅读函数有效 */
#define FM1702_RESTERR		13		/* 调用restore函数出错 */
#define FM1702_TRANSERR		14		/* 调用transfer函数出错 */
#define FM1702_WRITEERR		15		/* 调用write函数出错 */
#define FM1702_INCRERR		16		/* 调用increment函数出错 */
#define FM1702_DECRERR		17		/* 调用decrement函数出错 */
#define FM1702_READERR		18		/* 调用read函数出错 */
#define FM1702_LOADKEYERR	19		/* 调用LOADKEY函数出错 */
#define FM1702_FRAMINGERR	20		/* FM1702帧错误 */
#define FM1702_REQERR		21		/* 调用req函数出错 */
#define FM1702_SELERR		22		/* 调用sel函数出错 */
#define FM1702_ANTICOLLERR	23		/* 调用anticoll函数出错 */
#define FM1702_INTIVALERR	24		/* 调用初始化函数出错 */
#define FM1702_READVALERR	25		/* 调用高级读块值函数出错 */
#define FM1702_DESELECTERR	26
#define FM1702_CMD_ERR		42		/* 命令错误 */

#define Page_Sel		0x00	/* 页写寄存器 */
#define Command			0x01	/* 命令寄存器 */
#define FIFO			0x02	/* 64字节FIFO缓冲的输入输出寄存器 */
#define PrimaryStatus	0x03	/* 发射器接收器及FIFO的状态寄存器1 */
#define FIFO_Length		0x04	/* 当前FIFO内字节数寄存器 */
#define SecondaryStatus	0x05	/* 各种状态寄存器2 */
#define InterruptEn		0x06	/* 中断使能/禁止寄存器 */
#define Int_Req			0x07	/* 中断请求标识寄存器 */
#define Control			0x09	/* 控制寄存器 */
#define ErrorFlag		0x0A	/* 错误状态寄存器 */
#define CollPos			0x0B	/* 冲突检测寄存器 */
#define TimerValue		0x0c	/* 定时器当前值 */
#define Bit_Frame		0x0F	/* 位帧调整寄存器 */
#define TxControl		0x11	/* 发送控制寄存器 */
#define CWConductance	0x12	/* 选择发射脚TX1和TX2发射天线的阻抗 */

//#define CWConductance	0x12	/* 选择发射脚TX1和TX2发射天线的阻抗 */

#define ModConductance	0x13	/* 定义输出驱动阻抗 */
#define CoderControl	0x14	/* 定义编码模式和时钟频率 */
#define ModWidth	0x15	/* 定义编码模式和时钟频率 */
#define TypeBFraming	0x17	/* 定义ISO14443B帧格式 */

#define RxControl1	0x19	/* 接受放大增益*/

#define DecoderControl	0x1a	/* 解码控制寄存器 */

#define RxThreshold	0x1c	/* 接接收阈值*/
#define Rxcontrol2		0x1e	/* 解码控制及选择接收源 */
#define RxWait			0x21	/* 选择发射和接收之间的时间间隔 */
#define ChannelRedundancy	0x22	/* RF通道检验模式设置寄存器 */
#define CRCPresetLSB	0x23
#define CRCPresetMSB	0x24
#define MFOUTSelect		0x26	/* mf OUT 选择配置寄存器 */
#define TimerClock		0x2a	/* 定时器周期设置寄存器 */
#define TimerControl	0x2b	/* 定时器控制寄存器 */
#define TimerReload		0x2c	/* 定时器初值寄存器 */
#define TypeSH			0x31	/* 上海标准选择寄存器 */
#define TestDigiSelect	0x3d	/* 测试管脚配置寄存器 */
#endif


//////////////////////////////////////////////////////////////////////
/////////////////////////SPI 管脚定义/////////////////////////////////
/////////////////////////////////////////////////////////////////////
//FM1702
#define MF522_RST_PIN                    GPIO_Pin_3
#define MF522_RST_PORT                   GPIOA
#define MF522_RST_CLK                    RCC_APB2Periph_GPIOA
  
#define MF522_MISO_PIN                   GPIO_Pin_6
#define MF522_MISO_PORT                  GPIOA
#define MF522_MISO_CLK                   RCC_APB2Periph_GPIOA  

#define MF522_MOSI_PIN                   GPIO_Pin_7  
#define MF522_MOSI_PORT                  GPIOA
#define MF522_MOSI_CLK                   RCC_APB2Periph_GPIOA  

#define MF522_SCK_PIN                    GPIO_Pin_5  
#define MF522_SCK_PORT                   GPIOA
#define MF522_SCK_CLK                    RCC_APB2Periph_GPIOA 

#define MF522_NSS_PIN                    GPIO_Pin_4  
#define MF522_NSS_PORT                   GPIOA
#define MF522_NSS_CLK                    RCC_APB2Periph_GPIOA 

#define MF522_NSS_PIN_2nd                GPIO_Pin_2  
#define MF522_NSS_PORT_2nd               GPIOA
#define MF522_NSS_CLK_2nd                RCC_APB2Periph_GPIOA 

#define MF522_RST_PIN_2nd                GPIO_Pin_1  
#define MF522_RST_PORT_2nd               GPIOA
#define MF522_RST_CLK_2nd                RCC_APB2Periph_GPIOA 

//////////////////////////////指示灯1 管脚定义////////////////////////////////
#define LED1_R_PIN                         GPIO_Pin_4  
#define LED1_R_PORT                        GPIOB
#define LED1_R_CLK                         RCC_APB2Periph_GPIOB
#define LED1_G_PIN                         GPIO_Pin_3  
#define LED1_G_PORT                        GPIOB
#define LED1_G_CLK                         RCC_APB2Periph_GPIOB 
#define LED1_B_PIN                         GPIO_Pin_15  
#define LED1_B_PORT                        GPIOA
#define LED1_B_CLK                         RCC_APB2Periph_GPIOA
//////////////////////////////指示灯2 管脚定义////////////////////////////////
#define LED2_R_PIN                         GPIO_Pin_5  
#define LED2_R_PORT                        GPIOB
#define LED2_R_CLK                         RCC_APB2Periph_GPIOB
#define LED2_G_PIN                         GPIO_Pin_6  
#define LED2_G_PORT                        GPIOB
#define LED2_G_CLK                         RCC_APB2Periph_GPIOB 
#define LED2_B_PIN                         GPIO_Pin_7  
#define LED2_B_PORT                        GPIOB
#define LED2_B_CLK                         RCC_APB2Periph_GPIOB

//////////////////////////////////SPI 功能函数///////////////////////////////////////////////
#define RST_H_B                            GPIO_SetBits(MF522_RST_PORT, MF522_RST_PIN)
#define RST_L_B                            GPIO_ResetBits(MF522_RST_PORT, MF522_RST_PIN)

#define RST_H_A                        GPIO_SetBits(MF522_RST_PORT_2nd, MF522_RST_PIN_2nd)
#define RST_L_A                        GPIO_ResetBits(MF522_RST_PORT_2nd, MF522_RST_PIN_2nd)

#define MOSI_H                           GPIO_SetBits(MF522_MOSI_PORT, MF522_MOSI_PIN)
#define MOSI_L                           GPIO_ResetBits(MF522_MOSI_PORT, MF522_MOSI_PIN)
#define SCK_H                            GPIO_SetBits(MF522_SCK_PORT, MF522_SCK_PIN)
#define SCK_L                            GPIO_ResetBits(MF522_SCK_PORT, MF522_SCK_PIN)
#define NSS_H_B                            GPIO_SetBits(MF522_NSS_PORT, MF522_NSS_PIN)
#define NSS_L_B                            GPIO_ResetBits(MF522_NSS_PORT, MF522_NSS_PIN)

#define NSS_H_A                       GPIO_SetBits(MF522_NSS_PORT_2nd, MF522_NSS_PIN_2nd)
#define NSS_L_A                        GPIO_ResetBits(MF522_NSS_PORT_2nd, MF522_NSS_PIN_2nd)

#define READ_MISO                        GPIO_ReadInputDataBit(MF522_MISO_PORT, MF522_MISO_PIN)
#define READ_MISO                        GPIO_ReadInputDataBit(MF522_MISO_PORT, MF522_MISO_PIN)
/////////////////////////////////////////////////////////////////////
//初始状态及校验状态
/////////////////////////////////////////////////////////////////////
#define NO_ID 			0xFF								//没有初始化ID状态
#define CLB_WAIT 		0xF1								//等待校验状态
#define CLB_ING  		0x55								//校验中
#define CLB_ERROR   0x22								//校验中
#define CLB_FINISH  0x11								//校验完成

/////////////////////////////////////////////////////////////////////
//全局变量
/////////////////////////////////////////////////////////////////////
extern unsigned char UID[5];             //卡1 UID
extern unsigned char buffer_B[16];         //卡1 16字节
extern unsigned char buffer_A[16];     //卡2 16字节
extern unsigned char buffer_rfid[16]; 
extern unsigned char DefaultKey[6];      //密码keyA
extern unsigned char RFID_status_1st;
extern unsigned char RFID_status_2nd;
extern unsigned char out_main_A[16];//检测函数的返回值，里面包含了FM1702中RxThreshold的帧.其中out[15]为最终需要的值。out[2]=0xFF为没有检测，out[2]=0x11为检测完毕，out[2]=0x55为正在检测。
extern unsigned char out_main_B[16];//检测函数的返回值，里面包含了FM1702中RxThreshold的帧.其中out[15]为最终需要的值。out[2]=0xFF为没有检测，out[2]=0x11为检测完毕，out[2]=0x55为正在检测。
extern unsigned char rfid_match_A[16];
extern unsigned char rfid_match_B[16];
extern unsigned char chipID[16];
extern unsigned char Return_Fail;
extern unsigned char Return_Suc;
extern unsigned char Borrow_Fail;
extern unsigned char Borrow_Suc;
extern unsigned char flag_return;
extern unsigned char flag_borrow;
/////////////////////////////////////////////////////////////////////
//函数原型
/////////////////////////////////////////////////////////////////////
 void  STM32_FM1702_GPIO_Init(void);                                //SPI相关GPIO初始化 
 void delay(unsigned int dlength);                                  

/***************************tag functions****************************/	

uchar ReadID_FM1702(void);                                         //读FM1702 ID
void  RFID_RFclose(void);                                        //进入powerdown模式
void  RFID_RFopen(void);                                           //退出powerdown模式
void  RFID_Powerdown(void);                                    //进入powerdown模式
void  RFID_Poweron(void);                                       //退出powerdown模式
uchar Init_FM1702(unsigned char hole);                                           //初始化
uchar Request(uchar mode);                                         //寻卡
uchar AntiColl(void);                                              //冲突检测
uchar Select_Card(void);                                           //选卡
uchar Load_keyE2_CPY(uchar *uncoded_keys);                         //加载密码
uchar Authentication(uchar *UID, uchar SecNR, uchar mode);         //验证密码
uchar MIF_READ(uchar *buff, uchar Block_Adr);                      //读卡
uchar MIF_Write(uchar *buff, uchar Block_Adr);                     //写卡 
 
uchar Read_tag(unsigned char hole);                                              //卡2 寻卡 -- 读卡
uchar Write_tag(unsigned char hole,unsigned char *buffer_temp_write);
uchar Get_tag_A(void);
uchar Read_status(unsigned char hole);
uchar calibration(unsigned char hole);
void Chip_select(unsigned char hole);
void Stm32_restart(void);
///////////////////////////////////////////////////////////////////////////////////////
#define  Array_Len(array)   (sizeof(array) / sizeof(array[0]))



