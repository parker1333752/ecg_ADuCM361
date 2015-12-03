#include "timer.h"

void Timer0_init(int frequency)
{
	// set reload value as 160, so timer frequency is 32k/160 = 200Hz.
	GptLd(pADI_TM0, 32000/frequency); 
	// Choosing LFOSC as clock source, so source frequency is 32kHz.
	GptCfg(pADI_TM0, TCON_CLK_LFOSC, TCON_PRE_DIV1, TCON_MOD_PERIODIC|TCON_ENABLE);
	NVIC_EnableIRQ(TIMER0_IRQn);
}
