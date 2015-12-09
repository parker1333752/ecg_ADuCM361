#include "timer.h"

int32_t timer1_count = 0;
void Timer0_init(int frequency)
{
	// Following line will set reload value as 160, so timer frequency is 4000000/20000 = 200Hz.
	GptLd(pADI_TM0, 4000000/frequency); 
	// Choosing LFOSC as clock source, so source frequency is 32kHz.
	GptCfg(pADI_TM0, TCON_CLK_PCLK, TCON_PRE_DIV1, TCON_MOD_PERIODIC|TCON_ENABLE);
	NVIC_EnableIRQ(TIMER0_IRQn);
}

void Timer1_init(int frequency){
	// Following line will set reload value as 400, so timer frequency is 4000000/400 = 10000Hz.
	GptLd(pADI_TM1, 4000000/frequency); 
	// Choosing LFOSC as clock source, so source frequency is 32kHz.
	GptCfg(pADI_TM1, TCON_CLK_PCLK, TCON_PRE_DIV1, TCON_MOD_PERIODIC|TCON_ENABLE);
	NVIC_EnableIRQ(TIMER1_IRQn);
	timer1_count = 0;
}

void GP_Tmr1_Int_Handler(){
	volatile int flag = GptSta(pADI_TM1);
	if(flag & TSTA_TMOUT){
		GptClrInt(pADI_TM1, TCLRI_TMOUT);
		++timer1_count;
	}else if(flag & TSTA_CAP){
		GptClrInt(pADI_TM1, TCLRI_CAP);
	}
}

int32_t getCurrentCount_Timer1(){
	return timer1_count;
}
