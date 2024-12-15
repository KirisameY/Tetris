/**
  ******************************************************************************
  * @file       bsp_gpio_led.c
  * @author     embedfire
  * @version     V1.0
  * @date        2024
  * @brief      LED灯函数接口
  ******************************************************************************
  * @attention
  *
  * 实验平台  ：野火 STM32F103C8T6-STM32开发板 
  * 论坛      ：http://www.firebbs.cn
  * 官网      ：https://embedfire.com/
  * 淘宝      ：https://yehuosm.tmall.com/
  *
  ******************************************************************************
  */

#include "led/bsp_gpio_led.h"


/**
  * @brief  初始化控制 LED 的IO
  * @param  无
  * @retval 无
  */
void LED_GPIO_Config(void)
{
    /* 定义一个 GPIO 结构体 */
    GPIO_InitTypeDef gpio_initstruct = {0};
    
/**************************** 核心板载LED灯 *****************************/   
#if 1    
    
    /* 开启 LED 相关的GPIO外设/端口时钟 */
    RCC_APB2PeriphClockCmd(R_LED_GPIO_CLK_PORT,ENABLE);
    
    /* IO输出状态初始化控制 */
    GPIO_SetBits(R_LED_GPIO_PORT,R_LED_GPIO_PIN);
    
    /*选择要控制的GPIO引脚、设置GPIO模式为 推挽模式、设置GPIO速率为50MHz*/
    gpio_initstruct.GPIO_Pin    = R_LED_GPIO_PIN;
    gpio_initstruct.GPIO_Mode   = GPIO_Mode_Out_PP;
    gpio_initstruct.GPIO_Speed  = GPIO_Speed_50MHz;
    GPIO_Init(R_LED_GPIO_PORT,&gpio_initstruct);//配置端口引脚的模式
   
#endif 
    
#if 1    
    
    /* 开启 LED 相关的GPIO外设/端口时钟 */
    RCC_APB2PeriphClockCmd(G_LED_GPIO_CLK_PORT,ENABLE);
    
    /* IO输出状态初始化控制 */
    GPIO_SetBits(G_LED_GPIO_PORT,G_LED_GPIO_PIN);
    
    /*选择要控制的GPIO引脚、设置GPIO模式为 推挽模式、设置GPIO速率为50MHz*/
    gpio_initstruct.GPIO_Pin    = G_LED_GPIO_PIN;
    gpio_initstruct.GPIO_Mode   = GPIO_Mode_Out_PP;
    gpio_initstruct.GPIO_Speed  = GPIO_Speed_50MHz;
    GPIO_Init(G_LED_GPIO_PORT,&gpio_initstruct);//配置端口引脚的模式
   
#endif 

#if 1    
    
    /* 开启 LED 相关的GPIO外设/端口时钟 */
    RCC_APB2PeriphClockCmd(B_LED_GPIO_CLK_PORT,ENABLE);
    
    /* IO输出状态初始化控制 */
    GPIO_SetBits(B_LED_GPIO_PORT,B_LED_GPIO_PIN);
    
    /*选择要控制的GPIO引脚、设置GPIO模式为 推挽模式、设置GPIO速率为50MHz*/
    gpio_initstruct.GPIO_Pin    = B_LED_GPIO_PIN;
    gpio_initstruct.GPIO_Mode   = GPIO_Mode_Out_PP;
    gpio_initstruct.GPIO_Speed  = GPIO_Speed_50MHz;
    GPIO_Init(B_LED_GPIO_PORT,&gpio_initstruct);//配置端口引脚的模式
   
#endif 
}

/**
  * @brief  开启对应 LED 灯
  * @param  GPIOx：x 可以是 A，B，C等
  * @param  GPIO_Pin：待操作的pin脚号
  * @retval 无
  */
void LED_ON(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    GPIO_ResetBits(GPIOx,GPIO_Pin);
}

/**
  * @brief  关闭对应 LED 灯
  * @param  GPIOx：x 可以是 A，B，C等
  * @param  GPIO_Pin：待操作的pin脚号
  * @retval 无
  */
void LED_OFF(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    GPIO_SetBits(GPIOx,GPIO_Pin);
}
/*****************************END OF FILE***************************************/
