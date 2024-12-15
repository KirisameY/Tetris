#include "tetris.h"
#include "oled/oled.h"

#include <stdint.h>

extern uint8_t GuiBorder[];

extern uint8_t scr_cache[128][8]; // 图像缓存
extern uint8_t scr_dirty[16];     // 脏标记(每个位代表一个列)
uint16_t grid[20];                // 游戏网格，低10位为数据，高位在左；最高位为该行脏标记；其他位暂无意义，宜保留0值。

void Tetris_InitializeGameState(void);
void Tetris_Draw(void);

/// @brief 执行游戏的主循环
/// @param
void Tetris_MainGameLoop(void)
{
    Tetris_InitializeGameState();

    // 游戏主循环
    uint8_t gameloop = 1;
    while (gameloop)
    {
        // todo: 要等待一定时间间隔，进行一次时间流逝判定（计时器整上）；
        //       有输入时应当立即处理输入并视情况重置计时（如输入了指令“下”方块下落至底部时）；
        //       时间判定和输入处理后应当立刻执行一次显示更新

        Tetris_Draw(); // 执行一个绘制帧
    }
}

void Tetris_InitializeGameState(void)
{
    // 初始化图形缓存:
    for (uint8_t i = 0; i < 128; i++)
    {
        for (uint8_t j = 0; j < 8; j++)
        {
            scr_cache[i][j] = GuiBorder[i * 8 + j];
        }
    }

    // 初始化游戏网格
    for (uint8_t i = 0; i < 20; i++)
    {
        grid[i] = 0;
    }

    OLED_ForceUpdateScreen(); // 会同时重置脏标记
}

void Tetris_Draw(void)
{
    //todo 计算画面更新
    OLED_UpdateScreen();
}
