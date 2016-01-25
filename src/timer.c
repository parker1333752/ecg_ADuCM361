#include "timer.h"

void Timer0_init(int frequency)
{
	// Following line will set reload value as 160, so timer frequency is 4000000/20000 = 200Hz.
	GptLd(pADI_TM0, 4000000/frequency); 
	// Choosing LFOSC as clock source, so source frequency is 32kHz.
	GptCfg(pADI_TM0, TCON_CLK_PCLK, TCON_PRE_DIV1, TCON_MOD_PERIODIC|TCON_ENABLE);
	NVIC_EnableIRQ(TIMER0_IRQn);
}

int32_t incAndGetTimerCount(int32_t inc, int32_t reset){
	static int32_t timer1_c=0;
	if(reset){
		timer1_c = 0;
	}else{
		if(inc)timer1_c += 1;
		if(timer1_c>(int32_t)0x7fff)timer1_c -= (int32_t)0x7fff;
	}
	return timer1_c;
}

void Timer1_init(int frequency){
	// Following line will set reload value as 400, so timer frequency is 4000000/400 = 10000Hz.
	GptLd(pADI_TM1, 4000000/frequency); 
	// Choosing LFOSC as clock source, so source frequency is 32kHz.
	GptCfg(pADI_TM1, TCON_CLK_PCLK, TCON_PRE_DIV1, TCON_MOD_PERIODIC|TCON_ENABLE);
	NVIC_EnableIRQ(TIMER1_IRQn);
}

void GP_Tmr1_Int_Handler(){
	volatile int flag = GptSta(pADI_TM1);
	if(flag & TSTA_TMOUT){
		GptClrInt(pADI_TM1, TCLRI_TMOUT);
		incAndGetTimerCount(1,0);
	}else if(flag & TSTA_CAP){
		GptClrInt(pADI_TM1, TCLRI_CAP);
	}
}
void resetCurrentCount_Timer1(){
	incAndGetTimerCount(0,1);
}
int32_t getCurrentCount_Timer1(){
	return incAndGetTimerCount(0,0);
}
