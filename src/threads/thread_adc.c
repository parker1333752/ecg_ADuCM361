#include "thread_adc.h"
#include "thread_uart.h"
#include "adc.h"
#include <stdio.h>

osThreadId tid_Thread_adc;
osTimerId timerid_adc;

#define TECG_SIGNAL_TIMER  0x02
#define TIMER_FREQUENCY 	200		// Hz

#define SIG_ADC1_COMPLETE  0x01

#define MAX_ECG_ADC_VALUE 0x10000000
#define MAX_HS_ADC_VALUE 0x10000000

#pragma pack(push, 1)
typedef struct{
	int32_t date;
	double ecg_data;
	double hs_data;
	MPUDataDef mpudata;
}EcgDataDef;
#pragma pack(pop)

// sampling ECG signal
void Thread_adc (void const *argument)
{
	EcgDataDef ecg_data;
	int32_t utime;
	int32_t ms_tickcount;
	int32_t utime_start;
	#ifdef GET_MPU_DATA
		int32_t i;
		char *ptmp;
	#endif
	
	#ifdef GET_MPU_DATA
		MPUDataDef *cur_mpu = NULL;
		MPUDataDef *old_mpu = _PoolGetTop(P_MPUData);
	#endif
	
	ECG_init();
	Timer0_init(TIMER_FREQUENCY);

	// use to calculate ms from systicks.
	ms_tickcount = osKernelSysTickFrequency/1000;
	
	// wait for uart initialize complete.
	while(flagUartInitComplete == 0)osThreadYield();

	utime_start = osKernelSysTick();
	while(1) {
		//TODO : note that using signal here may cause Signal Lost.
		osSignalWait(TECG_SIGNAL_TIMER, osWaitForever);
		utime = osKernelSysTick() - utime_start;
		utime = (utime + (ms_tickcount>>1)) / ms_tickcount;
		
		// set timestamp
		ecg_data.date = utime;
		
		#ifdef GET_ECG_DATA
			ECG_start_sample();
			osSignalWait(SIG_ADC1_COMPLETE, osWaitForever);
			ecg_data.ecg_data = AdcValue;
			ecg_data.ecg_data /= MAX_ECG_ADC_VALUE;
		#endif

		#ifdef GET_HS_DATA
			HS_start_sample();
			osSignalWait(SIG_ADC1_COMPLETE, osWaitForever);
			ecg_data.hs_data = AdcValue;
			ecg_data.hs_data /= MAX_HS_ADC_VALUE;
		#endif

		#ifdef GET_MPU_DATA
			cur_mpu = _PoolGetTop(P_MPUData);
			if(cur_mpu != old_mpu){
				// receive new data;
				ptmp = (char*)&(ecg_data.mpudata);
				for(i=0;i<sizeof(MPUDataDef);++i)*(ptmp+i) = *((char*)(cur_mpu) + i);
			} else {
				// no new data;
				ptmp = (char*)&(ecg_data.mpudata);
				for(i=0;i<sizeof(MPUDataDef);++i)*(ptmp+i) = 0;
			}
			old_mpu =cur_mpu;
		#endif

		// Send as packed data
		UART_Write_Frame(0x01, sizeof(EcgDataDef), &ecg_data);
	}
}

void ADC1_Int_Handler(void)
{
	volatile int f_ADCSTA = 0;
	f_ADCSTA = AdcSta(pADI_ADC1);	// Read ADC status register to clear
	if(f_ADCSTA & DETSTA_STEPDATRDY){
		AdcValue = AdcRd(pADI_ADC1);            // Read ADC result register
		osSignalSet(tid_Thread_adc, SIG_ADC1_COMPLETE);
	}
}

void GP_Tmr0_Int_Handler(void)
{
	volatile int flag = GptSta(pADI_TM0);
	if(flag & TSTA_TMOUT){
		GptClrInt(pADI_TM0, TCLRI_TMOUT);
		osSignalSet(tid_Thread_adc , TECG_SIGNAL_TIMER);
	}else if(flag & TSTA_CAP){
		GptClrInt(pADI_TM0, TCLRI_CAP);
	}
}
