#ifndef __THREAD_UART__
#define __THREAD_UART__

#include <cmsis_os.h>

//#define GET_MPU_DATA

extern osThreadId tid_Thread_uart;

void Thread_uart(const void*);
void UART_Write_Frame(uint8_t, uint16_t, void*);

#endif
