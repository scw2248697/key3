#ifndef __FLASH_H
#define __FLASH_H
#include "stm32f10x.h"
u8 Write_Flash(u32 *buff, u8 len);
void Read_Flash(u32 *buff, u8 len);
void Char_To_Int(unsigned char *buffer, unsigned int *data,unsigned int position);
void Int_To_Char(unsigned char * buffer, unsigned int data,unsigned int position);
void FLASH_WriteByte(uint32_t addr , uint8_t *p , uint16_t Byte_Num);
void FLASH_ReadByte(uint32_t addr , uint8_t *p , uint16_t Byte_Num);
#endif
