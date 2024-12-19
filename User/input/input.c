#include "input.h"
#include "paj7620u2.h"

#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "misc.h"

static void Button_Init(void);

static uint8_t _btn = 0;

/// @brief 初始化输入模块
void Input_Init(void)
{
    PAJ7620U2_Init();
    Button_Init();
}

/// @brief 取出暂存的一个输入，会清除取出的输入
/// @return 当前需解决的输入
Input_Type Input_Pop(void)
{
    Input_Type result;
    result = PAJ7620U2_GetInput();
    if (!result && _btn)
    {
        result = Input_Type_Button;
        _btn = 0;
    }
    return result;
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
    _btn = 1;
}
