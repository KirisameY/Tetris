#include "stm32f10x.h"

#include "led/bsp_gpio_led.h"
#include "iic/bsp_iic_debug.h"
#include "oled/oled.h"

#include "tetris.h"

int main(void)
{
    LED_GPIO_Config(); //初始化LED引脚
    IIC_GPIO_Config(); //初始化IIC引脚
    OLED_Initialize(); //初始化OLED屏幕


    for (;;)
    {
        // 主程序循环
        RGB_ALL_ON
        int i = 0;
        while (i < 0x888888) i++;
        RGB_ALL_OFF
        OLED_DrawTestGui();

        Tetris_MainGameLoop();
    }
}
