#include <main.h>
#include <timer.h>

#include "thread_uart.h"
#include "thread_adc.h"
#include "thread_adc0.h"
#include "thread_led.h"
#include "thread_i2c.h"

/**
 * ThreadDeclare.
 * A list store the threads that need to run.
 */
const ThreadDeclare thread_list[] = {
	{{Thread_adc, osPriorityNormal, 1, 0}, &tid_Thread_adc, NULL},
	{{Thread_uart, osPriorityNormal, 1, 0}, &tid_Thread_uart, NULL},
	{{Thread_led, osPriorityNormal, 1, 0}, &tid_Thread_led, NULL},
	{{Thread_i2c, osPriorityNormal, 1, 0}, &tid_Thread_i2c, NULL},
};

int main (void) {
	int i, list_len;
	
	// initialize CMSIS-RTOS 
    osKernelInitialize ();
	
    // Enable all clocks. 
	ClkCfg(CLK_CD0, CLK_HF, CLKSYSDIV_DIV2EN_DIS, CLK_UCLKCG);
	ClkDis(0);
	
	// Addictional peripherals initialization.
	Timer1_init(10000);
	
	// Create threads.
	list_len = sizeof(thread_list) / sizeof(ThreadDeclare);
	for(i=0;i<list_len;++i)
	{
		*thread_list[i].tid = osThreadCreate(&thread_list[i].threadInfo, thread_list[i].argument);
	}
	
	// start thread execution 
	osKernelStart ();
}
