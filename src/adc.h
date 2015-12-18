#ifndef __ADC_HAL_H__
#define __ADC_HAL_H__

#include "ADUCM360.h"

void ECG_init(void);
void ECG_start_sample(void);
void ECG_start_continuous(void);
void HS_start_continuous(void);
void HS_start_sample(void);
void ECG_afe_shutdown(void);
void ECG_afe_start(void);

#endif
