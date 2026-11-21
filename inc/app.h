/**
 * @file    app.h
 * @brief   应用层：把数字 "3" 画到屏幕上
 */
#ifndef APP_H_INCLUDED
#define APP_H_INCLUDED

#include <stdint.h>

/**
 * @brief 画出铺满 64x64 的数字 "3"
 * @param r_top,g_top,b_top   分界线以上的颜色（每通道 0~15）
 * @param r_bot,g_bot,b_bot   分界线以下的颜色
 * @param split_y             分界线所在行 0~64，一般填 32
 */
void app_draw_digit3(uint8_t r_top, uint8_t g_top, uint8_t b_top,
                     uint8_t r_bot, uint8_t g_bot, uint8_t b_bot,
                     int split_y);

#endif /* APP_H_INCLUDED */
