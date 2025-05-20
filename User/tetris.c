#include "tetris.h"
#include "oled/oled.h"
#include "led/bsp_gpio_led.h"
#include "led/led_extension.h"
#include "input/input.h"
#include "random/random.h"
#include "main.h"

#include <stdint.h>
#include <math.h>

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

extern uint64_t scr_Cache[128];       // 图像缓存
extern uint8_t scr_Dirty[16];         // 图像缓存脏标记(每个位代表一个列)

static uint32_t _score;               // 游戏得分
static uint16_t _gameGrid[24];        // 游戏网格，低10位为数据，高位在左；最高位为该行脏标记；其他位暂无意义，宜保留0值。[20]开始是溢出位，不显示
static BlockType _currentBlock;       // 当前正在下落的块
static uint8_t _currentBlockRotation; // 当前正在下落的块的朝向(0-3)
static int8_t _currentPosX;           // 当前正在下落的块的x轴位置(相对最左下角)
static int8_t _currentPosY;           // 当前正在下落的块的y轴位置(相对最左下角)
static BlockType _savedBlock;         // 被暂存的块
static BlockType _nextBlock;          // 下一个会出的块
static uint8_t _gDirty;               // 显示更新用脏标记，从最低位开始依次表示：score, saved, next, saveUnable
static int8_t _time;                  // 游戏次级计时器
static uint8_t _gameSpd = 10;         // 每帧时间，数值回头会改
static uint8_t _gameLoop;             // 主循环持续标志，为0时游戏结束
static uint8_t _gameFlags;            // 用于标志游戏的各项状态
static uint8_t _saveSwitched;         // 用于标志当前是否进行过saved切换
static BlockType _blockBag[7];        // 该轮的随机出块包
static uint8_t _blockBagPos;          // 当前出块在随机出块包的位置

#pragma region 图形们

// static const uint64_t G_GridCells[10] = {
//     //     (0b1111 << 5),
//     //     (0b1111 << 10),
//     //     (0b1111 << 15),
//     //     (0b1111 << 20),
//     //     (0b1111 << 25),
//     //     (0b1111 << 30),
//     //     (0b1111 << 35),
//     //     (0b1111 << 40),
//     //     奇异搞笑编译器不能编译时计算，我硬编码吧
//     //    0000    0000
//     0x00000000000001E0,
//     0x0000000000003C00,
//     0x0000000000078000,
//     0x0000000000F00000,
//     0x000000001E000000,
//     0x00000003C0000000,
//     0x0000007800000000,
//     0x00000F0000000000,
//     0x0001E00000000000,
//     0x003C000000000000,
// };

#define GG_GRIDCELL(i) ((uint64_t)0x0f << 5*(i+1))
#define GG_GRIDCELL_MASK \
    0x007FFFFFFFFFFFF0
#define GG_GRIDCELL_SHIFT_V 23


static const uint8_t GG_BlockPreviews[8][4] = {
    {0x00,0x00,0x00,0x00},//None
    {0x00,0xFF,0xFF,0x00},//I
    {0x7E,0x7E,0x18,0x18},//T
    {0x3C,0x3C,0x3C,0x3C},//O
    {0x1E,0x1E,0x78,0x78},//Z
    {0x78,0x78,0x1E,0x1E},//S
    {0x06,0x06,0x7E,0x7E},//J
    {0x60,0x60,0x7E,0x7E},//L
};
// static const uint8_t G_BlockPreviewShiftV = 12;
// static const uint8_t G_NextPreviewShiftH = 23;
// static const uint8_t G_SavePreviewShiftH = 5;
#define GG_BLOCKPREV_SHIFT_V 12
#define GG_NEXTPREV_SHIFT_H 23
#define GG_SAVEPREV_SHIFT_H 5


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
// static const uint8_t G_NumsMask = 0x7C;
// static const uint8_t G_NumsShiftH = 20;
#define GG_NUMS_MASK 0x7C
#define GG_NUMS_SHIFT_H 20

// static const uint32_t G_SavUnable = 0x00016842;
// static const uint8_t G_SavUnableShiftV = 5;
#define GG_SAV_UNABLE 0x00016842
#define GG_SAV_UNABLE_SHIFT_V 5

#pragma endregion

#pragma region 游戏常量

