#ifndef __ADCMOD_H
#define __ADCMOD_H

#include <stdint.h>

void ADCMod_Init(void);
double ADCMod_GetKnob(void);
uint32_t ADCMod_GetRandom(void);

#endif
