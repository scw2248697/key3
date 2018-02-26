#ifndef __FLASH_H
#define __FLASH_H
#include "stm32f10x.h"
//#define ADDR_Check_B		0x8007040
//#define ADDR_Check_A		0x8007080
//#define ADDR_Check_ID		0x8007800
//#define RFID_MATCH_A		0x8007C00
//#define RFID_MATCH_B		0x8008000

#define ADDR_Check_B		0x8007000
#define ADDR_Check_A		0x8007500
#define ADDR_Check_ID		0x8007800
#define RFID_MATCH_A		0x8006200
#define RFID_MATCH_B		0x8006400
//0x8007000
//0x8007500
//0x8007800
//0x8007200
//0x8007400
u8 Write_Flash(u32 *buff, u8 len);
void Read_Flash(u32 *buff, u8 len);
void Char_To_Int(unsigned char *buffer, unsigned int *data,unsigned int position);
void Int_To_Char(unsigned char * buffer, unsigned int data,unsigned int position);
void FLASH_WriteByte(uint32_t addr , uint8_t *p , uint16_t Byte_Num);
void FLASH_ReadByte(uint32_t addr , uint8_t *p , uint16_t Byte_Num);
void FLASH_Write_Integrate(uint32_t addr,uint8_t *array);
#endif

