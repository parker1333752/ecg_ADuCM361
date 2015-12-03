#ifndef __THREAD_ACC__
#define __THREAD_ACC__

#include <cmsis_os.h>

extern osThreadId tid_Thread_acc;

void Thread_acc(const void*);
uint8_t Spix_read_reg(uint8_t);
void Spix_write_reg(uint8_t, uint8_t);

#endif
