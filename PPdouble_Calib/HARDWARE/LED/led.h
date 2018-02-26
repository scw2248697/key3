#ifndef __LED_H
#define __LED_H	 
#include "sys.h"

#define LEDR_B PAout(15)
#define LEDG_B PBout(4)
#define LEDB_B PBout(3)

#define LEDR_A PBout(7)
#define LEDG_A PBout(5)
#define LEDB_A PBout(6)


extern 	unsigned char Led_Red[3];
extern	unsigned char Led_Green[3];
extern	unsigned char	Led_Blue[3];
extern	unsigned char	Led_Magenta[3];
extern	unsigned char	Led_Cyan[3];
extern	unsigned char Led_Yellow[3];
extern	unsigned char Led_White[3];
void LED_Init(void);//≥ı ºªØ
void LED_Control(unsigned char frame_idx,unsigned char *data);//øÿ÷∆µ∆
#endif
