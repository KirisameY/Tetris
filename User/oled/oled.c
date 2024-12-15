#include "oled.h"
#include "bitmaps.h"
#include "../iic/bsp_iic_debug.h"
#include "../iic/iic_extension.h"
#include "../main.h"

#include <stdint.h>

uint8_t scr_cache[128][8]; // 图像缓存
uint8_t scr_dirty[16];     // 脏标记(每个位代表一个列)

/// @brief 初始化OLED
/// @param
void OLED_Initialize(void)
{
    IIC_Start();
    IIC_SEND_N_WAIT(OLED_CALL_WR)
    IIC_SEND_N_WAIT(OLED_WR_CMD_CO)

    IIC_SEND_N_WAIT(0xaf) // 显示开

    IIC_SEND_N_WAIT(0x81) // 对比度：
    IIC_SEND_N_WAIT(0x7f) // 50%

    IIC_SEND_N_WAIT(0x20) // 寻址模式：
    IIC_SEND_N_WAIT(0x01) // 纵向寻址

    IIC_SEND_N_WAIT(0x22)
    IIC_SEND_N_WAIT(0x00)
    IIC_SEND_N_WAIT(0x07) // 重置页地址

    IIC_SEND_N_WAIT(0x21)
    IIC_SEND_N_WAIT(0x00)
    IIC_SEND_N_WAIT(0x7f) // 重置列地址

    IIC_SEND_N_WAIT(0xA0); // 列映射：segX（自右向左

    IIC_SEND_N_WAIT(0xC8); // COM扫描方向：自下而上

    /* ↓这块是底层，没搞懂，直接抄了 */
    /* 设置预充期 */
    IIC_SEND_N_WAIT(0xD9);
    IIC_SEND_N_WAIT(0x22); /* 阶段一2个无效DCLK时钟/阶段二2个无效DCLK时钟 */

    /* 设置VCOMH取消选择电平
     * 00:0.65xVcc
     * 20:0.77xVcc
     * 30:0.83xVcc
     */
    IIC_SEND_N_WAIT(0xDB);
    IIC_SEND_N_WAIT(0x20);

    /* 设置电荷泵 */
    IIC_SEND_N_WAIT(0x8d);
    IIC_SEND_N_WAIT(0x14);

    IIC_SEND_N_WAIT(0xAF);

    // 清屏：
    IIC_Start();
    IIC_SEND_N_WAIT(OLED_CALL_WR);
    IIC_SEND_N_WAIT(OLED_WR_DATA_CO);
    for (int i = 128 * 8; i > 0; i--)
    {
        IIC_SEND_N_WAIT(0x00);
    }

    IIC_Stop();
}

/// @brief 绘制图像
/// @param pic 要绘制的图像序列
/// @param xpos x起始坐标(按分页算，0-7)
/// @param width 宽度(按分页算，1-8)
/// @param ypos y起始坐标(0-127)
/// @param height 高度(1-128)
void OLED_DrawPic(uint8_t *pic, uint8_t xpos, uint8_t width, uint8_t ypos, uint8_t height)
{
    uint8_t xend = xpos + width - 1,
            yend = ypos + height - 1;

    // 检查参数合法性
    if (xend >= 8 || yend >= 128 || width < 1 || height < 1)
    {
        exception("OLED_DrawPic接收到非法参数!");
    }

    IIC_Start();
    IIC_SEND_N_WAIT(OLED_CALL_WR)
    IIC_SEND_N_WAIT(OLED_WR_CMD_CO)

    IIC_SEND_N_WAIT(0x22)
    IIC_SEND_N_WAIT(xpos)
    IIC_SEND_N_WAIT(xend) // 设置页地址

    IIC_SEND_N_WAIT(0x21)
    IIC_SEND_N_WAIT(ypos)
    IIC_SEND_N_WAIT(yend) // 设置列地址

    // IIC_Stop();

    IIC_Start();
    IIC_SEND_N_WAIT(OLED_CALL_WR)

    IIC_SEND_N_WAIT(OLED_WR_DATA_CO)

    uint16_t bytes = width * height;
    for (uint16_t i = 0; i < bytes; i++)
    {
        IIC_SEND_N_WAIT(pic[i])
    }

    IIC_Stop();
}

#if _TEST_GUI_AVAILABLE
/// @brief 绘制测试用UI图像
/// @param
void OLED_DrawTestGui(void)
{
    OLED_DrawPic(TestGui, 0, 8, 0, 128);
}
#endif

/// @brief 绘制UI边框图像
/// @param
void OLED_DrawGuiBorder(void)
{
    OLED_DrawPic(GuiBorder, 0, 8, 0, 128);
}

/// @brief 根据缓存更新屏幕显示
/// @param
void OLED_UpdateScreen(void)
{
    IIC_Start();
    IIC_SEND_N_WAIT(OLED_WR_CMD_CO)
    IIC_SEND_N_WAIT(OLED_CALL_WR)
    IIC_SEND_N_WAIT(0x22)
    IIC_SEND_N_WAIT(0x00)
    IIC_SEND_N_WAIT(0x07) // 重置页地址

    uint8_t con = 0; // 用于标记是否连续读写，节省不必要的寻址指令

    for (uint8_t i = 0; i < 16; i++)
    {
        if (scr_dirty[i] == 0) 
        {
            con = 0;
            continue;
        }

        for (uint8_t j = 0; j < 8; j++)
        {
            if ((scr_dirty[i] & (0x80>>j)) == 0) 
            {
                con = 0;
                continue;
            }

            uint8_t col = i*8+j;

            if (!con)
            {
                IIC_Start();
                IIC_SEND_N_WAIT(OLED_CALL_WR)
                IIC_SEND_N_WAIT(OLED_WR_CMD_CO)
                IIC_SEND_N_WAIT(0x21)
                IIC_SEND_N_WAIT(col)
                IIC_SEND_N_WAIT(0x7f) // 设置行地址

                IIC_Start();
                IIC_SEND_N_WAIT(OLED_CALL_WR)
                IIC_SEND_N_WAIT(OLED_WR_DATA_CO)
            }

            for (uint8_t k = 0; k < 8; k++)
            {
                IIC_SEND_N_WAIT(scr_cache[col][k])
            }
            con = 1;
        }

        scr_dirty[i] = 0; //清除脏标记
    }

    IIC_Stop();
}

void OLED_ForceUpdateScreen(void)
{
    OLED_DrawPic(scr_cache[0], 0, 8, 0, 128);
    // 重置图形脏标记:
    for (uint8_t i = 0; i < 16; i++)
    {
        scr_dirty[i] = 0;
    }
}
