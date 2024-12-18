#include "stm32f10x.h"

#include "led/bsp_gpio_led.h"
#include "systick/bsp_SysTick.h"
#include "iic/bsp_iic_debug.h"
#include "oled/oled.h"
#include "input/input.h"

#include "tetris.h"
#include "main.h"

int main(void)
{
    LED_GPIO_Config(); // 初始化LED引脚
    Systick_Init();    // 初始化Systick
    IIC_GPIO_Config(); // 初始化IIC引脚
    OLED_Initialize(); // 初始化OLED屏幕
    Input_Init();

    for (;;)
    {
        // 主程序循环
        RGB_ALL_ON
        Delay_1ms(100);
        RGB_ALL_OFF

#if _TEST_GUI_AVAILABLE
        OLED_DrawTestGui();
#endif

        Tetris_MainGameLoop();
    }
}

/// @brief 锁死程序并打出异常
/// @param msg 异常信息(未实装)
void exception(char *msg)
{
    // 回头有必要的话整个串口打印异常信息，没必要就算了
    R_LED_ON_ONLY
    for (;;) ; // 锁死程序
}
