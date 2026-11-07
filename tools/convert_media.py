#!/usr/bin/env python3
"""
Convert any image or folder of images to 128x128 16-bit RGB565 binary (.rgb)
for Spotpear ESP32-C3 1.44" ST7735 LittleFS.
"""
import sys
import os

try:
    from PIL import Image, ImageFilter, ImageEnhance
except ImportError:
    print("Please install Pillow: pip install pillow")
    sys.exit(1)

def convert_to_rgb565(input_path, output_path):
    img = Image.open(input_path).convert('RGB')
    # Center Crop to square
    w, h = img.size
    min_dim = min(w, h)
    left = (w - min_dim) // 2
    top = (h - min_dim) // 2
    img = img.crop((left, top, left + min_dim, top + min_dim))
    
    # Step-down resize with Lanczos
    img = img.resize((128, 128), Image.Resampling.LANCZOS)
    
    # Mild contrast (+10%) & color vibrancy (+15%) optimized for ST7735 LCD
    enh_c = ImageEnhance.Contrast(img)
    img = enh_c.enhance(1.10)
    enh_s = ImageEnhance.Color(img)
    img = enh_s.enhance(1.15)
    
    # Edge Sharpening
    img = img.filter(ImageFilter.UnsharpMask(radius=1.2, percent=130, threshold=2))
    
    # Floyd-Steinberg error diffusion dithering to RGB565
    pixels = list(img.getdata())
    r_buf = [float(p[0]) for p in pixels]
    g_buf = [float(p[1]) for p in pixels]
    b_buf = [float(p[2]) for p in pixels]
    
    raw_bytes = bytearray(128 * 128 * 2)
    idx = 0
    
    for y in range(128):
        for x in range(128):
            p = y * 128 + x
            cur_r = max(0.0, min(255.0, r_buf[p]))
            cur_g = max(0.0, min(255.0, g_buf[p]))
            cur_b = max(0.0, min(255.0, b_buf[p]))
            
            r5 = max(0, min(31, int(round(cur_r * 31.0 / 255.0))))
            g6 = max(0, min(63, int(round(cur_g * 63.0 / 255.0))))
            b5 = max(0, min(31, int(round(cur_b * 31.0 / 255.0))))
            
            qr = (r5 * 255.0 + 15.0) / 31.0
            qg = (g6 * 255.0 + 31.0) / 63.0
            qb = (b5 * 255.0 + 15.0) / 31.0
            
            err_r = cur_r - qr
            err_g = cur_g - qg
            err_b = cur_b - qb
            
            if x + 1 < 128:
                n1 = p + 1
                r_buf[n1] += err_r * (7.0 / 16.0)
                g_buf[n1] += err_g * (7.0 / 16.0)
                b_buf[n1] += err_b * (7.0 / 16.0)
            if y + 1 < 128:
                if x > 0:
                    n2 = p + 128 - 1
                    r_buf[n2] += err_r * (3.0 / 16.0)
                    g_buf[n2] += err_g * (3.0 / 16.0)
                    b_buf[n2] += err_b * (3.0 / 16.0)
                n3 = p + 128
                r_buf[n3] += err_r * (5.0 / 16.0)
                g_buf[n3] += err_g * (5.0 / 16.0)
                b_buf[n3] += err_b * (5.0 / 16.0)
                if x + 1 < 128:
                    n4 = p + 128 + 1
                    r_buf[n4] += err_r * (1.0 / 16.0)
                    g_buf[n4] += err_g * (1.0 / 16.0)
                    b_buf[n4] += err_b * (1.0 / 16.0)
                    
            c = (r5 << 11) | (g6 << 5) | b5
            raw_bytes[idx]     = (c >> 8) & 0xFF
            raw_bytes[idx + 1] = c & 0xFF
            idx += 2
            
    with open(output_path, 'wb') as f:
        f.write(raw_bytes)
    print(f"Saved (HD Dithered RGB565): {output_path} ({len(raw_bytes)} bytes)")

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python convert_media.py <image_or_folder>")
        sys.exit(1)
    
    target = sys.argv[1]
    if os.path.isfile(target):
        out_name = os.path.splitext(target)[0] + ".rgb"
        convert_to_rgb565(target, out_name)
    elif os.path.isdir(target):
        for fname in os.listdir(target):
            if fname.lower().endswith(('.png', '.jpg', '.jpeg', '.webp', '.bmp')):
                in_p = os.path.join(target, fname)
                out_p = os.path.join(target, os.path.splitext(fname)[0] + ".rgb")
                convert_to_rgb565(in_p, out_p)
