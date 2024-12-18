/**
 ******************************************************************************
 * @file    main.c
 * @author  fire
 * @version V1.0
 * @date    2024-xx-xx
 * @brief   systick
 ******************************************************************************
 * @attention
 *
 * 实验平台:野火 F103 STM32 开发板
 * 论坛    :http://www.firebbs.cn
 * 淘宝    :https://fire-stm32.taobao.com
 *
 ******************************************************************************
 */
#include "systick/bsp_systick.h"

static __IO uint32_t TimingDelay;

void Systick_Init(void)
{
    /* SystemFrequency / 1000    1ms
     * SystemFrequency / 100000	 10us
     * SystemFrequency / 1000000 1us
     */
    if (SysTick_Config(SystemCoreClock / 1000))
    {
        /* Capture error */
        while (1)
            ;
    }
}

void Delay_1ms(__IO uint32_t nTime)
{
    TimingDelay = nTime;

    while (TimingDelay != 0)
        ;
}

void TimingDelay_Decrement(void)
{
    if (TimingDelay != 0x00)
    {
        TimingDelay--;
    }
}
