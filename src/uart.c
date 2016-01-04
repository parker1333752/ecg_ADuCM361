#include <uart.h>

/**
	@description config uart
**/
void UART_init(void)
{
	DioCfgPin(pADI_GP0, PIN6, 1);
	DioCfgPin(pADI_GP0, PIN7, 2);
	
	UrtCfg(pADI_UART,B115200,COMLCR_WLS_8BITS,0);        // setup baud rate for 9600, 8-bits
	
	UrtIntCfg(pADI_UART,COMIEN_ERBFI|COMIEN_ETBEI);
	NVIC_EnableIRQ(UART_IRQn);	// config uart nvic enable
}

uint8_t UART_read(void)
{
	return (uint8_t)UrtRx(pADI_UART);
}
