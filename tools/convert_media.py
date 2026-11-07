#!/usr/bin/env python3
"""
Convert any image or folder of images to 128x128 16-bit RGB565 binary (.rgb)
for Spotpear ESP32-C3 1.44" ST7735 LittleFS.
"""
import sys
import os

try:
    from PIL import Image
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
    # Resize to exact 128x128
    img = img.resize((128, 128), Image.Resampling.LANCZOS)
    
    # Save as raw RGB565 bytes (Big-Endian)
    raw_bytes = bytearray()
    for y in range(128):
        for x in range(128):
            r, g, b = img.getpixel((x, y))
            rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
            raw_bytes.append((rgb565 >> 8) & 0xFF)
            raw_bytes.append(rgb565 & 0xFF)
            
    with open(output_path, 'wb') as f:
        f.write(raw_bytes)
    print(f"Saved: {output_path} ({len(raw_bytes)} bytes)")

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
