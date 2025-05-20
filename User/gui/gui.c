#include "gui.h"
#include "../adc/adcmod.h"
#include "../input/input.h"
#include "../oled/oled.h"

#include <math.h>
#include <stdio.h>

extern uint8_t GuiStartTitle[];
extern uint8_t GuiStartMiddle[];
extern uint8_t GuiStartTail[];
extern uint8_t GuiGameoverTitle[];
extern uint8_t GuiGameoverMiddle[];
extern uint8_t GuiGameoverTail[];

static uint8_t _guitim;

// static const uint8_t Gui_TitleShift = 4;
// static const uint8_t Gui_TitleHeight = 23;
// static const uint8_t Gui_MiddleShift = 51;
// static const uint8_t Gui_TailShift = 114;
// static const uint8_t Gui_MiddleTailHeight = 7;
// static const uint8_t Gui_NumsShift = 62;
#define GUI_TITLE_SHIFT 4
#define GUI_TITLE_HEIGHT 23
#define GUI_MIDDLE_SHIFT 51
#define GUI_TAIL_SHIFT 114
#define GUI_MIDDLE_TAIL_HEIGHT 7
#define GUI_NUMS_SHIFT 62
static const uint8_t Gui_Nums[10][5] = {
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
    uint8_t result;
    Input_Pop(); // 重置输入
    OLED_Clear();
    OLED_DrawPic(GuiStartTitle, 0, 8, GUI_TITLE_SHIFT, GUI_TITLE_HEIGHT);
    OLED_DrawPic(GuiStartMiddle, 0, 8, GUI_MIDDLE_SHIFT, GUI_MIDDLE_TAIL_HEIGHT);
    OLED_DrawPic(GuiStartTail, 0, 8, GUI_TAIL_SHIFT, GUI_MIDDLE_TAIL_HEIGHT);
    while (1)
    {
        while (!_guitim) { }

        result = ADCMod_GetKnob() / 256;
        if (Input_Pop() == Input_Type_Button) return result;
        
        _UpdateNums(result);
        
        _guitim = 0;
    }
}

/// @brief 显示游戏结束画面
/// @param score 游戏的最终得分
void GUI_Gameover(uint32_t score)
{
    _guitim = 0;
    Input_Pop(); // 重置输入
    OLED_Clear();
    OLED_DrawPic(GuiGameoverTitle, 0, 8, GUI_TITLE_SHIFT, GUI_TITLE_HEIGHT);
    OLED_DrawPic(GuiGameoverMiddle, 0, 8, GUI_MIDDLE_SHIFT, GUI_MIDDLE_TAIL_HEIGHT);
    OLED_DrawPic(GuiGameoverTail, 0, 8, GUI_TAIL_SHIFT, GUI_MIDDLE_TAIL_HEIGHT);
    _UpdateNums(score);
    while (1)
    {
        while (!_guitim) { }
        
        if (Input_Pop() == Input_Type_Button) return;

        _guitim = 0;
    }
}

void GUI_TimHandler(void)
{
    _guitim = 1;
}

static void _UpdateNums(uint32_t num)
{
    uint64_t nums[3];

    for (uint8_t i = 0; i < 5; i++)
    {
        nums[i] = 0;
    }

    // uint8_t digit = num ? log10f(num)+1 : 1;
    uint8_t digit = 0;
    for (uint32_t n = num; n; n /= 10)
    {
        digit++;
    }
    if(digit == 0) digit = 1;
    

    uint8_t startpos = 28 + digit*2;
    uint32_t n = num;

    for (uint8_t i = 0; i < digit; i++)
    {
        uint8_t pos = startpos - 4*i;
        for (uint8_t j = 0; j < 5; j++)
        {
            nums[j] |= ((uint64_t)Gui_Nums[n%10][j] << pos);
        }
        
        n /= 10;
    }

    OLED_DrawPic((uint8_t*)(void*)nums, 0, 8, GUI_NUMS_SHIFT, 5);
}
