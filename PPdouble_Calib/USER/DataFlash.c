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
#include "DataFlash.h"
#include "delay.h"
void FLASH_WriteByte(uint32_t addr , uint8_t *p , uint16_t Byte_Num)
{
          uint32_t HalfWord;
          Byte_Num = Byte_Num/2;
          FLASH_Unlock();
					delay_ms(10);
          FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
					delay_ms(10);
          FLASH_ErasePage(addr);
					delay_ms(10);
          while(Byte_Num --)
          {
                  HalfWord=*(p++);
                  HalfWord|=*(p++)<<8;
                  FLASH_ProgramHalfWord(addr, HalfWord);
                  addr += 2;
          }
          FLASH_Lock();
}
void FLASH_ReadByte(uint32_t addr , uint8_t *p , uint16_t Byte_Num)
{
    while(Byte_Num--)
    {
     *(p++)=*((uint8_t*)addr++);
    }
		delay_ms(100);
}
void FLASH_Write_Integrate(uint32_t addr,uint8_t *array)
{
	FLASH_SetLatency(FLASH_Latency_2); 
//	FLASH_Unlock();	
//	FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
//	FLASH_ErasePage(addr); 
	FLASH_WriteByte(addr,array,16);  
	delay_ms(1000);
}
