/*
 * --------------------
 * Company							: Tianjin techvan
 * --------------------
 * Project Name					: AGV
 * Description					: C file
 * --------------------
 * Tool Versions				: uVision V5.17.0.0
 * Target Device				: STM32F407IGT6
 * --------------------
 * Engineer							: wangyulong
 * Revision							: 1.0
 * Created Date					: 2016.01.03
 * --------------------
 * Engineer							:
 * Revision							:
 * Modified Date				:
 * --------------------
 * Additional Comments	:
 *
 * --------------------
 */
#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"


void TIM3_Int_Init(u16 arr,u16 psc);

typedef void (* timerfun)(void);
extern timerfun timerfun1,timerfun2;

#endif
