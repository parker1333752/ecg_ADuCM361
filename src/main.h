#ifndef __MAIN_H__
#define __MAIN_H__

#include "cmsis_os.h"	
#include "ADUCM360.h"
#include "lib.h"
#include "stdio.h"
#include "retarget.h"

#include "thread_uart.h"
#include "thread_adc.h"
#include "thread_led.h"
#include "thread_acc.h"

typedef struct {
	osThreadDef_t threadInfo;
	osThreadId *tid;
	void* argument;
}ThreadDeclare;

#endif
