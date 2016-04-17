#include "thread_adc.h"
#include "thread_uart.h"
#include "adc.h"
#include "adc0.h"
#include <stdio.h>
#include "pools.h"
#include "AdcLib.h"
#include "timer.h"
#include "thread_i2c.h"

//#define GET_ECG_CONTINUOUS
#define GET_HS_CONTINUOUS

osThreadId tid_Thread_adc;

#define MAX_ECG_ADC_VALUE 0x10000000
#define ECG_ADC_TO_VOLTAGE 1.2
#define MAX_HS_ADC_VALUE 0x10000000
#define HS_ADC_TO_VOLTAGE 2.8

#define Q_ADCVALUE_SIZE 20
osMessageQDef (Q_ADCVALUE, Q_ADCVALUE_SIZE, uint32_t);
osMessageQId Q_ADCVALUE;
_PoolDef(P_ADCVALUE, Q_ADCVALUE_SIZE, AdcValueDef);

static enum AdcDataType current_adc_type;
int32_t adc0_data;

#define FRAME_BUFFER_SIZE 10
//volatile EcgDataDef ecg_frame = {0,0,0};
int ecg_frame_buffer_count;
EcgDataDef ecg_fram_buffer[FRAME_BUFFER_SIZE] = {{0,0,0}};

// sampling ECG signal
void Thread_adc (void const *argument)
{
	AdcValueDef *pdata;
	osEvent os_result;
	MpuDataDef* mpu_data;
//	uint8_t* p;
//	int i;

	ADC1_init();
	ADC0_init();
	
	// Create message queue.
	Q_ADCVALUE = osMessageCreate(osMessageQ(Q_ADCVALUE), NULL);

	#if defined(GET_ECG_CONTINUOUS)
		ECG_start_continuous();
		current_adc_type = ecg;
	#elif defined(GET_HS_CONTINUOUS)
		HS_start_continuous();
		ECG_start_continuous();
		current_adc_type = hs;
	#endif

	// store starting tick count.
	resetCurrentCount_Timer1();
	ecg_frame_buffer_count = 0;
	while(1) {
		os_result = osMessageGet(Q_ADCVALUE, osWaitForever);
		if(os_result.status == osEventMessage){
			pdata = (AdcValueDef*)os_result.value.v;
			ecg_fram_buffer[ecg_frame_buffer_count].date = pdata->date;
			if(pdata->type == ecg){
				//ecg_frame.ecg_data = ECG_ADC_TO_VOLTAGE * pdata->adc / MAX_ECG_ADC_VALUE;
				ecg_fram_buffer[ecg_frame_buffer_count].ecg_data = pdata->adc;
				ecg_fram_buffer[ecg_frame_buffer_count].hs_data = pdata->adc0;
			} else if (pdata->type == hs){
				//ecg_frame.hs_data = HS_ADC_TO_VOLTAGE * pdata->adc / MAX_HS_ADC_VALUE;
				ecg_fram_buffer[ecg_frame_buffer_count].hs_data = pdata->adc;
				ecg_fram_buffer[ecg_frame_buffer_count].ecg_data = pdata->adc0;
			}
			mpu_data = popNewestMPUforExt();
			ecg_fram_buffer[ecg_frame_buffer_count].Accelerometer_X = mpu_data->Accelerometer_X;
			ecg_fram_buffer[ecg_frame_buffer_count].Accelerometer_Y = mpu_data->Accelerometer_Y;
			ecg_fram_buffer[ecg_frame_buffer_count].Accelerometer_Z = mpu_data->Accelerometer_Z;
			ecg_fram_buffer[ecg_frame_buffer_count].Gyroscope_X = mpu_data->Gyroscope_X;
			ecg_fram_buffer[ecg_frame_buffer_count].Gyroscope_Y = mpu_data->Gyroscope_Y;
			ecg_fram_buffer[ecg_frame_buffer_count].Gyroscope_Z = mpu_data->Gyroscope_Z;
			
			if(ecg_frame_buffer_count == FRAME_BUFFER_SIZE-1){
//				UART_Write_Frame(0x01, sizeof(EcgDataDef) * FRAME_BUFFER_SIZE, (void*)&ecg_fram_buffer);
				ecg_frame_buffer_count = 0;
			}else{
				ecg_frame_buffer_count += 1;
			}
		}
	}
}

void ADC1_Int_Handler(void)
{
	int f_ADCSTA = 0;
	AdcValueDef *pdata;
	int32_t data;
	f_ADCSTA = AdcSta(pADI_ADC1);	// Read ADC status register to clear
	if(f_ADCSTA & DETSTA_STEPDATRDY){
		data = AdcRd(pADI_ADC1);            // Read ADC result register
		pdata = _PoolAlloc(P_ADCVALUE);
		pdata->adc = data;
		pdata->adc0 = adc0_data;
		pdata->date = getCurrentCount_Timer1();
		pdata->type = current_adc_type;
		osMessagePut(Q_ADCVALUE, (uint32_t)pdata, 0);
	}
}

void ADC0_Int_Handler(void)
{
	int f_ADCSTA = 0;
	f_ADCSTA = AdcSta(pADI_ADC0);	// Read ADC status register to clear
	if(f_ADCSTA & DETSTA_STEPDATRDY){
		adc0_data = AdcRd(pADI_ADC0);            // Read ADC result register
		++adc0_data;
	}
}