static const uint16_t Blocks[8][4]= { // 格式：Blocks[类型][朝向] 里面每4位代表一个块的坐标，高2位Y低两位X
    {0x0000,0x0000,0x0000,0x0000}, // None, this should not be used
    {0x159D,0x4567,0x159D,0x4567}, // I
    {0x1569,0x1456,0x1459,0x4569}, // T
    {0x1256,0x1256,0x1256,0x1256}, // O
    {0x156A,0x1245,0x156A,0x1245}, // Z
    {0x2569,0x0156,0x2569,0x0156}, // S
    {0x126A,0x4568,0x159A,0x2456}, // J
    {0x1259,0x0456,0x269A,0x456A}, // L
};

#define GF_FAST_DROP 0x01 // 速降标志（下输入）
#define GF_T_SPIN 0x02    // T-Spin标志（最后一步为旋转而非下落）todo:未实装

#pragma endregion

static void _InitializeGameState(void);
static void _Input(Input_Type input);
static void _Time(void);
static void _Draw(void);

/// @brief 执行游戏的主循环，游戏结束时返回
/// @param timescale 游戏时间倍率，单位为计时器间隔
/// @return 最终得分
uint32_t Tetris_MainGameLoop(uint8_t timescale)
{
    _gameSpd = timescale;
    _InitializeGameState();

    // 游戏主循环
    while (_gameLoop)
    {
        while(_time > 0) // 帧时间
        {
            Input_Type input = Input_Pop();
            if(input) {
                _Input(input); // 等待并阻塞主循环，但轮询保持输入监听
                _Draw();       // 过个绘制帧，更新显示
            }
        }

        _Time(); // 时间更新
        _Draw(); // 在最后执行一个绘制帧
        Led_Flash_B();
        while (_time <= 0) ; // 锁定程序至下一个时间周期，防止程序重复执行
    }

    return _score;
}

void Tetris_TimHandler(void)
{
    _time = (_time+1) % _gameSpd;
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
    for (uint8_t i = 0; i < 24; i++)
    {
        _gameGrid[i] = 0;
    }

    // 初始化游戏数据
    _score = 0;
    _currentBlock = _nextBlock = _savedBlock = BlockType_None;
    _gDirty = 0;
    _gameFlags = 0;
    _saveSwitched = 0;
    _blockBagPos = 0;
    _gameLoop = 1;
    _time = 1; // 初始化时给一帧缓冲时间

    // 清除现有输入，以防误输入
    Input_Pop();

    OLED_ForceUpdateScreen(); // 初始化显示，会同时重置scr_dirty
}

#pragma region 游戏逻辑

static void _UpdateNext(void);
static void _SaveSwitch(void);
static void _AfterDrop(void);
static int8_t _TryMoveCurrent(int8_t x, int8_t y);
static int8_t _TryRotateCurrent(int8_t dir);
static uint8_t _FastDrop(void);
static void _GetBlockRealPos(BlockType block, uint8_t rotate, int8_t x, int8_t y, int8_t* buffer);
static int8_t _DetectCollision(int8_t* blocks, int8_t xshift, int8_t yshift);

static void _Input(Input_Type input)
{
    uint8_t input_valid = 1;
    switch (input)
    {
        case Input_Type_Up:    
            _SaveSwitch();
            break;
        case Input_Type_Down:  
            _gameFlags |= GF_FAST_DROP;
            _time = -1;
            break;
        case Input_Type_Left:  
            _TryMoveCurrent(-1,0);
            break;
        case Input_Type_Right: 
            _TryMoveCurrent(1,0);
            break;
        case Input_Type_Clockwise:
            _TryRotateCurrent(1);
            _time = 1; // 旋转操作刷新倒计时
            break;
        case Input_Type_AntiClockwise:
            _TryRotateCurrent(-1);
            _time = 1;
            break;

        default: input_valid=0; break;
    }

    if (input_valid) Led_Flash_G();
}

static void _Time(void)
{
    if(_currentBlock == BlockType_None)
    {
        _currentPosX = 3;
        _currentPosY = 20;
        _currentBlockRotation = 0;
        _UpdateNext();
    }

    if (_gameFlags & GF_FAST_DROP)
    {
        if(_FastDrop()) _gameFlags &= ~GF_T_SPIN;
        _gameFlags &= ~GF_FAST_DROP;
        _currentBlock = BlockType_None;
        _AfterDrop();
        return;
    }
    
    if(_TryMoveCurrent(0, -1))
    {
        _gameFlags &= ~GF_T_SPIN;
    }
    else
    {
        _currentBlock = BlockType_None;
        _AfterDrop();
        return;
    }
}

