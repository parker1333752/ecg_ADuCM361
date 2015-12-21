#include "thread_adc.h"
#include "thread_uart.h"
#include "adc.h"
#include <stdio.h>
#include "pools.h"
#include "AdcLib.h"

//#define GET_ECG_CONTINUOUS
#define GET_HS_CONTINUOUS

#if !defined(GET_ECG_CONTINUOUS) && !defined(GET_HS_CONTINUOUS)
#define GET_ECG_DATA
#define GET_HS_DATA
#endif

osThreadId tid_Thread_adc;
//osTimerId timerid_adc;

//#define TECG_SIGNAL_TIMER  0x02
//#if defined(GET_ECG_DATA) || defined(GET_HS_DATA)
//	#define TIMER_FREQUENCY 	200		// Hz
//#endif

//#define MAX_ECG_ADC_VALUE 0x10000000
//#define ECG_ADC_TO_VOLTAGE 1.2
//#define MAX_HS_ADC_VALUE 0x10000000
//#define HS_ADC_TO_VOLTAGE 2.8

#define Q_ADCVALUE_SIZE 30
osMessageQDef (Q_ADCVALUE, Q_ADCVALUE_SIZE, uint32_t);
osMessageQId Q_ADCVALUE;
_PoolDef(P_ADCVALUE, Q_ADCVALUE_SIZE, AdcValueDef);

static const enum AdcDataType current_adc_type = hs;

EcgDataDef ecg_data;
// sampling ECG signal
void Thread_adc (void const *argument)
{
#if !defined(GET_ECG_CONTINUOUS) && !defined(GET_HS_CONTINUOUS) \
	&& !defined(GET_ECG_DATA) && !defined(GET_HS_DATA)
#warning "Thread adc didn't work, cause by undefined one of \
(GET_ECG_CONTINUOUS, GET_HS_CONTINUOUS, GET_ECG_DATA, GET_HS_DATA)
#endif
	
	int32_t tickcount_start;
	AdcValueDef *pdata;
	osEvent os_result;

	ADC1_init();
	ecg_data.ecg_data = 0;
	ecg_data.hs_data = 0;
	
	// Create message queue.
	Q_ADCVALUE = osMessageCreate(osMessageQ(Q_ADCVALUE), NULL);

//	#if defined(GET_ECG_CONTINUOUS)
//		ECG_start_continuous();
//		current_adc_type = ecg;
//	#elif defined(GET_HS_CONTINUOUS)
//		HS_start_continuous();
//		current_adc_type = hs;
//	#elif defined(GET_ECG_DATA) || defined(GET_HS_DATA)
//		Timer0_init(TIMER_FREQUENCY);
//	#endif

	// store starting tick count.
	tickcount_start = getCurrentCount_Timer1();
	while(1) {
		os_result = osMessageGet(Q_ADCVALUE, osWaitForever);
		if(os_result.status == osEventMessage){
			pdata = (AdcValueDef*)os_result.value.v;
			ecg_data.date = (pdata->date - tickcount_start);
			if(pdata->type == ecg){
				//ecg_data.ecg_data = ECG_ADC_TO_VOLTAGE * pdata->adc / MAX_ECG_ADC_VALUE;
				ecg_data.ecg_data = pdata->adc;
			} else if (pdata->type == hs){
				//ecg_data.hs_data = HS_ADC_TO_VOLTAGE * pdata->adc / MAX_HS_ADC_VALUE;
				ecg_data.hs_data = pdata->adc;
			}
		}
		#if defined(GET_ECG_CONTINUOUS) || defined(GET_HS_CONTINUOUS)
			UART_Write_Frame(0x01, sizeof(EcgDataDef), &ecg_data);
		#else
			#if defined(GET_ECG_DATA) && !defined(GET_HS_DATA)
				if(pdata->type == ecg){
					UART_Write_Frame(0x01, sizeof(EcgDataDef), &ecg_data);
				}
			#elif defined(GET_HS_DATA)
				if(pdata->type == hs){
					UART_Write_Frame(0x01, sizeof(EcgDataDef), &ecg_data);
				}
			#endif
		#endif
	}
}

void ADC1_Int_Handler(void)
{
	volatile int f_ADCSTA = 0;
	AdcValueDef *pdata;
	int32_t data;
	f_ADCSTA = AdcSta(pADI_ADC1);	// Read ADC status register to clear
	if(f_ADCSTA & DETSTA_STEPDATRDY){
		data = AdcRd(pADI_ADC1);            // Read ADC result register
		pdata = _PoolAlloc(P_ADCVALUE);
		pdata->adc = data;
		pdata->date = getCurrentCount_Timer1();
		pdata->type = current_adc_type;
		osMessagePut(Q_ADCVALUE, (uint32_t)pdata, 0);
	}
}

//// Be called when timer update occur.
//void TimerOnInterruptHandler(){
//	static int state = -1;
//	#if defined(GET_ECG_DATA) && defined(GET_HS_DATA)
//		++state;
//		if(state>1) { state = 0; }
//	#elif defined(GET_ECG_DATA)
//		state = 0;
//	#elif defined(GET_HS_DATA)
//		state = 1;
//	#else
//		state = 0xff;
//	#endif
//	switch(state){
//		case 0:
//			ECG_start_sample();
//			current_adc_type = ecg;
//			break;
//		case 1:
//			HS_start_sample();
//			current_adc_type = hs;
//			break;
//		default:
//			break;
//	}
//}

//void GP_Tmr0_Int_Handler(void)
//{
//	volatile int flag = GptSta(pADI_TM0);
//	if(flag & TSTA_TMOUT){
//		GptClrInt(pADI_TM0, TCLRI_TMOUT);
//		TimerOnInterruptHandler();
//	}else if(flag & TSTA_CAP){
//		GptClrInt(pADI_TM0, TCLRI_CAP);
//	}
//}
