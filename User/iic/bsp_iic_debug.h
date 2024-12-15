 #ifndef __BSP_IIC_SOFTWARE_H
 #define __BSP_IIC_SOFTWARE_H

 #include "stm32f10x.h"

 /* IIC引脚宏定义　
  * IIC_SDA---->PB11
  * IIC_SCL---->PB10
  * 用户可自定义引脚 详见STM32官方数据手册
  */
#define IIC_NUM                  I2C2
#define IIC_GPIO_CLK_ENABLE      RCC_APB2PeriphClockCmd
#define IIC_GPIO_CLK             RCC_APB2Periph_GPIOB
#define IIC_CLK_ENABLE           RCC_APB1PeriphClockCmd
#define IIC_CLK                  RCC_APB1Periph_I2C2
#define IIC_GPIO_PORT            GPIOB
#define IIC_SDA_GPIO_PIN         GPIO_Pin_11
#define IIC_SCL_GPIO_PIN         GPIO_Pin_10

 /* 定义读写SCL和SDA的宏，已增加代码的可移植性和可阅读性 */
 /* 条件编译： 1 选择GPIO的HAL库实现IO读写 */
 #if 0
     #define IIC_SDA_0    GPIO_WriteBit(IIC_GPIO_PORT, IIC_SDA_GPIO_PIN, Bit_RESET)
     #define IIC_SDA_1    GPIO_WriteBit(IIC_GPIO_PORT, IIC_SDA_GPIO_PIN, Bit_SET)
     #define IIC_SCL_0    GPIO_WriteBit(IIC_GPIO_PORT, IIC_SCL_GPIO_PIN, Bit_RESET)
     #define IIC_SCL_1    GPIO_WriteBit(IIC_GPIO_PORT, IIC_SCL_GPIO_PIN, Bit_SET)
    
     #define IIC_SDA_READ GPIO_ReadInputDataBit(IIC_GPIO_PORT, IIC_SDA_GPIO_PIN)

 /* 这个分支选择直接寄存器操作实现IO读写 
  * 注意：如下写法，在IAR最高级别优化时，会被编译器错误优化
  */
 #else
     #define IIC_SDA_0 IIC_GPIO_PORT->BSRR = IIC_SDA_GPIO_PIN << 16u     /* SDA = 0 */
     #define IIC_SDA_1 IIC_GPIO_PORT->BSRR = IIC_SDA_GPIO_PIN            /* SDA = 1 */
     #define IIC_SCL_0 IIC_GPIO_PORT->BSRR = IIC_SCL_GPIO_PIN << 16u     /* SCL = 0 */
     #define IIC_SCL_1 IIC_GPIO_PORT->BSRR = IIC_SCL_GPIO_PIN            /* SCL = 1 */
    
     #define IIC_SDA_READ (IIC_GPIO_PORT->IDR & IIC_SDA_GPIO_PIN) >> 11
    
 #endif

 void IIC_GPIO_Config(void);
 void IIC_Start(void);
 void IIC_Stop(void);
 void IIC_ACK(uint8_t ack);
 uint8_t IIC_Wait_ACK(void);
 void IIC_SendByte(uint8_t byte);
 uint8_t IIC_ReciveByte(void);

 #endif

