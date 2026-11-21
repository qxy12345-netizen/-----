# -*- coding: utf-8 -*-
"""
字形生成器：把数字 "3" 的矢量笔画渲染成 64x64 / 4bit 字模表，直接写成 C 头文件。

用法（工程根目录）：
    python tools/gen_glyph.py

输出：
    inc/glyph3_data.h     64 行 x 32 字节的字模（每字节两个像素，低半字节=左）
    tools/preview.ppm     预览图（上红下蓝），再跑 tools/ppm2png.py 转 PNG

为什么是预渲染而不是在单片机里实时算：
    F103C8T6 是 Cortex-M3，没有 FPU，跑 4096 像素 x 71 段贝塞尔的浮点距离场
    要一两秒，而且 20KB SRAM 也放不下中间结果。所以字形在 PC 上算好，
    固件里只查表 —— 2KB Flash 换掉全部浮点运算和启动延时。
"""
import math
import os

W = H = 64

# 6 段三次贝塞尔，描述笔画中心线（设计坐标系 0~64）
SEGS = [
    ((14, 11), (17, 4), (33, 2), (47, 5)),      # 左上 -> 顶
    ((47, 5), (56, 7), (60, 14), (58, 22)),     # 顶 -> 右上
    ((58, 22), (56, 28), (43, 31), (27, 32)),   # 右上 -> 中腰（向左收）
    ((27, 32), (43, 33), (56, 36), (58, 42)),   # 中腰 -> 右下
    ((58, 42), (60, 50), (56, 57), (47, 59)),   # 右下 -> 底
    ((47, 59), (33, 62), (17, 60), (14, 53)),   # 底 -> 左下
]

R = 4.0       # 笔画半径
STEPS = 12    # 每段采样数
AA = 0.9      # 抗锯齿过渡宽度


def sample_path():
    pts = []
    for (p0, c1, c2, p1) in SEGS:
        for i in range(STEPS):
            t = i / STEPS
            mt = 1 - t
            a, b, c, d = mt**3, 3*mt*mt*t, 3*mt*t*t, t**3
            pts.append((a*p0[0] + b*c1[0] + c*c2[0] + d*p1[0],
                        a*p0[1] + b*c1[1] + c*c2[1] + d*p1[1]))
    pts.append(SEGS[-1][3])
    return pts


PTS = sample_path()
xs = [p[0] for p in PTS]
ys = [p[1] for p in PTS]

# 笔画外扩后的包围盒 -> 映射到 0~64（各轴独立缩放，保证铺满整屏）
BX0, BX1 = min(xs) - R, max(xs) + R
BY0, BY1 = min(ys) - R, max(ys) + R


def dist_seg(px, py, a, b):
    ax, ay = a
    bx, by = b
    dx, dy = bx - ax, by - ay
    l2 = dx*dx + dy*dy
    if l2 <= 1e-12:
        return math.hypot(px - ax, py - ay)
    t = ((px - ax)*dx + (py - ay)*dy) / l2
    t = 0.0 if t < 0 else (1.0 if t > 1 else t)
    return math.hypot(px - (ax + t*dx), py - (ay + t*dy))


def level(x, y):
    """返回 0~15 的亮度等级"""
    qx = BX0 + (x + 0.5) / W * (BX1 - BX0)
    qy = BY0 + (y + 0.5) / H * (BY1 - BY0)
    d = min(dist_seg(qx, qy, PTS[i], PTS[i+1]) for i in range(len(PTS)-1))
    c = (R - d) / AA + 0.5
    c = 0.0 if c < 0 else (1.0 if c > 1 else c)
    return int(c * 15.0 + 0.5)


def main():
    lv = [[level(x, y) for x in range(W)] for y in range(H)]

    # ---------- 生成 C 表 ----------
    rows = []
    for y in range(H):
        b = []
        for x in range(0, W, 2):
            b.append(lv[y][x] | (lv[y][x+1] << 4))      # 低半字节 = 左边那个像素
        rows.append(b)

    out = []
    out.append("/**")
    out.append(" * @file    glyph3_data.h")
    out.append(" * @brief   数字 \"3\" 的字模表（自动生成，别手改）")
    out.append(" *")
    out.append(" * 由 tools/gen_glyph.py 生成。数组含义：")
    out.append(" *   k_glyph[行 0~63][列 0~31]，每字节打包两个水平相邻像素，")
    out.append(" *   低半字节 = 左边像素，高半字节 = 右边像素，取值 0~15 = 亮度等级。")
    out.append(" *   取第 x 列: (k_glyph[y][x >> 1] >> ((x & 1) * 4)) & 0x0F")
    out.append(" *")
    out.append(" * 字形 4bit 共 64x32 = 2048 字节，全部在 Flash 里，不占 SRAM。")
    out.append(" */")
    out.append("#ifndef GLYPH3_DATA_H_INCLUDED")
    out.append("#define GLYPH3_DATA_H_INCLUDED")
    out.append("")
    out.append("#include <stdint.h>")
    out.append("")
    out.append("static const uint8_t k_glyph[64][32] = {")
    for y, row in enumerate(rows):
        out.append("    { " + ", ".join("0x%02X" % v for v in row) + " },  /* y=%2d */" % y)
    out.append("};")
    out.append("")
    out.append("#endif /* GLYPH3_DATA_H_INCLUDED */")

    os.makedirs("inc", exist_ok=True)
    open("inc/glyph3_data.h", "w", encoding="utf-8", newline="\n").write("\n".join(out) + "\n")

    # ---------- 生成预览图 ----------
    os.makedirs("tools", exist_ok=True)
    with open("tools/preview.ppm", "wb") as f:
        f.write(b"P6\n%d %d\n255\n" % (W, H))
        for y in range(H):
            for x in range(W):
                v = lv[y][x] * 17
                f.write(bytes((v, 0, 0)) if y < H // 2 else bytes((0, 0, v)))

    # ---------- 控制台核对 ----------
    chars = " .:-=+*#%@"
    for y in range(H):
        print("".join(chars[min(9, lv[y][x] * 10 // 16)] for x in range(W)))
    solid = sum(1 for y in range(H) for x in range(W) if lv[y][x] >= 8)
    print("\n笔画像素占比 %.1f%%  总亮度 %d" % (solid * 100.0 / (W*H), sum(map(sum, lv))))
    print("已生成 inc/glyph3_data.h 和 tools/preview.ppm")


main()
