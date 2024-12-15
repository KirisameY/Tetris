#include "tetris.h"
#include "oled/oled.h"

#include <stdint.h>

extern uint8_t GuiBorder[];

uint8_t cache[128][8];
uint8_t dirty[128];

/// @brief 执行游戏的主循环
/// @param 
void Tetris_MainGameLoop(void)
{
    // 初始化图形缓存及屏幕显示:
    for (uint8_t i = 0; i < 128; i++)
    {
        dirty[i] = 0;
        for (uint8_t j = 0; j < 8; j++)
        {
            cache[i][j] = GuiBorder[i*8+j];
        }
    }

    OLED_DrawPic(cache[0], 0, 8, 0, 128);
    

    uint8_t gameloop = 1;
    while (gameloop)
    {
        // 游戏主循环
        // todo: 要等待一定时间间隔，进行一次时间流逝判定；
        //       有输入时应当立即处理输入并视情况重置计时（如输入了指令“下”方块下落至底部时）
        //       时间判定和输入处理后应当立刻执行一次显示更新
    }
}
