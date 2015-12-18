#ifndef __THREAD_ADC__
#define __THREAD_ADC__

#include <cmsis_os.h>
#include "timer.h"

//#define GET_ECG_CONTINUOUS
#define GET_HS_CONTINUOUS

#if !defined(GET_ECG_CONTINUOUS) && !defined(GET_HS_CONTINUOUS)
#define GET_ECG_DATA
#define GET_HS_DATA
#endif

/* store data capture in adc interrupt; */
enum AdcDataType{ 
	ecg = 1,
	hs,
};
typedef struct{
	int32_t adc;
	int32_t date;
	enum AdcDataType type;
}AdcValueDef;

extern osThreadId tid_Thread_adc;
extern osTimerId timerid_adc;

void Thread_adc(const void*);
void adc_timer_callback(void const *);

#endif
