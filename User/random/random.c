#include "random.h"
#include "../adc/adcmod.h"

#include <stdlib.h>


void Random_Init(void)
{
    uint32_t seed = ADCMod_GetRandom();
    srand(seed);
}

int Random_Next(int from, int to)
{
    int rd = rand();
    srand(rd);
    return rd%(to-from)+from;
}

/// @brief 从from开始，在buffer中生成count个连续的整数，以随机顺序排列
/// @param from     最小值
/// @param count    生成的数量
/// @param buffer   结果填充的位置，长度需要至少为count
/// @param rdbuffer 用于存储中间随机值的缓存，长度需要至少为count
void Random_Shuffle(uint8_t from, uint8_t count, uint8_t* buffer)
{
    uint8_t rdbuf[count];

    for (uint8_t i = 0; i < count; i++) //清空rdbuffer
    {
        rdbuf[i] = 0;
    }

    for (uint8_t i = 0; i < count; i++) //向buffer中填充随机数据
    {
        uint8_t rd = rand();
        srand(rd);
        for (uint8_t j = 0; j <= i; j++)
        {
            if (j==i)
            {
                rdbuf[j] = rd;
                buffer [j] = from+i;
                break;
            }
            if (rdbuf[j] <= rd) continue;
            
            for (int8_t k = i-1; k >=j; k--)
            {
                rdbuf[k+1] = rdbuf[k];
                buffer[k+1] = buffer[k];
            }
            rdbuf[j] = rd;
            buffer [j] = from+i;
            break;
        }
        
    }
}
