#include "input.h"
#include "paj7620u2.h"

#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "misc.h"

Input_Type Input_Current = 0;

static void Button_Init(void);

/// @brief 初始化输入模块
void Input_Init(void)
{
    PAJ7620U2_Init();
    Button_Init();
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


//按钮初始化
static void Button_Init(void)
{
    GPIO_InitTypeDef GPIO_InitInfo;
    NVIC_InitTypeDef NVIC_InitInfo;
    EXTI_InitTypeDef EXTI_InitInfo;

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource6);

    GPIO_InitInfo.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitInfo.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitInfo.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitInfo);

    EXTI_InitInfo.EXTI_Line = EXTI_Line6;
    EXTI_InitInfo.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitInfo.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitInfo.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitInfo);

    NVIC_InitInfo.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitInfo.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitInfo.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitInfo.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitInfo);
}

void Button_HandleInt(void)
{
    Input_Set(Input_Type_Button);
}