static void _UpdateNext(void)
{
    if(_blockBagPos == 0)
    {
        Random_Shuffle(1, 7, _blockBag);
    }
    _currentBlock = _nextBlock;
    _nextBlock = _blockBag[_blockBagPos];
    _blockBagPos = (_blockBagPos+1) % 7;

    _gDirty |= 0x04;

    if(_currentBlock == BlockType_None) _UpdateNext(); // 第一次执行时会递归一次以填充next和current
}

void _SaveSwitch(void)
{
    if (_saveSwitched) return;
    if (_currentBlock == BlockType_None) return;
    _saveSwitched = 1;

    int8_t blocks[4][2];
    _GetBlockRealPos(_currentBlock, _currentBlockRotation, _currentPosX, _currentPosY, blocks[0]);
    for (uint8_t i = 0; i < 4; i++)
    {
        if (blocks[i][1] >= 24) continue;
        _gameGrid[blocks[i][1]] &= ~(1 << (blocks[i][0]));
        _gameGrid[blocks[i][1]] |= 0x8000;
    }

    _gDirty |= 0x0A;

    _currentPosX = 3;
    _currentPosY = 20;
    _currentBlockRotation = 0;
    BlockType tmp = _savedBlock;
    _savedBlock = _currentBlock;
    _currentBlock = tmp;
}

void _AfterDrop(void)
{
    // 重置sav状态
    if(_saveSwitched)
    {
        _saveSwitched = 0;
        _gDirty |= ~0x01;
    }

    // 判定消行
    uint8_t h = 0;
    while (_gameGrid[h] != 0 && h < 24)
    {
        h++;
    }

    uint8_t cleared = 0;
    uint8_t i = 0;
    while (i < h)
    {
        if((_gameGrid[i] & 0x03FF) != 0x03FF)
        {
            i++;
            continue;
        }

        cleared++;
        for (uint8_t j = i; j < h; j++)
        {
            if(j == h-1) _gameGrid[j] = 0;
            else _gameGrid[j] = _gameGrid[j+1];
            _gameGrid[j] |= 0x8000;
        }
    }
    
    if (cleared != 0)
    {
        _gDirty |= 0x01;
        uint32_t add_score = pow(2, cleared) * (19-_gameSpd);
        if (_gameFlags & GF_T_SPIN) add_score *= 2;
        _score += add_score;
    }

    // 判定gameover
    if(_gameGrid[20] & 0x03FF) _gameLoop = 0;
}

static int8_t _TryMoveCurrent(int8_t x, int8_t y)
{
    if(_currentBlock == BlockType_None) return 1;

    int8_t blocks[4][2];
    _GetBlockRealPos(_currentBlock, _currentBlockRotation, _currentPosX, _currentPosY, blocks[0]);
    for (uint8_t i = 0; i < 4; i++)
    {
        if (blocks[i][1] >= 24) continue;
        _gameGrid[blocks[i][1]] &= ~(1 << (blocks[i][0]));
        _gameGrid[blocks[i][1]] |= 0x8000;
    }

    int8_t succeed = _DetectCollision(blocks[0], x, y);

    if (succeed)
    {
        _currentPosX += x;
        _currentPosY += y;
    }

    _GetBlockRealPos(_currentBlock, _currentBlockRotation, _currentPosX, _currentPosY, blocks[0]);
    for (uint8_t i = 0; i < 4; i++)
    {
        if (blocks[i][0] < 0 || blocks[i][0] >= 10 || blocks[i][1] < 0 || blocks[i][1] >= 24)
            continue;
        _gameGrid[blocks[i][1]] |= 1 << (blocks[i][0]);
        _gameGrid[blocks[i][1]] |= 0x8000;
    }

    return succeed;
}

