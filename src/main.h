#ifndef __MAIN_H__
#define __MAIN_H__

#include "cmsis_os.h"	
#include "ADUCM360.h"
#include "lib.h"
#include "stdio.h"
#include "retarget.h"

typedef struct {
	osThreadDef_t threadInfo;
	osThreadId *tid;
	void* argument;
}ThreadDeclare;

#endif
