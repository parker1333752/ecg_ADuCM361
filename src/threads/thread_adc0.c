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

#define Q_ADC0VALUE_SIZE 20
osMessageQDef (Q_ADC0VALUE, Q_ADC0VALUE_SIZE, uint32_t);
osMessageQId Q_ADC0VALUE;
_PoolDef(P_ADC0VALUE, Q_ADC0VALUE_SIZE, AdcValueDef);

static enum AdcDataType current_adc_type;

extern EcgDataDef ecg_frame;
// sampling ECG signal
void Thread_adc0 (void const *argument)
{

	AdcValueDef *pdata;
	osEvent os_result;

	// Create message queue.
	Q_ADC0VALUE = osMessageCreate(osMessageQ(Q_ADC0VALUE), NULL);

	ADC0_init();
	#if defined(GET_ECG_CONTINUOUS)
		ECG_start_continuous();
		current_adc_type = ecg;
	#elif defined(GET_HS_CONTINUOUS)
		HS_start_continuous();
		current_adc_type = hs;
	#endif

	while(1) {
		os_result = osMessageGet(Q_ADC0VALUE, osWaitForever);
		if(os_result.status == osEventMessage){
			pdata = (AdcValueDef*)os_result.value.v;
			if(pdata->type == ecg){
				//ecg_frame.ecg_data = ECG_ADC_TO_VOLTAGE * pdata->adc / MAX_ECG_ADC_VALUE;
				ecg_frame.ecg_data = pdata->adc;
			} else if (pdata->type == hs){
				//ecg_frame.hs_data = HS_ADC_TO_VOLTAGE * pdata->adc / MAX_HS_ADC_VALUE;
				ecg_frame.hs_data = pdata->adc;
			}
		}
	}
}

void ADC0_Int_Handler(void)
{
	volatile int f_ADCSTA = 0;
	AdcValueDef *pdata;
	int32_t data;
	f_ADCSTA = AdcSta(pADI_ADC0);	// Read ADC status register to clear
	if(f_ADCSTA & DETSTA_STEPDATRDY){
		data = AdcRd(pADI_ADC0);            // Read ADC result register
		pdata = _PoolAlloc(P_ADC0VALUE);
		pdata->adc = data;
		pdata->date = getCurrentCount_Timer1();
		pdata->type = current_adc_type;
		osMessagePut(Q_ADC0VALUE, (uint32_t)pdata, 0);
	}
}
