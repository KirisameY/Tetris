#include "tetris.h"
#include "oled/oled.h"
#include "led/bsp_gpio_led.h"
#include "input/input.h"
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
extern Input_Type Input_Current;

extern uint64_t scr_Cache[128]; // 图像缓存
extern uint8_t scr_Dirty[16];   // 脏标记(每个位代表一个列)
static uint16_t _gameGrid[20];   // 游戏网格，低10位为数据，高位在左；最高位为该行脏标记；其他位暂无意义，宜保留0值。
static uint32_t _score;          // 游戏得分
static BlockType _currentBlock;  // 当前正在下落的块
static BlockType _savedBlock;    // 被暂存的块
static BlockType _nextBlock;     // 下一个会出的块
static uint8_t _scrDirty;        // 显示更新用脏标记，从最低位开始依次表示：score, saved, next

#pragma region 图形们

static const uint64_t G_GridCells[10] = {
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
static const uint64_t G_GridCellMask = 
    0x007FFFFFFFFFFFF0;
static const uint8_t G_GridCellShiftV = 23;


static const uint8_t G_BlockPreviews[8][4] = {
    {0x00,0x00,0x00,0x00},//None
    {0x00,0xFF,0xFF,0x00},//I
    {0x7E,0x7E,0x18,0x18},//T
    {0x3C,0x3C,0x3C,0x3C},//O
    {0x1E,0x1E,0x78,0x78},//Z
    {0x78,0x78,0x1E,0x1E},//S
    {0x06,0x06,0x7E,0x7E},//J
    {0x60,0x60,0x7E,0x7E},//L
};
static const uint8_t G_BlockPreviewShiftV = 12;
static const uint8_t G_NextPreviewShiftH = 23;
static const uint8_t G_SavePreviewShiftH = 5;


static const uint8_t G_Nums[10][3] = {
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
static const uint8_t G_NumsMask = 0x7C;
static const uint8_t G_NumsShiftH = 20;

#pragma endregion

static void _InitializeGameState(void);
static void _Input(void);
static void _Draw(void);

//temp
static uint8_t _xpos, _ypos;

/// @brief 执行游戏的主循环
/// @param
void Tetris_MainGameLoop(void)
{
    _InitializeGameState();

    // 测试用：
    _gameGrid[19] = 1<<2 | 0x8000;
    _gameGrid[18] = 1<<7 | 0x8000;
    _gameGrid[5] = 1<<2 | 0x8000;
    _score = 0;//1024357689;
    _savedBlock = BlockType_I;
    _nextBlock = BlockType_S;
    _scrDirty = 0x07;

    // 游戏主循环
    int gameloop = 1;
    while (gameloop)
    {
        // todo: 要等待一定时间间隔，进行一次时间流逝判定（计时器整上）；
        //       有输入时应当立即处理输入并视情况重置计时（如输入了指令“下”方块下落至底部时）；
        //       时间判定和输入处理后应当立刻执行一次显示更新
        //       输入和时间可以闪一下绿/蓝灯

        while(Input_Current == Input_Type_None) {}; //todo: 加入计时后就不要阻塞轮询了
        _Input();

        _Draw(); // 在最后执行一个绘制帧
    }
}

static void _InitializeGameState(void)
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
        _gameGrid[i] = 0;
    }

    // 初始化游戏数据
    _score = 0;
    _currentBlock = _nextBlock = _savedBlock = BlockType_None;
    _scrDirty = 0;

    //temp
    _xpos = _ypos = 0;

    OLED_ForceUpdateScreen(); // 会同时重置scr_dirty
}

static void _Input(void)
{
    //temp 测试输入用

    _gameGrid[_ypos] |= 0x8000;
    _gameGrid[_ypos] &= ~(1 << _xpos);

    switch (Input_Current)
    {
        case Input_Type_Up:    
            _ypos = MIN(_ypos+1, 19); 
            _savedBlock = (_savedBlock+1)%8;
            break;
        case Input_Type_Down:  
            _ypos = MAX(_ypos-1, 0);   
            _savedBlock = (_savedBlock+6)%8;
            break;
        case Input_Type_Left:  
            _xpos = MAX(_xpos-1, 0);  
            _nextBlock = (_nextBlock+6)%8;
            break;
        case Input_Type_Right: 
            _xpos = MIN(_xpos+1, 9);   
            _nextBlock = (_nextBlock+1)%8;
            break;
        
        default: break;
    }

    Input_Clear();
    _score++;
    _scrDirty |= 0x07;
    _gameGrid[_ypos] |= 0x8000;
    _gameGrid[_ypos] |= 1 << _xpos;
}

#pragma region 图像更新&绘制

static void _CalculateGridLine(uint8_t h, uint16_t grid);
static void _Draw(void)
{
    // 更新得分显示
    if (_scrDirty & 0x01)
    {
        uint32_t score = _score;
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
    if (_scrDirty & 0x02) // SavedBlock
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            uint8_t scr_h = G_BlockPreviewShiftV+i;
            scr_Dirty[scr_h/8] |= 0x80>>(scr_h%8);
            scr_Cache[scr_h] &= ~((uint64_t)0xFF << G_SavePreviewShiftH);
            scr_Cache[scr_h] |= (uint64_t)G_BlockPreviews[_savedBlock][i] << G_SavePreviewShiftH;
        }
    }
    if (_scrDirty & 0x04) // NextBlock
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            uint8_t scr_h = G_BlockPreviewShiftV+i;
            scr_Dirty[scr_h/8] |= 0x80>>(scr_h%8);
            scr_Cache[scr_h] &= ~((uint64_t)0xFF << G_NextPreviewShiftH);
            scr_Cache[scr_h] |= (uint64_t)G_BlockPreviews[_nextBlock][i] << G_NextPreviewShiftH;
        }
    }

    // 更新游戏格子画面
    for (uint8_t i = 0; i < 20; i++)
    {
        if ((_gameGrid[i] & 0x8000) == 0) continue; // 检查脏标记

        _CalculateGridLine(i, _gameGrid[i]); // 更新格子显示

        _gameGrid[i] &= 0x7fff; // 清除脏标记
    }

    // 刷新屏幕
    OLED_UpdateScreen();
    return;
}

static void _CalculateGridLine(uint8_t h, uint16_t grid)
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
