#include "thread_adc.h"
#include "thread_uart.h"
#include "adc0.h"
#include <stdio.h>
#include "pools.h"
#include "AdcLib.h"
#include "timer.h"

#define GET_ECG_CONTINUOUS
//#define GET_HS_CONTINUOUS

osThreadId tid_Thread_adc0;

//volatile int32_t adc0_data;

// sampling ECG signal
void Thread_adc0 (void const *argument)
{
	/* adc0 initialize */
//	#if defined(GET_ECG_CONTINUOUS)
//		ECG_start_continuous();
//	#elif defined(GET_HS_CONTINUOUS)
//		HS_start_continuous();
//	#endif

//	while(1) {
//		osDelay(2);
//	}
}

//void ADC0_Int_Handler(void)
//{
//	volatile int f_ADCSTA = 0;
//	f_ADCSTA = AdcSta(pADI_ADC0);	// Read ADC status register to clear
//	if(f_ADCSTA & DETSTA_STEPDATRDY){
//		adc0_data = AdcRd(pADI_ADC0);            // Read ADC result register
//	}
//}
