/**
 * @file    app.c
 * @brief   把字形表刷到屏幕上，并按行分成两种颜色
 *
 * 逐像素做三件事：
 *   1. 从字模表查这一点的亮度等级 lv（0~15）
 *   2. 按 lv 给这一行该用的颜色打折扣（lv=15 满亮，lv=3 就是 3/15 亮度）
 *   3. 写进 HUB75 的位平面
 * lv 落在 1~14 的那些像素就是笔画边缘，看着是平滑的过渡而不是硬台阶。
 */
#include "app.h"
#include "glyph3.h"
#include "hub75.h"

void app_draw_digit3(uint8_t r_top, uint8_t g_top, uint8_t b_top,
                     uint8_t r_bot, uint8_t g_bot, uint8_t b_bot,
                     int split_y)
{
    hub75_clear();

    for (int y = 0; y < GLYPH3_HEIGHT; y++) {
        uint8_t r = (y < split_y) ? r_top : r_bot;
        uint8_t g = (y < split_y) ? g_top : g_bot;
        uint8_t b = (y < split_y) ? b_top : b_bot;

        for (int x = 0; x < GLYPH3_WIDTH; x++) {
            uint8_t lv = glyph3_level(x, y);

            if (lv > 0) {
                /* 整数缩放，不用浮点：lv 0~15 直接映射到 0~15 的通道值 */
                hub75_set_pixel(x, y,
                                (uint8_t)((r * lv) / 15),
                                (uint8_t)((g * lv) / 15),
                                (uint8_t)((b * lv) / 15));
            }
        }
    }
}
