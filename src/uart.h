#ifndef __UART_H__
#define __UART_H__

#include "ADUCM360.h"
#include "UrtLib.h"
#include "DioLib.h"

// extern volatile uint8_t UartReadData;

void UART_init(void);
void UART_write(uint8_t data);
uint8_t UART_read(void);

#endif
