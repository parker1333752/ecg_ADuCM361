#ifndef __ADC0_HAL_H__
#define __ADC0_HAL_H__

#include "ADUCM360.h"

void ADC0_init(void);
void ECG_start_sample(void);
void ECG_start_continuous(void);
//void HS_start_continuous(void);
//void HS_start_sample(void);
void ECG_afe_shutdown(void);
void ECG_afe_start(void);

#endif
