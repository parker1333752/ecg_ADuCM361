#include <led.h>

LED_TypeDef LED_table[] = {
	{pADI_GP0,PIN4,0},
//	{pADI_GP0,PIN7,0},
};

/**
	@description LED initialization
**/
void LED_init(void){
	int len = sizeof(LED_table)/sizeof(LED_TypeDef);
	int i=0;
	
	/* bug fixed for short circuit between led1 and led2 */
	DioCfgPin(pADI_GP0,PIN1,0);
	DioOenPin(pADI_GP0,PIN1,0);
	
	for(i=0;i<len;i++){
		// set configuration to GPIO mode
		DioCfgPin(LED_table[i].port,LED_table[i].pin,0);
		// set to open drain mode
		DioOcePin(LED_table[i].port,LED_table[i].pin,1);
		// set to output mode
		DioOenPin(LED_table[i].port,LED_table[i].pin,1);
	}
}

/**
	@description LED reset
**/
void LED_reset(void){
	int len = sizeof(LED_table)/sizeof(LED_TypeDef);
	int i=0;
	for(i=0;i<len;i++){
		// reset to defaut mode (input mode)
		DioOcePin(LED_table[i].port,LED_table[i].pin,0);
		DioOenPin(LED_table[i].port,LED_table[i].pin,0);
	}
}

/**
	@description turn on LED
	@param LED
		- LED1
		- LED2
		select which led to turn on
**/
void LED_on(uint8_t LEDx){
	if(LED_table[LEDx].on == 1){
		DioSet(LED_table[LEDx].port,1<<(LED_table[LEDx].pin));
	}else{
		DioClr(LED_table[LEDx].port,1<<(LED_table[LEDx].pin));
	}
}

/**
	@description turn off LED
	@param LED
		- LED1
		- LED2
		select which led to turn off
**/
void LED_off(uint8_t LEDx){
	if(LED_table[LEDx].on == 0){
		DioSet(LED_table[LEDx].port,1<<(LED_table[LEDx].pin));
	}else{
		DioClr(LED_table[LEDx].port,1<<(LED_table[LEDx].pin));
	}
}

/**
	@description toggle LED
	@param LED
		- LED1
		- LED2
**/
void LED_toggle(uint8_t LEDx){
	DioTgl(LED_table[LEDx].port,1<<(LED_table[LEDx].pin));
}
