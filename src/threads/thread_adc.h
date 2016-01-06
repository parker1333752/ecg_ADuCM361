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
	int32_t adc0;
	int32_t date;
	enum AdcDataType type;
}AdcValueDef;
#pragma pack(push, 1)
typedef struct{
	int32_t ecg_data;
	int32_t hs_data;
	int16_t Accelerometer_X; /*!< Accelerometer value X axis */
	int16_t Accelerometer_Y; /*!< Accelerometer value Y axis */
	int16_t Accelerometer_Z; /*!< Accelerometer value Z axis */
	int16_t Gyroscope_X;     /*!< Gyroscope value X axis */
	int16_t Gyroscope_Y;     /*!< Gyroscope value Y axis */
	int16_t Gyroscope_Z;     /*!< Gyroscope value Z axis */
	int16_t date;
}EcgDataDef;
#pragma pack(pop)

extern osThreadId tid_Thread_adc;

void Thread_adc(const void*);

#endif
