#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "misc.h"

#include "led/bsp_gpio_led.h"
#include "tim2/tim2.h"
#include "systick/bsp_SysTick.h"
#include "iic/bsp_iic_debug.h"
#include "oled/oled.h"
#include "input/input.h"
#include "random/random.h"

#include "tetris.h"
#include "main.h"

static void Program_EXTI_Init(void);

int main(void)
{
    Program_EXTI_Init(); // 初始化中断设置
    LED_GPIO_Config();   // 初始化LED引脚
    TIM2_Init();         // 初始化TIM2计时器
    Systick_Init();      // 初始化Systick
    IIC_GPIO_Config();   // 初始化IIC引脚
    OLED_Initialize();   // 初始化OLED屏幕
    Input_Init();        // 初始化输入模块
    Random_Init();       // 初始化随机模块

    for (;;)
    {
        // 主程序
        RGB_ALL_ON
        Delay_1ms(100);
        RGB_ALL_OFF

        Tetris_MainGameLoop(10);
    }
}

/// @brief 锁死程序并打出异常（目前只做了锁死程序和亮个警报灯）
/// @param msg 异常信息(未实装)
void exception(char *msg)
{
    // 回头有必要的话整个串口打印异常信息，没必要就算了
    R_LED_ON_ONLY
    for (;;) ; // 锁死程序
}

static void Program_EXTI_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}
