Import("env")
import gzip
import hashlib
import os
import re
import struct
import zlib

PROJECT_DIR = env.subst("$PROJECT_DIR")
HTML_HEADER = os.path.join(PROJECT_DIR, "src", "WebPortalHtml.h")
CONFIG_HEADER = os.path.join(PROJECT_DIR, "src", "Config.h")
OUT_HEADER = os.path.join(PROJECT_DIR, "src", "WebPortalAssets.h")


def _extract_app_version():
    if not os.path.exists(CONFIG_HEADER):
        return "0.0.0"
    with open(CONFIG_HEADER, "r", encoding="utf-8") as f:
        m = re.search(r'#define\s+APP_VERSION\s+"([^"]+)"', f.read())
    return m.group(1) if m else "0.0.0"


def _extract_html():
    with open(HTML_HEADER, "r", encoding="utf-8") as f:
        content = f.read()
    m = re.search(r'R"rawliteral\((.*?)\)rawliteral"', content, re.DOTALL)
    if not m:
        raise RuntimeError("Could not find PAGE_INDEX_HTML rawliteral in WebPortalHtml.h")
    return m.group(1).strip().encode("utf-8")


def _make_png_192():
    """Generate a crisp 192x192 RGBA PNG app icon without external dependencies."""
    w = h = 192
    # Colors (RGBA)
    bg_top = (19, 28, 46, 255)
    bg_bot = (9, 13, 22, 255)
    cyan = (56, 189, 248, 255)
    light_cyan = (125, 211, 252, 255)
    screen_bg = (24, 34, 54, 255)
    gold = (250, 204, 21, 255)
    green = (16, 185, 129, 255)

    def in_round_rect(x, y, x0, y0, x1, y1, r):
        if x < x0 or x > x1 or y < y0 or y > y1:
            return False
        cx = max(x0 + r, min(x, x1 - r))
        cy = max(y0 + r, min(y, y1 - r))
        return (x - cx) ** 2 + (y - cy) ** 2 <= r * r

    rows = []
    for y in range(h):
        t = y / (h - 1)
        row = bytearray([0])  # filter type 0
        bg = (
            int(bg_top[0] * (1 - t) + bg_bot[0] * t),
            int(bg_top[1] * (1 - t) + bg_bot[1] * t),
            int(bg_top[2] * (1 - t) + bg_bot[2] * t),
            255,
        )
        for x in range(w):
            px = bg
            # Outer TV frame (28..164, 44..152, r=18)
            in_tv_outer = in_round_rect(x, y, 26, 46, 166, 154, 18)
            in_tv_inner = in_round_rect(x, y, 36, 56, 156, 144, 12)
            if in_tv_outer and not in_tv_inner:
                px = cyan
            elif in_tv_inner:
                # Left inner screen (46..126, 66..134)
                if in_round_rect(x, y, 46, 66, 126, 134, 8):
                    px = screen_bg
                    if not in_round_rect(x, y, 50, 70, 122, 130, 6):
                        px = light_cyan
                    # Eyes
                    if (x - 71) ** 2 + (y - 94) ** 2 <= 7 ** 2:
                        px = cyan
                    elif (x - 101) ** 2 + (y - 94) ** 2 <= 7 ** 2:
                        px = cyan
                    # Smile arc
                    elif 74 <= x <= 98 and abs(y - (112 + 0.045 * ((x - 86) ** 2 - 144) * -1)) <= 2.4:
                        px = gold
                # Side buttons
                elif (x - 142) ** 2 + (y - 84) ** 2 <= 7 ** 2:
                    px = gold
                elif (x - 142) ** 2 + (y - 114) ** 2 <= 7 ** 2:
                    px = green
            # Antenna V-shape
            elif 22 <= y <= 46:
                if abs((x - 96) + (46 - y) * 0.9) <= 4.2 or abs((x - 96) - (46 - y) * 0.9) <= 4.2:
                    px = cyan
            row.extend(px)
        rows.append(bytes(row))

    raw = b"".join(rows)
    compressed = zlib.compress(raw, level=9)

    def chunk(tag, data):
        return (
            struct.pack(">I", len(data))
            + tag
            + data
            + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)
        )

    ihdr = struct.pack(">IIBBBBB", w, h, 8, 6, 0, 0, 0)
    return b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", ihdr) + chunk(b"IDAT", compressed) + chunk(b"IEND", b"")


ICON_SVG = (
    '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64">'
    '<rect width="64" height="64" rx="14" fill="#0b0f17"/>'
    '<path d="M24 10 L32 18 L40 10" stroke="#38bdf8" stroke-width="3.5" stroke-linecap="round" stroke-linejoin="round" fill="none"/>'
    '<rect x="9" y="17" width="46" height="34" rx="7" fill="#131c2e" stroke="#38bdf8" stroke-width="3"/>'
    '<rect x="15" y="23" width="26" height="22" rx="3" fill="#1e293b" stroke="#7dd3fc" stroke-width="1.5"/>'
    '<circle cx="23" cy="32" r="2.6" fill="#38bdf8"/>'
    '<circle cx="33" cy="32" r="2.6" fill="#38bdf8"/>'
    '<path d="M24 38 Q28 41.5 32 38" stroke="#facc15" stroke-width="2.2" stroke-linecap="round" fill="none"/>'
    '<circle cx="48" cy="29" r="2.5" fill="#facc15"/>'
    '<circle cx="48" cy="39" r="2.5" fill="#10b981"/>'
    '</svg>'
).encode("utf-8")


def _format_c_array(name, data):
    lines = [f"const uint8_t {name}[] PROGMEM = {{"]
    for i in range(0, len(data), 16):
        chunk = ", ".join(f"0x{b:02X}" for b in data[i : i + 16])
        lines.append(f"  {chunk},")
    lines.append("};")
    return "\n".join(lines)


def generate_assets():
    ver = _extract_app_version()
    html_bytes = _extract_html()
    html_gz = gzip.compress(html_bytes, compresslevel=9, mtime=0)
    svg_gz = gzip.compress(ICON_SVG, compresslevel=9, mtime=0)
    png_bytes = _make_png_192()
    etag = f'"lymini-{ver}-{hashlib.sha1(html_gz).hexdigest()[:8]}"'

    header_text = "\n".join(
        [
            "#pragma once",
            "#include <Arduino.h>",
            "",
            f"// Auto-generated by scripts/gzip_web.py (raw={len(html_bytes)} B, gzip={len(html_gz)} B)",
            f'#define PAGE_INDEX_HTML_ETAG {etag}',
            "",
            _format_c_array("PAGE_INDEX_HTML_GZ", html_gz),
            "",
            _format_c_array("ICON_SVG_GZ", svg_gz),
            "",
            _format_c_array("ICON_192_PNG", png_bytes),
            "",
        ]
    )

    if os.path.exists(OUT_HEADER):
        with open(OUT_HEADER, "r", encoding="utf-8") as f:
            if f.read() == header_text:
                return

    with open(OUT_HEADER, "w", encoding="utf-8") as f:
        f.write(header_text)
    print(f"[gzip_web] Generated WebPortalAssets.h: HTML {len(html_bytes)} B -> {len(html_gz)} B ({100 - len(html_gz)*100//len(html_bytes)}% smaller)")


generate_assets()