int8_t _TryRotateCurrent(int8_t dir)
{
    if(_currentBlock == BlockType_None) return 1;

    int8_t blocks[4][2];
    _GetBlockRealPos(_currentBlock, _currentBlockRotation, _currentPosX, _currentPosY, blocks[0]);
    for (uint8_t i = 0; i < 4; i++)
    {
        if (blocks[i][1] >= 24) continue;
        _gameGrid[blocks[i][1]] &= ~(1 << (blocks[i][0]));
        _gameGrid[blocks[i][1]] |= 0x8000;
    }
    
    _GetBlockRealPos(_currentBlock, (_currentBlockRotation+4+dir)%4, _currentPosX, _currentPosY, blocks[0]);
    int8_t succeed = 0;
    succeed = _DetectCollision(blocks[0], 0, 0);
    if (!succeed) 
    {
        succeed = _DetectCollision(blocks[0], 1, 0);
        if (succeed) _currentPosX+=1;
    }
    if (!succeed) 
    {
        succeed = _DetectCollision(blocks[0], -1, 0);
        if (succeed) _currentPosX-=1;
    }
    if (!succeed) 
    {
        succeed = _DetectCollision(blocks[0], 0, 1);
        if (succeed) _currentPosY+=1;
    }
    if (!succeed) 
    {
        succeed = _DetectCollision(blocks[0], 0, -1);
        if (succeed) _currentPosY-=1;
    }

    if (succeed)
    {
        _currentBlockRotation = (_currentBlockRotation+4+dir) % 4;
        _gameFlags |= GF_T_SPIN;
    }

    _GetBlockRealPos(_currentBlock, _currentBlockRotation, _currentPosX, _currentPosY, blocks[0]);
    for (uint8_t i = 0; i < 4; i++)
    {
        if (blocks[i][0] < 0 || blocks[i][0] >= 10 || blocks[i][1] < 0 || blocks[i][1] >= 24)
            continue;
        _gameGrid[blocks[i][1]] |= 1 << (blocks[i][0]);
        _gameGrid[blocks[i][1]] |= 0x8000;
    }

    return succeed;
}

uint8_t _FastDrop(void)
{
    if(_currentBlock == BlockType_None) return 0;

    int8_t blocks[4][2];
    _GetBlockRealPos(_currentBlock, _currentBlockRotation, _currentPosX, _currentPosY, blocks[0]);
    for (uint8_t i = 0; i < 4; i++)
    {
        if (blocks[i][1] >= 24) continue;
        _gameGrid[blocks[i][1]] &= ~(1 << (blocks[i][0]));
        _gameGrid[blocks[i][1]] |= 0x8000;
    }

    uint8_t dropping = 1;
    int8_t yshift=0;
    while (dropping)
    {
        yshift--;
        dropping = _DetectCollision(blocks[0], 0, yshift);
    }
    yshift++;
    
    _currentPosY += yshift;

    _GetBlockRealPos(_currentBlock, _currentBlockRotation, _currentPosX, _currentPosY, blocks[0]);
    for (uint8_t i = 0; i < 4; i++)
    {
        if (blocks[i][0] < 0 || blocks[i][0] >= 10 || blocks[i][1] < 0 || blocks[i][1] >= 24)
            continue;
        _gameGrid[blocks[i][1]] |= 1 << (blocks[i][0]);
        _gameGrid[blocks[i][1]] |= 0x8000;
    }

    return (uint8_t)-yshift;
}

static void _GetBlockRealPos(BlockType block, uint8_t rotate, int8_t x, int8_t y, int8_t* buffer)
{
    if(block == BlockType_None) exception("Tried to get blocks of BlockType_None");

    uint16_t blocks = Blocks[block][rotate];
    for (uint8_t i = 0; i < 4; i++)
    {
        buffer[2*i] = x + (blocks & 0x03);
        blocks>>=2;
        buffer[2*i+1] = y + (blocks & 0x03);
        blocks>>=2;
    }
}

static int8_t _DetectCollision(int8_t *blocks, int8_t xshift, int8_t yshift)
{
    for (uint8_t i = 0; i < 8; i += 2)
    {
        int8_t newx = blocks[i]+xshift, newy = blocks[i+1]+yshift;
        if (newy >= 20) continue;
        if (newx < 0 || newx >= 10 || newy < 0 ||
            (_gameGrid[newy] & (1<<(newx))) != 0) 
        {
            return 0;
        }
    }

    return 1;
}

#pragma endregion

#pragma region 图像更新&绘制

