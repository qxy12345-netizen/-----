# -*- coding: utf-8 -*-
"""生成 README 用的接线示意图 docs/wiring.svg"""

ROWS = [
    # (排线脚号, 屏幕丝印, MCU 引脚, 颜色)
    (1,  "R1",      "PE7",  "#e53935"),
    (2,  "G1",      "PE8",  "#2e7d32"),
    (3,  "B1",      "PE9",  "#1e88e5"),
    (4,  "GND",     None,   "#9e9e9e"),
    (5,  "R2",      "PE10", "#e53935"),
    (6,  "G2",      "PE11", "#2e7d32"),
    (7,  "B2",      "PE12", "#1e88e5"),
    (8,  "E",       "PB14", "#7b1fa2"),
    (9,  "A",       "PB10", "#7b1fa2"),
    (10, "B",       "PB11", "#7b1fa2"),
    (11, "C",       "PB12", "#7b1fa2"),
    (12, "D",       "PB13", "#7b1fa2"),
    (13, "CLK",     "PE13", "#546e7a"),
    (14, "LAT/STB", "PE14", "#546e7a"),
    (15, "OE",      "PE15", "#546e7a"),
    (16, "GND",     None,   "#9e9e9e"),
]

LX_START = 156
TOP = 118
STEP = 33
W, H = 760, TOP + STEP * len(ROWS) + 150

out = []
out.append(f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" '
           f'viewBox="0 0 {W} {H}" font-family="Segoe UI, Arial, sans-serif">')
out.append(f'<rect width="{W}" height="{H}" fill="#ffffff"/>')

# 标题
out.append('<text x="30" y="42" font-size="21" font-weight="700" fill="#1f2937">'
           '64×64 HUB75 点阵  →  STM32F407VET6（天猛星）左侧排针 H1 接线图</text>')
out.append('<text x="30" y="66" font-size="13" fill="#6b7280">'
           '排线有防呆凸起，插反插不进去；以屏背面丝印为准，脚号仅供参考</text>')

# 左侧：排线
lx0, lx1 = 30, 300
out.append(f'<rect x="{lx0}" y="{TOP-34}" width="{lx1-lx0}" height="{STEP*len(ROWS)+40}" '
           'rx="10" fill="#f8fafc" stroke="#cbd5e1" stroke-width="1.5"/>')
out.append(f'<text x="{lx0+16}" y="{TOP-14}" font-size="13" font-weight="600" fill="#475569">'
           'HUB75 16P 排线（丝印）</text>')

# 右侧：MCU
rx0, rx1 = 500, 730
out.append(f'<rect x="{rx0}" y="{TOP-34}" width="{rx1-rx0}" height="{STEP*len(ROWS)+40}" '
           'rx="10" fill="#f8fafc" stroke="#cbd5e1" stroke-width="1.5"/>')
out.append(f'<text x="{rx0+16}" y="{TOP-14}" font-size="13" font-weight="600" fill="#475569">'
           'MCU 引脚</text>')

for i, (pin, name, mcu, color) in enumerate(ROWS):
    y = TOP + i * STEP
    cy = y + 10
    dim = mcu is None

    # 脚号
    out.append(f'<text x="{lx0+18}" y="{cy+5}" font-size="13" fill="#94a3b8">{pin}</text>')
    # 丝印名
    fw = "600" if not dim else "400"
    out.append(f'<text x="{lx0+48}" y="{cy+5}" font-size="14" font-weight="{fw}" '
               f'fill="{color if not dim else "#9e9e9e"}">{name}</text>')

    if dim:
        out.append(f'<text x="{rx0+16}" y="{cy+5}" font-size="13" fill="#b0b7c3">'
                   f'{"电源地，必须和板子共地" if pin == 16 else "电源地"}</text>')
        out.append(f'<line x1="{LX_START}" y1="{cy}" x2="{rx0-6}" y2="{cy}" '
                   'stroke="#d1d5db" stroke-width="1" stroke-dasharray="4 5"/>')
        continue

    # 连线
    out.append(f'<line x1="{LX_START}" y1="{cy}" x2="{rx0-6}" y2="{cy}" '
               f'stroke="{color}" stroke-width="2" stroke-linecap="round"/>')
    # 引脚框
    out.append(f'<rect x="{rx0+6}" y="{cy-13}" width="112" height="26" rx="6" '
               f'fill="{color}" fill-opacity="0.10" stroke="{color}" stroke-width="1.4"/>')
    out.append(f'<text x="{rx0+62}" y="{cy+5}" font-size="14" font-weight="600" '
               f'text-anchor="middle" fill="{color}">{mcu}</text>')

# 底部说明
by = TOP + STEP * len(ROWS) + 26
notes = [
    ("#e53935", "R1/R2 = 红色数据；", ),
    ("#1e88e5", "B1/B2 = 蓝色数据"),
]
out.append(f'<text x="30" y="{by}" font-size="13" fill="#374151">'
           '颜色不需要额外的选择脚：HUB75 每个像素自带 R/G/B 三路通道，'
           '红蓝由数据位决定。</text>')
out.append(f'<text x="30" y="{by+24}" font-size="13" fill="#374151">'
           '上半屏（第 0~31 行）走 R1/G1/B1，下半屏（第 32~63 行）走 R2/G2/B2，同时传输。</text>')
out.append(f'<text x="30" y="{by+48}" font-size="13" fill="#b91c1c" font-weight="600">'
           '屏幕 +5V 单独用 5V/3A 电源供电，GND 必须和板子 GND 连在一起。</text>')

out.append('</svg>')

import os
os.makedirs("docs", exist_ok=True)
open("docs/wiring.svg", "w", encoding="utf-8").write("\n".join(out))
print("docs/wiring.svg", W, "x", H)
