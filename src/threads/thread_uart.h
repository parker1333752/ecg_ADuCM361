#ifndef __THREAD_UART__
#define __THREAD_UART__

#include <cmsis_os.h>
#include "pools.h"
#include "timer.h"

//#define GET_MPU_DATA

extern osThreadId tid_Thread_uart;

#pragma pack(push, 1)
typedef struct
{
    int32_t date;
    int16_t accx;   // accelerated speed.
    int16_t accy;
    int16_t accz;
    int16_t omegax; // angular velocity.
    int16_t omegay;
    int16_t omegaz;
    int16_t anglex; // angle.
    int16_t angley;
    int16_t anglez;
}MPUDataDef;
#pragma pack(pop)

void Thread_uart(const void*);
void UART_Write_Frame(uint8_t, uint16_t, void*);

#endif
