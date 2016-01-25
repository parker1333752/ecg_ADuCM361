#ifndef __TIMER_H__
#define __TIMER_H__

#include "ADUCM360.h"
#include "GptLib.h"

void Timer0_init(int);

void Timer1_init(int);
int32_t getCurrentCount_Timer1(void);
void resetCurrentCount_Timer1(void);


#endif
