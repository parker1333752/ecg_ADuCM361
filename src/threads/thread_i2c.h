#ifndef __THREAD_I2C__
#define __THREAD_I2C__

#include <cmsis_os.h>

#pragma pack(push, 1)
typedef struct{
	int16_t Accelerometer_X; /*!< Accelerometer value X axis */
	int16_t Accelerometer_Y; /*!< Accelerometer value Y axis */
	int16_t Accelerometer_Z; /*!< Accelerometer value Z axis */
	int16_t Gyroscope_X;     /*!< Gyroscope value X axis */
	int16_t Gyroscope_Y;     /*!< Gyroscope value Y axis */
	int16_t Gyroscope_Z;     /*!< Gyroscope value Z axis */
	int32_t Quat[4];
	int16_t date;
}MpuDataDef;
#pragma pack(pop)

extern osThreadId tid_Thread_i2c;

void Thread_i2c(const void*);
MpuDataDef* popNewestMPUforExt(void);

#endif
