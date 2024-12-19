#ifndef __PAJ7620U2_H
#define __PAJ7620U2_H

// 宏定义

//IIC
#define PAJ7620U2_CALL_WR      0xE6 // 手势识别模块ID-写模式
#define PAJ7620U2_CALL_RD      0xE7 // 手势识别模块ID-读模式
#define PAJ7620U2_REG_BANK_SEL 0xEF // 选择寄存器
#define PAJ7620U2_BANK_0       0x00
#define PAJ7620U2_BANK_1       0x01

#define PAJ7620U2_GET_INT_FLAG1          0X43 //获取手势检测中断标志寄存器1(获取手势结果)
#define PAJ7620U2_GET_INT_FLAG2          0X44 //获取手势检测中断标志寄存器2(获取手势结果)

// 方法定义

void PAJ7620U2_Init(void);
void PAJ7620U2_ClearInt(void);

#endif
