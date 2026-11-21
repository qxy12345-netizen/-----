/**
 * @file    glyph3.c
 * @brief   字模查表
 *
 * 没有浮点、没有循环计算 —— 就是从一个 2KB 的 const 数组里取半字节。
 * 为什么这么做：F103C8T6 是 Cortex-M3，没有硬件浮点，实时跑距离场
 * 要一两秒，而字模表只花 2KB Flash、0 字节 RAM、0 微秒启动时间。
 */
#include "glyph3.h"
#include "glyph3_data.h"

uint8_t glyph3_level(int x, int y)
{
    if ((unsigned)x >= GLYPH3_WIDTH || (unsigned)y >= GLYPH3_HEIGHT) {
        return 0;
    }
    /* 每字节打包左右两个像素：低半字节 = 左 */
    return (uint8_t)((k_glyph[y][x >> 1] >> ((x & 1) * 4)) & 0x0F);
}
