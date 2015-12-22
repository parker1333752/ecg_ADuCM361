#include "thread_led.h"
#include <utils.h>
#include "led.h"
#include <math.h>

osThreadId tid_Thread_led;

// for led test
void Thread_led (void const *argument) {
	int a = 0;
	uint32_t t = 0;
	int temp = 0;
//	const uint32_t period = osKernelSysTickFrequency*2;
//	const uint32_t tick = period / 200;
	
	LED_init();
	
	while (1) {
		a += 1;
		if(a>=200){
			a=0;
		}
		if(a<200){
			temp = a;
		}else{
			temp = 0;
		}
		t = floor((cos200[temp]+1)*(cos200[temp]+1)*14/4+0.5);
		LED_off(LED1);
		osDelay(15-t);
		LED_on(LED1);
		osDelay(t);
		/*
		t = osKernelSysTick();
		t %= (period);
		t /= osKernelSysTickFrequency;
		if(t ==  0)LED_off(LED1);
		else LED_on(LED1);
		osThreadYield();
		*/
	}
}
