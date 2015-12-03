#ifndef __THREAD_LED__
#define __THREAD_LED__

#include "cmsis_os.h"

extern osThreadId tid_Thread_led;
void Thread_led(const void*);

#endif
