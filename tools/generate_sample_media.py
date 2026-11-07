#!/usr/bin/env python3
"""
Generate sample 128x128 .rgb photos, .mvid videos, and .bmp previews
into G:/ws/ESP32-C3/MiniGames/sample_media/
Zero external dependencies required (uses Python standard library).
"""
import os
import math
import struct

BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_DIR = os.path.join(BASE_DIR, "sample_media")
os.makedirs(OUT_DIR, exist_ok=True)

W, H = 128, 128

def blank():
    return [[(0, 0, 0) for _ in range(W)] for _ in range(H)]

def fill_rect(b, x, y, w, h, col):
    for py in range(max(0, int(y)), min(H, int(y + h))):
        for px in range(max(0, int(x)), min(W, int(x + w))):
            b[py][px] = col

def fill_circle(b, cx, cy, r, col):
    r2 = r * r
    for py in range(max(0, int(cy - r)), min(H, int(cy + r + 1))):
        for px in range(max(0, int(cx - r)), min(W, int(cx + r + 1))):
            if (px - cx) ** 2 + (py - cy) ** 2 <= r2:
                b[py][px] = col

def save_rgb565(path, buf):
    raw = bytearray(W * H * 2)
    idx = 0
    for y in range(H):
        for x in range(W):
            r, g, b = buf[y][x]
            c = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
            raw[idx] = (c >> 8) & 0xFF
            raw[idx + 1] = c & 0xFF
            idx += 2
    with open(path, "wb") as f:
        f.write(raw)

def frame_to_rgb332(buf):
    raw = bytearray(W * H)
    idx = 0
    for y in range(H):
        for x in range(W):
            r, g, b = buf[y][x]
            raw[idx] = (r & 0xE0) | ((g & 0xE0) >> 3) | (b >> 6)
            idx += 1
    return raw

def save_bmp(path, buf, scale=2):
    uw, uh = W * scale, H * scale
    row_bytes = (uw * 3 + 3) & (~3)
    img_size = row_bytes * uh
    header = bytearray(54)
    header[0:2] = b'BM'
    struct.pack_into('<I', header, 2, 54 + img_size)
    struct.pack_into('<I', header, 10, 54)
    struct.pack_into('<I', header, 14, 40)
    struct.pack_into('<i', header, 18, uw)
    struct.pack_into('<i', header, 22, uh)
    struct.pack_into('<H', header, 26, 1)
    struct.pack_into('<H', header, 28, 24)
    struct.pack_into('<I', header, 34, img_size)
    pix = bytearray(img_size)
    for uy in range(uh):
        sy = H - 1 - (uy // scale)
        ro = uy * row_bytes
        for ux in range(uw):
            sx = ux // scale
            r, g, b = buf[sy][sx]
            i = ro + ux * 3
            pix[i], pix[i+1], pix[i+2] = b, g, r
    with open(path, "wb") as f:
        f.write(header)
        f.write(pix)

def make_cyber_city():
    b = blank()
    for y in range(84):
        t = y / 84.0
        col = (int(12 + 80 * t), int(8 + 16 * t), int(36 + 60 * t))
        for x in range(W): b[y][x] = col
    fill_circle(b, 96, 26, 14, (255, 220, 90))
    blds = [(2,14,42),(18,15,54),(34,16,38),(52,18,62),(72,16,46),(90,16,52),(108,18,36)]
    for i, (bx, bw, bh) in enumerate(blds):
        ty = 84 - bh
        fill_rect(b, bx, ty, bw, bh, (16, 18, 38))
        for wy in range(ty + 4, 80, 6):
            for wx in range(bx + 3, bx + bw - 3, 4):
                if (wx + wy + i) % 3 != 0:
                    fill_rect(b, wx, wy, 2, 3, (0, 255, 240) if (wy // 6) % 2 == 0 else (255, 230, 60))
    fill_rect(b, 0, 84, 128, 44, (14, 10, 32))
    for y in range(86, 126, 3):
        fill_rect(b, 84, y, 24, 1, (255, 60, 180) if y % 6 == 0 else (255, 210, 60))
    return b

def make_astronaut(angle=0.0):
    b = blank()
    for y in range(H):
        t = y / 128.0
        for x in range(W): b[y][x] = (int(6 + 16 * t), int(8 + 4 * t), int(22 + 24 * t))
    fill_circle(b, 98, 28, 14, (255, 190, 60))
    fill_circle(b, 98, 28, 10, (240, 110, 70))
    cx = 60 + int(math.sin(angle) * 8)
    cy = 66 + int(math.cos(angle * 2) * 5)
    fill_rect(b, cx - 16, cy - 10, 32, 26, (180, 190, 205))
    fill_rect(b, cx - 12, cy - 6, 24, 24, (245, 248, 255))
    fill_circle(b, cx, cy - 18, 14, (245, 248, 255))
    vs = int(math.sin(angle) * 4)
    fill_rect(b, cx - 9 + vs, cy - 23, 18, 11, (255, 200, 40))
    return b

def make_synthwave(phase=0.0):
    b = blank()
    for y in range(72):
        t = y / 72.0
        for x in range(W): b[y][x] = (int(24 + 160 * t), int(6 + 24 * t), int(52 + 60 * t))
    fill_circle(b, 64, 44, 22, (255, 210, 50))
    for y in range(46, 66, 4):
        fill_rect(b, 40, y, 48, 2, (120, 20, 90))
    fill_rect(b, 0, 72, 128, 56, (12, 6, 26))
    for i in range(6):
        p = ((i + phase) % 6.0) / 6.0
        gy = 72 + int(p * p * 55)
        if 72 <= gy < 128:
            fill_rect(b, 0, gy, 128, 1, (0, 255, 240))
    return b

def make_pixel_cat():
    b = blank()
    for y in range(H):
        for x in range(W): b[y][x] = (25, 30, 65)
    fill_rect(b, 24, 92, 80, 22, (240, 70, 150))
    fill_rect(b, 34, 46, 60, 46, (255, 180, 50))
    fill_rect(b, 42, 74, 44, 24, (255, 255, 255))
    fill_circle(b, 48, 64, 5, (10, 10, 10))
    fill_circle(b, 80, 64, 5, (10, 10, 10))
    fill_rect(b, 61, 68, 6, 4, (255, 90, 160))
    return b

if __name__ == "__main__":
    photos = [
        ("01_cyber_city", make_cyber_city()),
        ("02_astronaut",  make_astronaut(0.3)),
        ("03_synthwave",  make_synthwave(0.2)),
        ("04_pixel_cat",  make_pixel_cat()),
    ]
    for name, buf in photos:
        save_rgb565(os.path.join(OUT_DIR, name + ".rgb"), buf)
        save_bmp(os.path.join(OUT_DIR, name + ".bmp"), buf)

    # Generate 2 multi-frame .mvid videos (12 frames, 12 FPS, RGB332)
    frames = 12
    hdr = bytes([ord('M'), ord('V'), ord('1'), 1, 128, 128, 12, frames])

    with open(os.path.join(OUT_DIR, "01_astronaut_spin.mvid"), "wb") as f:
        f.write(hdr)
        for i in range(frames):
            f.write(frame_to_rgb332(make_astronaut(i * 2 * math.pi / frames)))

    with open(os.path.join(OUT_DIR, "02_synthwave_drive.mvid"), "wb") as f:
        f.write(hdr)
        for i in range(frames):
            f.write(frame_to_rgb332(make_synthwave(i / float(frames))))

    print("Generated sample photos (.rgb/.bmp) and sample videos (.mvid) in:", OUT_DIR)
