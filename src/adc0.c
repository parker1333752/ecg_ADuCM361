#include <adc0.h>
#include "DioLib.h"
#include "AdcLib.h"

// Hardware pin map define
#define SDN_PORT pADI_GP1
#define SDN_PIN PIN2
#define LOP_PORT pADI_GP1
#define LOP_PIN PIN1
#define LON_PORT pADI_GP1
#define LON_PIN PIN0

void ADC0_init(void)
{
	// ADC configuration.
	AdcRng(pADI_ADC0, ADCCON_ADCREF_INTREF, ADCMDE_PGA_G1, ADCCON_ADCCODE_INT);
	/* Note That, if sampling rate is greater than or equal to 500Hz, ADCFLT_SINC4EN must be set. */
    AdcFlt(pADI_ADC0, 18, 0, 0); //
	pADI_ADC0->CON &= ~(uint32_t)ADCCON_ADCREF_MSK;
	pADI_ADC0->CON |= ADCCON_ADCREF_INTREF;
	AdcPin(pADI_ADC0,ADCCON_ADCCN_AIN1,ADCCON_ADCCP_AIN0);
    
	AdcMski(pADI_ADC0, ADCMSKI_RDY, 1); 
    AdcBuf(pADI_ADC0,ADCCFG_EXTBUF_OFF,ADCCON_BUFBYPN|ADCCON_BUFBYPP|ADCCON_BUFPOWP|ADCCON_BUFPOWN); 
    AdcGo(pADI_ADC0, ADCMDE_ADCMD_IDLE);
    NVIC_EnableIRQ(ADC0_IRQn);
	
    // config shutdown pin
    DioCfgPin(SDN_PORT,SDN_PIN,0); // config SDN pin as GPIO
    DioOenPin(SDN_PORT,SDN_PIN,0); // config SDN pin as output

    // config lead off detection pin.
    DioCfgPin(LOP_PORT,LOP_PIN,0); // config LO+ pin as input GPIO
    DioCfgPin(LON_PORT,LON_PIN,0); // config LO- pin as input GPIO

    ECG_afe_start();
}

void ECG_start_continuous(void)
{
	AdcGo(pADI_ADC0, ADCMDE_ADCMD_IDLE);
	if ((pADI_ADC0->MDE & ADCMDE_ADCMD_MSK) != ADCMDE_ADCMD_CONT){
		AdcGo(pADI_ADC0, ADCMDE_ADCMD_CONT);
	}
}

/**
 * check if RA lead off
 * @param  void [description]
 * @return  check if RA lead off
 *      1 , RA lead off
 *      0 , RA lead not off
 */
int ECG_check_RA(void)
{
    int value = DioRd(LON_PORT);
    return (value & (1<<LON_PIN))?1:0;
}

/**
 * check if LA lead off
 * @param  viod [description]
 * @return  check if LA lead off
 *      1 , LA lead off
 *      0 , LA lead not off
 */
int ECG_check_LA(void)
{
    int value = DioRd(LOP_PORT);
    return (value & (1<<LOP_PIN))?1:0;
}

void ECG_afe_shutdown(void)
{
    DioClr(SDN_PORT,SDN_PIN);
}

void ECG_afe_start(void)
{
    DioSet(SDN_PORT,SDN_PIN);
}
