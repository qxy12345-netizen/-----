/**
 * @file    glyph3.h
 * @brief   数字 "3" 的字模（预渲染，Flash 里 2KB，不占 SRAM）
 *
 * 字形是 tools/gen_glyph.py 用"贝塞尔笔画 + 距离场"算出来的，
 * 生成结果在 inc/glyph3_data.h。想改字形就改那个脚本里的 SEGS 再重新生成，
 * 不用碰单片机代码。
 */
#ifndef GLYPH3_H_INCLUDED
#define GLYPH3_H_INCLUDED

#include <stdint.h>

#define GLYPH3_WIDTH    64
#define GLYPH3_HEIGHT   64

/**
 * @brief 取字形在某一点的亮度等级
 * @param x 列 0~63
 * @param y 行 0~63
 * @return 0（笔画外）~ 15（笔画内），边缘是中间值 = 抗锯齿
 */
uint8_t glyph3_level(int x, int y);

#endif /* GLYPH3_H_INCLUDED */
