#ifndef __THREAD_I2C__
#define __THREAD_I2C__

#include <cmsis_os.h>

extern osThreadId tid_Thread_i2c;

void Thread_i2c(const void*);

#endif
