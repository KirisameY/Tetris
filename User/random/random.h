#ifndef __RANDOM_H
#define __RANDOM_H

#include <stdint.h>

void Random_Init(void);
void Random_ResetSeed(void);
int Random_Next(int from, int to);
void Random_Shuffle(uint8_t from, uint8_t count, uint8_t* buffer);

#endif
