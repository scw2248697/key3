#ifndef __LOCK_H
#define __LOCK_H	 
#include "sys.h"
#define LOCK1P PBout(8)
#define LOCK1N PBout(9)
#define LOCK2P PBout(10)
#define LOCK2N PBout(11)	   

void LOCK_Init(void);	//≥ı ºªØ
void Lock_Control(unsigned char frame_idx,unsigned char *data);

extern unsigned char LOCK_OFF;
extern unsigned char LOCK_ON;
#endif

