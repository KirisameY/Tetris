#ifndef __TETRIS_H
#define __TETRIS_H

#include <stdint.h>

uint32_t Tetris_MainGameLoop(uint8_t timescale);
void Tetris_TimHandler(void);

#endif
