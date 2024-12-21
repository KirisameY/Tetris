#ifndef __GUI_H
#define __GUI_H

#include <stdint.h>

uint8_t GUI_Start(void);
void GUI_Gameover(uint32_t score);
void GUI_TimHandler(void);

#endif
