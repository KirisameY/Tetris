#include "paj7620u2.h"
#include "input.h"
#include "../iic/bsp_iic_debug.h"
#include "../iic/iic_extension.h"
#include "../systick/bsp_systick.h"
#include "../main.h"

#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "misc.h"

#pragma region Cmds

#define INIT_CMD_SIZE sizeof(InitCmd)/2
//上电初始化数组
const static uint8_t InitCmd[][2] = {
    {0xEF,0x00},
	{0x37,0x07},
    {0x38,0x17},
	{0x39,0x06},
	{0x41,0x00},
	{0x42,0x00},
	{0x46,0x2D},
	{0x47,0x0F},
	{0x48,0x3C},
	{0x49,0x00},
	{0x4A,0x1E},
	{0x4C,0x20},
	{0x51,0x10},
	{0x5E,0x10},
	{0x60,0x27},
	{0x80,0x42},
	{0x81,0x44},
	{0x82,0x04},
	{0x8B,0x01},
	{0x90,0x06},
	{0x95,0x0A},
	{0x96,0x0C},
	{0x97,0x05},
	{0x9A,0x14},
	{0x9C,0x3F},
	{0xA5,0x19},
	{0xCC,0x19},
	{0xCD,0x0B},
	{0xCE,0x13},
	{0xCF,0x64},
	{0xD0,0x21},
	{0xEF,0x01},
	{0x02,0x0F},
	{0x03,0x10},
	{0x04,0x02},
	{0x25,0x01},
	{0x27,0x39},
	{0x28,0x7F},
	{0x29,0x08},
	{0x3E,0xFF},
	{0x5E,0x3D},
	{0x65,0x96},
	{0x67,0x97},
	{0x69,0xCD},
	{0x6A,0x01},
	{0x6D,0x2C},
	{0x6E,0x01},
	{0x72,0x01},
	{0x73,0x35},
	{0x74,0x00},
	{0x77,0x01},
};

#define GESTURE_INIT_CMD_SIZE (sizeof(GestureInitCmd)/2)
//手势识别初始化数组
const static uint8_t GestureInitCmd[][2]={
	{0xEF,0x00},
	{0x41,0x00},
	{0x42,0x00},
	{0xEF,0x00},
	{0x48,0x3C},
	{0x49,0x00},
	{0x51,0x10},
	{0x83,0x20},
	{0x9F,0xF9},
	{0xEF,0x01},
	{0x01,0x1E},
	{0x02,0x0F},
	{0x03,0x10},
	{0x04,0x02},
	{0x41,0x40},
	{0x43,0x30},
	{0x65,0x96},
	{0x66,0x00},
	{0x67,0x97},
	{0x68,0x01},
	{0x69,0xCD},
	{0x6A,0x01},
	{0x6B,0xB0},
	{0x6C,0x04},
	{0x6D,0x2C},
	{0x6E,0x01},
	{0x74,0x00},
	{0xEF,0x00},
	{0x41,0xFF},
	{0x42,0x01},
};

#pragma endregion

#pragma region Results

#define BIT(x) 1<<(x)

#define GES_UP              BIT(0) //向上
#define GES_DOWN            BIT(1) //向下
#define GES_LEFT            BIT(2) //向左
#define GES_RIGHT           BIT(3) //向右
#define GES_FORWARD         BIT(4) //向前
#define GES_BACKWARD        BIT(5) //向后
#define GES_CLOCKWISE       BIT(6) //顺时针
#define GES_COUNT_CLOCKWISE BIT(7) //逆时针
#define GES_WAVE            BIT(8) //挥动

#pragma endregion

static void PAJ7620U2_IntInit(void);
static uint8_t PAJ7620U2_Wakeup(void);
static void PAJ7620U2_I2CInit(void);
static void PAJ7620U2_GestureInit(void);

uint8_t PAJ7620U2_HasInput = 0;

/// @brief 初始化手势识别模块
void PAJ7620U2_Init(void)
{
    PAJ7620U2_IntInit();
    if(!PAJ7620U2_Wakeup()) exception("手势识别模块唤醒失败");
    Delay_1ms(5);
    PAJ7620U2_I2CInit();
    Delay_1ms(5);
    PAJ7620U2_GestureInit();
    //EXTI_GenerateSWInterrupt(EXTI_Line8);
    //EXTI_GenerateSWInterrupt(EXTI_Line0);
    //EXTI->SWIER |= EXTI_Line0;
    //EXTI->SWIER |= EXTI_Line8;
}

void PAJ7620U2_HandleInt(void)
{
    PAJ7620U2_HasInput = 1;
}

