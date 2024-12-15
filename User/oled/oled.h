#ifndef _OLED
#define _OLED

//宏定义
#define OLED_CALL_WR        0x78 //写模式呼叫设备
#define OLED_WR_CMD_CO      0x00 //控制字节：连续写指令
#define OLED_WR_DATA_CO     0x40 //         连续写数据
#define OLED_WR_CMD_ONCE    0x80 //         单次写指令
#define OLED_WR_DATA_ONCE   0xC0 //         单次写数据

//方法定义
void OLED_Initialize(void);
void OLED_DrawTestGui(void);

#endif
