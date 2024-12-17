#include "tetris.h"
#include "oled/oled.h"
#include "main.h"

#include <stdint.h>

extern uint8_t GuiBorder[];

extern uint64_t scr_cache[128]; // 图像缓存
extern uint8_t scr_dirty[16];   // 脏标记(每个位代表一个列)
uint16_t Tetris_GameGrid[20];   // 游戏网格，低10位为数据，高位在左；最高位为该行脏标记；其他位暂无意义，宜保留0值。

enum BlockType{
    BlockType_I,
    BlockType_T,
    BlockType_O,
    BlockType_Z,
    BlockType_S,
    BlockType_J,
    BlockType_L,
};

#pragma region 图形们

const uint64_t G_GridCells[10] = {
    //     (0b1111 << 5),
    //     (0b1111 << 10),
    //     (0b1111 << 15),
    //     (0b1111 << 20),
    //     (0b1111 << 25),
    //     (0b1111 << 30),
    //     (0b1111 << 35),
    //     (0b1111 << 40),
    //     奇异搞笑编译器不能编译时计算，我硬编码吧
    //    0000    0000
    0x00000000000001E0,
    0x0000000000003C00,
    0x0000000000078000,
    0x0000000000F00000,
    0x000000001E000000,
    0x00000003C0000000,
    0x0000007800000000,
    0x00000F0000000000,
    0x0001E00000000000,
    0x003C000000000000,
};

const uint64_t G_GridCellMask =
    0x007FFFFFFFFFFFF0;

const uint8_t G_BlockPreviews[7][8] = {
    {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18},//I
    {0x00,0x30,0x30,0x3C,0x3C,0x30,0x30,0x00},//T
    {0x00,0x00,0x3C,0x3C,0x3C,0x3C,0x00,0x00},//O
    {0x00,0x0C,0x0C,0x3C,0x3C,0x30,0x30,0x00},//Z
    {0x00,0x30,0x30,0x3C,0x3C,0x0C,0x0C,0x00},//S
    {0x00,0x0C,0x0C,0x0C,0x0C,0x3C,0x3C,0x00},//J
    {0x00,0x30,0x30,0x30,0x30,0x3C,0x3C,0x00},//L
};

const uint8_t G_BlockPreviewStart = 10;
const uint8_t G_NextPreviewShift = 23;
const uint8_t G_SavePreviewShift = 5;

const uint8_t G_Nums[10][3] = {
    {0x3E,0x22,0x3E},//0
    {0x02,0x3E,0x22},//1
    {0x3A,0x2A,0x2E},//2
    {0x3E,0x2A,0x2A},//3
    {0x3E,0x08,0x38},//4
    {0x2E,0x2A,0x3A},//5
    {0x2E,0x2A,0x3E},//6
    {0x3E,0x20,0x20},//7
    {0x3E,0x2A,0x3E},//8
    {0x3E,0x2A,0x3A},//9
};

const uint8_t G_NumsMask = 0xC1;
const uint8_t G_NumsShift = 56;

#pragma endregion

void Tetris_InitializeGameState(void);
void Tetris_Draw(void);

/// @brief 执行游戏的主循环
/// @param
void Tetris_MainGameLoop(void)
{
    Tetris_InitializeGameState();

    // 测试用：
    Tetris_GameGrid[19] = 1<<2 | 0x8000;
    Tetris_GameGrid[18] = 1<<7 | 0x8000;
    Tetris_GameGrid[5] = 1<<2 | 0x8000;

    // 游戏主循环
    int gameloop = 1;
    while (gameloop)
    {
        // todo: 要等待一定时间间隔，进行一次时间流逝判定（计时器整上）；
        //       有输入时应当立即处理输入并视情况重置计时（如输入了指令“下”方块下落至底部时）；
        //       时间判定和输入处理后应当立刻执行一次显示更新
        //       输入和时间可以闪一下绿/蓝灯

        Tetris_Draw(); // 执行一个绘制帧

        while (1) ; // 写完之前暂时阻塞一下循环
    }
}

void Tetris_InitializeGameState(void)
{
    // 初始化图形缓存:
    for (uint8_t i = 0; i < 128; i++)
    {
        scr_cache[i] = 0;
        for (uint8_t j = 0; j < 8; j++)
        {
            scr_cache[i] |= (uint64_t)GuiBorder[i * 8 + j] << (j * 8);
        }
    }

    // 初始化游戏网格
    for (uint8_t i = 0; i < 20; i++)
    {
        Tetris_GameGrid[i] = 0;
    }

    OLED_ForceUpdateScreen(); // 会同时重置脏标记
}

void Tetris_CalculateGridLine(uint8_t h, uint16_t grid);
void Tetris_Draw(void)
{
    // todo 计算画面更新
    for (uint8_t i = 0; i < 20; i++)
    {
        if ((Tetris_GameGrid[i] & 0x8000) == 0) continue; // 检查脏标记

        Tetris_CalculateGridLine(i, Tetris_GameGrid[i]); // 更新格子显示

        Tetris_GameGrid[i] &= 0x7fff; // 清除脏标记
    }

    // 刷新屏幕
    OLED_UpdateScreen();
    return;
}

void Tetris_CalculateGridLine(uint8_t h, uint16_t grid)
{
    uint64_t line = 0;
    for (uint8_t i = 0; i < 10; i++)
    {
        if ((grid & 1<<i) == 0) continue;
        line |= G_GridCells[i];
    }
    uint8_t scr_hb = 24 + (19-h)*5;
    scr_dirty[scr_hb/8] |= 0x80>>(scr_hb%8);
    for (uint8_t j = 1; j <= 4; j++)
    {
        uint8_t scr_h = scr_hb + j;
        scr_dirty[scr_h/8] |= 0x80>>(scr_h%8);
        scr_cache[scr_h] &=  ~G_GridCellMask ;
        scr_cache[scr_h] |= line;
    }
}
