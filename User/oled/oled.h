#ifndef __OLED_H
#define __OLED_H

#include <stdint.h>

//宏定义

#define OLED_CALL_WR        0x78 //写模式呼叫设备
#define OLED_WR_CMD_CO      0x00 //控制字节：连续写指令
#define OLED_WR_DATA_CO     0x40 //         连续写数据
#define OLED_WR_CMD_ONCE    0x80 //         单次写指令
#define OLED_WR_DATA_ONCE   0xC0 //         单次写数据

#define OLED_DRAW_FULLSCREEN(pic) OLED_DrawPic(pic, 0, 8, 0, 128);

//方法定义

void OLED_Initialize(void);
void OLED_DrawPic(uint8_t* pic, uint8_t xpos, uint8_t width, uint8_t ypos, uint8_t height);
void OLED_UpdateScreen(void);
void OLED_ForceUpdateScreen(void);

#endif