Input_Type PAJ7620U2_GetInput(void)
{
    if(!PAJ7620U2_HasInput) return Input_Type_None;

    uint8_t ges[2];

    IIC_Start();
    IIC_SEND_N_WAIT(PAJ7620U2_CALL_WR)
    IIC_SEND_N_WAIT(PAJ7620U2_GET_INT_FLAG1) // 导航到中断位
    
    IIC_Start();
    IIC_SEND_N_WAIT(PAJ7620U2_CALL_RD)
    ges[0] = IIC_ReciveByte();
    IIC_ACK(0);
    ges[1] = IIC_ReciveByte() << 8;
    IIC_ACK(1);

    IIC_Stop();

    PAJ7620U2_HasInput = 0;

    switch ((uint16_t)ges[1]<<8 | ges[0])
    {
        case GES_UP:              return Input_Type_Down;         // 反向映射
        case GES_DOWN:            return Input_Type_Up;           
        case GES_LEFT:            return Input_Type_Right;        
        case GES_RIGHT:           return Input_Type_Left;         
        case GES_CLOCKWISE:       return Input_Type_Clockwise;    
        case GES_COUNT_CLOCKWISE: return Input_Type_AntiClockwise;
        default:                  break;
    }
    
    return Input_Type_None;
}

#pragma region init

static void PAJ7620U2_IntInit(void)
{
    GPIO_InitTypeDef GPIO_InitInfo;
    NVIC_InitTypeDef NVIC_InitInfo;
    EXTI_InitTypeDef EXTI_InitInfo;

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource7);

    GPIO_InitInfo.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitInfo.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitInfo.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitInfo);

    EXTI_InitInfo.EXTI_Line = EXTI_Line7;
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

static uint8_t PAJ7620U2_Wakeup(void)
{
    IIC_Start();
    IIC_SendByte(PAJ7620U2_CALL_WR);
    // IIC_Stop();
    Delay_1ms(5);
    IIC_Start();
    IIC_SendByte(PAJ7620U2_CALL_WR); //唤醒PAJ7620U2
    // IIC_Stop();
    Delay_1ms(5);

    IIC_Start();
    IIC_SEND_N_WAIT(PAJ7620U2_CALL_WR)
    IIC_SEND_N_WAIT(PAJ7620U2_REG_BANK_SEL)
    IIC_SEND_N_WAIT(PAJ7620U2_BANK_0)

    IIC_Start();
    IIC_SEND_N_WAIT(PAJ7620U2_CALL_WR)
    IIC_SEND_N_WAIT(0x00)
    IIC_Start();
    IIC_SEND_N_WAIT(PAJ7620U2_CALL_RD)
    uint8_t receive = IIC_ReciveByte();
    IIC_ACK(1);
    IIC_Stop();

    return receive;
}

static void PAJ7620U2_I2CInit(void)
{
    IIC_Start();
    IIC_SEND_N_WAIT(PAJ7620U2_CALL_WR)
    IIC_SEND_N_WAIT(PAJ7620U2_REG_BANK_SEL)
    IIC_SEND_N_WAIT(PAJ7620U2_BANK_0)

    // IIC_Start();
    // IIC_SEND_N_WAIT(PAJ7620U2_CALL_WR)

    for (uint8_t i = 0; i < INIT_CMD_SIZE; i++)
    {
        IIC_Start();
        IIC_SEND_N_WAIT(PAJ7620U2_CALL_WR)
        IIC_SEND_N_WAIT(InitCmd[i][0])
        IIC_SEND_N_WAIT(InitCmd[i][1])
    }

    IIC_Start();
    IIC_SEND_N_WAIT(PAJ7620U2_CALL_WR)
    IIC_SEND_N_WAIT(PAJ7620U2_REG_BANK_SEL)
    IIC_SEND_N_WAIT(PAJ7620U2_BANK_0)
    IIC_Stop();
}

static void PAJ7620U2_GestureInit(void)
{
    IIC_Start();
    IIC_SEND_N_WAIT(PAJ7620U2_CALL_WR)
    IIC_SEND_N_WAIT(PAJ7620U2_REG_BANK_SEL)
    IIC_SEND_N_WAIT(PAJ7620U2_BANK_0)

    // IIC_Start();
    // IIC_SEND_N_WAIT(PAJ7620U2_CALL_WR)

    for (uint8_t i = 0; i < GESTURE_INIT_CMD_SIZE; i++)
    {
        IIC_Start();
        IIC_SEND_N_WAIT(PAJ7620U2_CALL_WR)
        IIC_SEND_N_WAIT(GestureInitCmd[i][0])
        IIC_SEND_N_WAIT(GestureInitCmd[i][1])
    }

    IIC_Start();
    IIC_SEND_N_WAIT(PAJ7620U2_CALL_WR)
    IIC_SEND_N_WAIT(PAJ7620U2_REG_BANK_SEL)
    IIC_SEND_N_WAIT(PAJ7620U2_BANK_0)
    IIC_Stop();
}

#pragma endregion
