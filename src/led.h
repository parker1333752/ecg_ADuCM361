#ifndef __LED_H__
#define __LED_H__

#include "ADUCM360.h"
#include "DioLib.h"

typedef struct{
	ADI_GPIO_TypeDef *port;
	uint8_t pin;
	int8_t on; // define the GPIO value that let led on.
}LED_TypeDef;

#define LED1 0
//#define LED2 1

void LED_init(void);
void LED_reset(void);
void LED_on(uint8_t LEDx);
void LED_off(uint8_t LEDx);

#endif
