#include "tetris.h"
#include "oled/oled.h"
#include "led/bsp_gpio_led.h"
#include "main.h"

#include <stdint.h>

typedef enum{
    BlockType_None,
    BlockType_I,
    BlockType_T,
    BlockType_O,
    BlockType_Z,
    BlockType_S,
    BlockType_J,
    BlockType_L,
} BlockType;

extern uint8_t GuiBorder[];

extern uint64_t scr_Cache[128]; // 图像缓存
extern uint8_t scr_Dirty[16];   // 脏标记(每个位代表一个列)
uint16_t Tetris_GameGrid[20];   // 游戏网格，低10位为数据，高位在左；最高位为该行脏标记；其他位暂无意义，宜保留0值。
uint32_t Tetris_Score;          // 游戏得分
BlockType Tetris_CurrentBlock;  // 当前正在下落的块
BlockType Tetris_SavedBlock;    // 被暂存的块
BlockType Tetris_NextBlock;     // 下一个会出的块
uint8_t Tetris_ScrDirty;        // 显示更新用脏标记，从最低位开始依次表示：score, saved, next

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
const uint8_t G_GridCellShiftV = 23;


const uint8_t G_BlockPreviews[8][4] = {
    {0x00,0x00,0x00,0x00},//None
    {0x00,0xFF,0xFF,0x00},//I
    {0x7E,0x7E,0x18,0x18},//T
    {0x3C,0x3C,0x3C,0x3C},//O
    {0x1E,0x1E,0x78,0x78},//Z
    {0x78,0x78,0x1E,0x1E},//S
    {0x06,0x06,0x7E,0x7E},//J
    {0x60,0x60,0x7E,0x7E},//L
};
const uint8_t G_BlockPreviewShiftV = 12;
const uint8_t G_NextPreviewShiftH = 23;
const uint8_t G_SavePreviewShiftH = 5;


const uint8_t G_Nums[10][3] = {
    {0x7C,0x44,0x7C},//0
    {0x40,0x7C,0x44},//1
    {0x5C,0x54,0x74},//2
    {0x7C,0x54,0x54},//3
    {0x7C,0x10,0x1C},//4
    {0x74,0x54,0x5C},//5
    {0x74,0x54,0x7C},//6
    {0x7C,0x04,0x04},//7
    {0x7C,0x54,0x7C},//8
    {0x7C,0x54,0x5C},//9
};
const uint8_t G_NumsMask = 0x7C;
const uint8_t G_NumsShiftH = 20;

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
    Tetris_Score = 1024357689;
    Tetris_SavedBlock = BlockType_I;
    Tetris_NextBlock = BlockType_S;
    Tetris_ScrDirty = 0x07;

    // 游戏主循环
    int gameloop = 1;
    while (gameloop)
    {
        // todo: 要等待一定时间间隔，进行一次时间流逝判定（计时器整上）；
        //       有输入时应当立即处理输入并视情况重置计时（如输入了指令“下”方块下落至底部时）；
        //       时间判定和输入处理后应当立刻执行一次显示更新
        //       输入和时间可以闪一下绿/蓝灯

        Tetris_Draw(); // 在最后执行一个绘制帧

        while (1) ; // 写完之前暂时阻塞一下循环
    }
}

void Tetris_InitializeGameState(void)
{
    // 初始化图形缓存:
    for (uint8_t i = 0; i < 128; i++)
    {
        scr_Cache[i] = 0;
        for (uint8_t j = 0; j < 8; j++)
        {
            scr_Cache[i] |= (uint64_t)GuiBorder[i * 8 + j] << (j * 8);
        }
    }

    // 初始化游戏网格
    for (uint8_t i = 0; i < 20; i++)
    {
        Tetris_GameGrid[i] = 0;
    }

    // 初始化游戏数据
    Tetris_Score = 0;
    Tetris_CurrentBlock = Tetris_NextBlock = Tetris_SavedBlock = BlockType_None;
    Tetris_ScrDirty = 0;

    OLED_ForceUpdateScreen(); // 会同时重置scr_dirty
}

#pragma region 图像更新&绘制

void Tetris_CalculateGridLine(uint8_t h, uint16_t grid);
void Tetris_Draw(void)
{
    // 更新得分显示
    if (Tetris_ScrDirty & 0x01)
    {
        uint32_t score = Tetris_Score;
        for (uint8_t i = 0; score != 0 ; i++) // 考虑到一局内分数只会递增而开始时会重置UI，不需要考虑更新更高位的显式
        {
            uint8_t n = score%10;
            for (uint8_t j = 1; j <= 3; j++)
            {
                uint8_t scr_h = G_NumsShiftH + i*4 +j;
                scr_Dirty[scr_h/8] |= 0x80>>(scr_h%8);
                scr_Cache[scr_h] |= (uint64_t)G_NumsMask << 56; // 分数显示是反色的
                scr_Cache[scr_h] &= ~((uint64_t)G_Nums[n][j-1] << 56);
            }
            score/=10;
        }
        
    }

    // 更新方块预览画面
    if (Tetris_ScrDirty & 0x02) // SavedBlock
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            uint8_t scr_h = G_BlockPreviewShiftV+i;
            scr_Dirty[scr_h/8] |= 0x80>>(scr_h%8);
            scr_Cache[scr_h] &= ~((uint64_t)0xFF << G_SavePreviewShiftH);
            scr_Cache[scr_h] |= (uint64_t)G_BlockPreviews[Tetris_SavedBlock][i] << G_SavePreviewShiftH;
        }
    }
    if (Tetris_ScrDirty & 0x04) // NextBlock
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            uint8_t scr_h = G_BlockPreviewShiftV+i;
            scr_Dirty[scr_h/8] |= 0x80>>(scr_h%8);
            scr_Cache[scr_h] &= ~((uint64_t)0xFF << G_NextPreviewShiftH);
            scr_Cache[scr_h] |= (uint64_t)G_BlockPreviews[Tetris_NextBlock][i] << G_NextPreviewShiftH;
        }
    }

    // 更新游戏格子画面
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
    uint8_t scr_hb = G_GridCellShiftV + (19-h)*5;
    // scr_dirty[scr_hb/8] |= 0x80>>(scr_hb%8); // 算了一下，还是不必要的行就略过开销更小
    for (uint8_t j = 1; j <= 4; j++)
    {
        uint8_t scr_h = scr_hb + j;
        scr_Dirty[scr_h/8] |= 0x80>>(scr_h%8);
        scr_Cache[scr_h] &=  ~G_GridCellMask ;
        scr_Cache[scr_h] |= line;
    }
}

#pragma endregion
