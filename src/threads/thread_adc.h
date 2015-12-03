#ifndef __THREAD_ADC__
#define __THREAD_ADC__

#include <cmsis_os.h>
#include "timer.h"

//#define GET_ECG_DATA
#define GET_MPU_DATA
#define GET_HS_DATA

extern osThreadId tid_Thread_adc;
extern osTimerId timerid_adc;

void Thread_adc(const void*);
void adc_timer_callback(void const *);

#endif
