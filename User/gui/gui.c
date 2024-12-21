#include "gui.h"
#include "../adc/adcmod.h"
#include "../input/input.h"
#include "../oled/oled.h"

#include <math.h>

extern uint8_t GuiStart[];
extern uint8_t GuiGameover[];

static uint8_t _guitim;
static uint8_t _guiquit;

static uint64_t _g_nums[3];
static const uint8_t G_NumsShift = 62;
static const uint8_t G_Nums[10][5] = {
    {0x7,0x5,0x5,0x5,0x7}, //0
    {0x3,0x2,0x2,0x2,0x7}, //1
    {0x7,0x4,0x7,0x1,0x7}, //2
    {0x7,0x4,0x7,0x4,0x7}, //3
    {0x5,0x5,0x7,0x4,0x4}, //4
    {0x7,0x1,0x7,0x4,0x7}, //5
    {0x7,0x1,0x7,0x5,0x7}, //6
    {0x7,0x4,0x4,0x4,0x4}, //7
    {0x7,0x5,0x7,0x5,0x7}, //8
    {0x7,0x5,0x7,0x4,0x7}, //9
};

static void _UpdateNums(uint32_t num);

/// @brief 显示开始画面，并返回最终选中的游戏难度
/// @return 游戏难度，即每帧持续的时间
uint8_t GUI_Start(void)
{
    _guitim = 0;
    _guiquit = 0;
    uint8_t result = 0;
    Input_Pop(); // 重置输入
    OLED_DRAW_FULLSCREEN(GuiStart);
    while (1)
    {
        while (!_guitim) { }

        // todo 难度 0-15
        if (_guiquit) return result;
        if (Input_Pop() == Input_Type_Button) _guiquit = 1;
        result = ADCMod_GetKnob() * 16;
        //result++;
        _UpdateNums(result);
        
        _guitim = 0;
    }
}

/// @brief 显示游戏结束画面
/// @param score 游戏的最终得分
void GUI_Gameover(uint32_t score)
{
    _guitim = 0;
    _guiquit = 0;
    Input_Pop(); // 重置输入
    OLED_DRAW_FULLSCREEN(GuiGameover);
    _UpdateNums(score);
    while (1)
    {
        while (!_guitim) { }
        
        if (_guiquit) return;
        if (Input_Pop() == Input_Type_Button) _guiquit = 1;

        _guitim = 0;
    }
}

void GUI_TimHandler(void)
{
    _guitim = 1;
}

static void _UpdateNums(uint32_t num)
{
    for (uint8_t i = 0; i < 5; i++)
    {
        _g_nums[i] = 0;
    }

    uint8_t digit = num ? log10(num)+1 : 1;
    uint8_t startpos = 28 + digit*2;

    for (uint8_t i = 0; i < digit; i++)
    {
        uint8_t pos = startpos - 4*i;
        for (uint8_t j = 0; j < 5; j++)
        {
            _g_nums[j] |= ((uint64_t)G_Nums[num%10][j] << pos);
        }
        
        num /= 10;
    }

    OLED_DrawPic((uint8_t*)(void*)_g_nums, 0, 8, G_NumsShift, 5);
}
