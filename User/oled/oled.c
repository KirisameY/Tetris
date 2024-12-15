#include "oled.h"
#include "bitmaps.h"
#include "../iic/bsp_iic_debug.h"
#include "../iic/iic_extension.h"

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
    IIC_SEND_N_WAIT(0x07) //重置页地址
    
    IIC_SEND_N_WAIT(0x21)
    IIC_SEND_N_WAIT(0x00)
    IIC_SEND_N_WAIT(0x7f) //重置列地址

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
    for(int i = 128*8; i > 0; i--)
    {   
        IIC_SEND_N_WAIT(0x00);
    }


    IIC_Stop();
}


/// @brief 绘制测试用UI图像
/// @param  
void OLED_DrawTestGui(void)
{
    IIC_Start();
    IIC_SEND_N_WAIT(OLED_CALL_WR)
    IIC_SEND_N_WAIT(OLED_WR_CMD_CO)

    IIC_SEND_N_WAIT(0x22)
    IIC_SEND_N_WAIT(0x00)
    IIC_SEND_N_WAIT(0x07) //重置页地址
    
    IIC_SEND_N_WAIT(0x21)
    IIC_SEND_N_WAIT(0x00)
    IIC_SEND_N_WAIT(0x7f) //重置列地址

    // IIC_Stop();

    IIC_Start();
    IIC_SEND_N_WAIT(OLED_CALL_WR)

    IIC_SEND_N_WAIT(OLED_WR_DATA_CO)

    for (uint16_t i = 0; i < 128*8; i++)
    {
        IIC_SEND_N_WAIT(TestGui[i])
    }

    IIC_Stop();
}
