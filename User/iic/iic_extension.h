#ifndef __IIC_EXTENSION_H
#define __IIC_EXTENSION_H

// 扩展定义

#include "bsp_iic_debug.h"

// 更方便地发送一个字节并等待确认，赞美宏定义
#define IIC_SEND_N_WAIT(byte)   IIC_SendByte((byte)); \
                                while(IIC_Wait_ACK());

#endif
