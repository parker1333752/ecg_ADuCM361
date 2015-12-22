#ifndef __THREAD_ADC__
#define __THREAD_ADC__

#include <cmsis_os.h>

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
#pragma pack(push, 1)
typedef struct{
	int32_t date;
	int32_t ecg_data;
	int32_t hs_data;
}EcgDataDef;
#pragma pack(pop)

extern osThreadId tid_Thread_adc;
extern EcgDataDef ecg_data;

void Thread_adc(const void*);

#endif
