#include "adcmod.h"

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_adc.h"

void ADCMod_Init(void)
{
    GPIO_InitTypeDef GPIO_InitInfo;
    ADC_InitTypeDef ADC_InitInfo;

    // 使能时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2 | RCC_APB2Periph_GPIOA, ENABLE);

    // 初始化A0、A11为模拟输入模式
    GPIO_InitInfo.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_5;
    GPIO_InitInfo.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitInfo.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitInfo);

    // 设置ADC模式
    ADC_InitInfo.ADC_Mode = ADC_Mode_Independent;                  // 独立模式
    ADC_InitInfo.ADC_ScanConvMode = DISABLE;                       // 禁用扫描
    ADC_InitInfo.ADC_ContinuousConvMode = DISABLE;                 // 禁用连续转换
    ADC_InitInfo.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // 软件触发
    ADC_InitInfo.ADC_DataAlign = ADC_DataAlign_Right;              // 右对齐（低12位）
    ADC_InitInfo.ADC_NbrOfChannel = 1;                             // 单通道

    // 初始化ADC1，绑定到PA0（浮空引脚）
    ADC_Init(ADC1, &ADC_InitInfo);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);
    ADC_Cmd(ADC1, ENABLE);

    // 初始化ADC2，绑定到PA5（电位器）
    ADC_Init(ADC2, &ADC_InitInfo);
    ADC_RegularChannelConfig(ADC2, ADC_Channel_5, 1, ADC_SampleTime_71Cycles5);
    ADC_Cmd(ADC2, ENABLE);

    // 启用 ADC1 和 ADC2 校准
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1)) ;
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1)) ;

    ADC_ResetCalibration(ADC2);
    while (ADC_GetResetCalibrationStatus(ADC2)) ;
    ADC_StartCalibration(ADC2);
    while (ADC_GetCalibrationStatus(ADC2)) ;

    // 软件触发
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    ADC_SoftwareStartConvCmd(ADC2, ENABLE);
}

uint16_t ADCMod_GetKnob(void)
{
    while (!ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC)) ;
    uint16_t adc = ADC_GetConversionValue(ADC2);
    ADC_ClearFlag(ADC2, ADC_FLAG_EOC);
    ADC_SoftwareStartConvCmd(ADC2, ENABLE);
    return adc;
}

uint32_t ADCMod_GetRandom(void)
{
    uint32_t result = 0;

    for (uint8_t i = 0; i < 32; i++)
    {
        while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)) ;
        uint16_t adc = ADC_GetConversionValue(ADC1);
        ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
        ADC_SoftwareStartConvCmd(ADC1, ENABLE);
        result |= (adc & 0x01);
        result <<= 1;
    }

    return result;
}
