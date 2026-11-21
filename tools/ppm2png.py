# -*- coding: utf-8 -*-
"""把 preview.ppm 放大并转成 preview.png（纯标准库，无第三方依赖）"""
import zlib, struct, sys

def read_ppm(path):
    with open(path, "rb") as f:
        data = f.read()
    # 头: P6\nW H\n255\n
    parts, idx = [], 0
    for _ in range(4):
        while data[idx:idx+1].isspace():
            idx += 1
        if data[idx:idx+1] == b'#':
            while data[idx:idx+1] != b'\n':
                idx += 1
            continue
        s = idx
        while not data[idx:idx+1].isspace():
            idx += 1
        parts.append(data[s:idx])
    idx += 1
    w, h = int(parts[1]), int(parts[2])
    return w, h, data[idx:idx + w*h*3]

def write_png(path, w, h, rgb, scale=8):
    raw = bytearray()
    for y in range(h):
        row = rgb[y*w*3:(y+1)*w*3]
        big = bytearray()
        for x in range(w):
            big += row[x*3:x*3+3] * scale
        for _ in range(scale):
            raw += b'\x00' + big
    def chunk(tag, data):
        c = struct.pack(">I", len(data)) + tag + data
        return c + struct.pack(">I", zlib.crc32(tag + data) & 0xffffffff)
    png = b'\x89PNG\r\n\x1a\n'
    png += chunk(b'IHDR', struct.pack(">IIBBBBB", w*scale, h*scale, 8, 2, 0, 0, 0))
    png += chunk(b'IDAT', zlib.compress(bytes(raw), 9))
    png += chunk(b'IEND', b'')
    with open(path, "wb") as f:
        f.write(png)

w, h, rgb = read_ppm("preview.ppm")
write_png("preview.png", w, h, rgb)
print("preview.png %dx%d (x8)" % (w, h))