static void _CalculateGridLine(uint8_t h, uint16_t grid);
static void _Draw(void)
{
    // 更新得分显示
    if (_gDirty & 0x01)
    {
        uint32_t score = _score;
        for (uint8_t i = 0; score != 0 ; i++) // 考虑到一局内分数只会递增而开始时会重置UI，不需要考虑更新更高位
        {
            uint8_t n = score%10;
            for (uint8_t j = 1; j <= 3; j++)
            {
                uint8_t scr_h = GG_NUMS_SHIFT_H + i*4 +j;
                scr_Dirty[scr_h/8] |= 0x80>>(scr_h%8);
                scr_Cache[scr_h] |= (uint64_t)GG_NUMS_MASK << 56; // 分数显示是反色的
                scr_Cache[scr_h] &= ~((uint64_t)G_Nums[n][j-1] << 56);
            }
            score/=10;
        }
        
    }

    // 更新方块预览画面
    if (_gDirty & 0x02) // SavedBlock
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            uint8_t scr_h = GG_BLOCKPREV_SHIFT_V+i;
            scr_Dirty[scr_h/8] |= 0x80>>(scr_h%8);
            scr_Cache[scr_h] &= ~((uint64_t)0xFF << GG_SAVEPREV_SHIFT_H);
            scr_Cache[scr_h] |= (uint64_t)GG_BlockPreviews[_savedBlock][i] << GG_SAVEPREV_SHIFT_H;
        }
    }
    if (_gDirty & 0x04) // NextBlock
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            uint8_t scr_h = GG_BLOCKPREV_SHIFT_V+i;
            scr_Dirty[scr_h/8] |= 0x80>>(scr_h%8);
            scr_Cache[scr_h] &= ~((uint64_t)0xFF << GG_NEXTPREV_SHIFT_H);
            scr_Cache[scr_h] |= (uint64_t)GG_BlockPreviews[_nextBlock][i] << GG_NEXTPREV_SHIFT_H;
        }
    }

    // 更新sav字符状态
    if (_gDirty & 0x08)
    {
        scr_Dirty[GG_SAV_UNABLE_SHIFT_V/8] |= 0x80>>(GG_SAV_UNABLE_SHIFT_V%8);
        if (_saveSwitched) 
        {
            scr_Cache[GG_SAV_UNABLE_SHIFT_V] &= ~(uint64_t)GG_SAV_UNABLE;
        }
        else
        {
            scr_Cache[GG_SAV_UNABLE_SHIFT_V] |= GG_SAV_UNABLE;
        }
    }

    // 更新游戏格子画面
    for (uint8_t i = 0; i < 20; i++)
    {
        if ((_gameGrid[i] & 0x8000) == 0) continue; // 检查脏标记

        _CalculateGridLine(i, _gameGrid[i]); // 更新格子显示

        _gameGrid[i] &= 0x7fff; // 清除脏标记
    }

    // 当前块特殊显示
    if (_currentBlock != BlockType_None)
    {
        int8_t blocks[4][2];
        _GetBlockRealPos(_currentBlock, _currentBlockRotation, _currentPosX, _currentPosY, blocks[0]);
        for (uint8_t i = 0; i < 4; i++)
        {
            if (blocks[i][1] >= 20) continue;
            uint8_t scr_h = GG_GRIDCELL_SHIFT_V + (19-blocks[i][1])*5;
            uint64_t cellv = (uint64_t)0x06 << (5 * (blocks[i][0]+1));
            scr_Cache[scr_h+2] &= ~cellv;
            scr_Cache[scr_h+3] &= ~cellv;
            /* code */
        }
    }

    // 清除脏标记
    _gDirty = 0;

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
        line |= GG_GRIDCELL(i);//G_GridCells[i];
    }
    uint8_t scr_hb = GG_GRIDCELL_SHIFT_V + (19-h)*5;
    // scr_dirty[scr_hb/8] |= 0x80>>(scr_hb%8); // 算了一下，还是不必要的行就略过开销更小
    for (uint8_t j = 1; j <= 4; j++)
    {
        uint8_t scr_h = scr_hb + j;
        scr_Dirty[scr_h/8] |= 0x80>>(scr_h%8);
        scr_Cache[scr_h] &=  ~GG_GRIDCELL_MASK ;
        scr_Cache[scr_h] |= line;
    }
}

#pragma endregion
