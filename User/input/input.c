#include "input.h"
#include "paj7620u2.h"

Input_Type Input_Current = 0;

/// @brief 初始化输入模块
void Input_Init(void)
{
    PAJ7620U2_Init();
    //todo: 还有按钮
}

/// @brief 设置输入状态
/// @param input 新的输入
void Input_Set(Input_Type input)
{
    Input_Current = input;
}

/// @brief 清除当前输入状态，即将本次输入设为已处理
void Input_Clear(void)
{
    Input_Current = Input_Type_None;
}
