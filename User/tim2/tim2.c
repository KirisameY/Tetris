#include "tim2.h"

#include "led/led_extension.h"

#include "stm32f10x.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "misc.h"

/// @brief 初始化Tim2计时器
void TIM2_Init(void)
{
    NVIC_InitTypeDef NVIC_InitInfo;
    TIM_TimeBaseInitTypeDef TIM_InitInfo;
    
    NVIC_InitInfo.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitInfo.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitInfo.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitInfo.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitInfo);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_InitInfo.TIM_Period               = (500-1);             // 中断频率--50ms
    TIM_InitInfo.TIM_Prescaler            = (7200-1);            // 预分频----100us
    TIM_InitInfo.TIM_ClockDivision        = TIM_CKD_DIV1;        // 
    TIM_InitInfo.TIM_CounterMode          = TIM_CounterMode_Up;  // 向上计数
    TIM_InitInfo.TIM_RepetitionCounter    = 0;                   
    TIM_TimeBaseInit(TIM2, &TIM_InitInfo);

    TIM_ClearFlag(TIM2, TIM_FLAG_Update);     // 清除计数器中断标志位
    TIM_ITConfig(TIM2, TIM_IT_Update,ENABLE); // 开启计数器中断

    TIM_Cmd(TIM2, ENABLE); // 使能TIM2
}

int temp = 0;

void TIM2_IntHandler(void)
{
    temp = (temp+1)%20;
    if(temp == 0) Led_Flash_B();
    LedExtension_TimHandler();
}
