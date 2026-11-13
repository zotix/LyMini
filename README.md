<div align="center">

# LyMini 🕹️📺

**9-in-1 Retro Arcade & Smart Idle TV Firmware for ESP32-C3 1.44" ST7735 (128×128)**

[ 🇬🇧 English ](README.md) | [ 🇹🇭 ภาษาไทย ](README_TH.md)

[![License: Non-Commercial](https://img.shields.io/badge/License-Non--Commercial-red.svg)](LICENSE)
[![Platform: ESP32-C3](https://img.shields.io/badge/Platform-ESP32--C3-yellow.svg)](https://www.espressif.com/en/products/socs/esp32-c3)
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-blue.svg)](https://www.arduino.cc/)
[![Version: v0.2.6](https://img.shields.io/badge/Version-v0.2.6-green.svg)](https://github.com/zotix/LyMini/releases)

</div>

---

## 📖 Overview

**LyMini** is a high-performance, power-optimized **9-in-1 Retro Arcade & Smart Idle TV** firmware designed for the **ESP32-C3 1.44" ST7735 (128×128)** mini display board.

Built with a custom zero-flicker 16-bit RGB565 framebuffer engine running over 40 MHz hardware SPI, **LyMini** packs 9 classic arcade games, an intelligent multi-mode Idle TV screensaver, a bilingual (🇹🇭 TH / 🇬🇧 EN) Captive Web Portal (`http://lymini.local`) with real-time 128×128 photo/video cropping & live streaming, multi-location weather station, and dynamic power management.

---

## ✨ Key Features

- 🎮 **9 Built-in Retro Arcade Games (with Visual Cover Art Launcher)**:
  1. **Chrome Dino** — Endless desert runner with day/night speed scaling.
  2. **Cyber Pong** — Classic neon paddle battle against adaptive AI.
  3. **Space Runner** — Dodge and blast incoming asteroids in deep space.
  4. **Car Dodge** — 3-lane high-speed highway racer.
  5. **Retro Snake** — 2-button relative steering snake in a garden arena.
  6. **Timberman** — Fast-paced tree chopping while dodging branches.
  7. **Cyber Tetris** — Full block-stacking puzzle with line clears.
  8. **Neon Breakout** — Brick-breaking arcade action.
  9. **Cyber Bomber** — Grid-based Bomberman maze with destructible bricks, bombs, and roaming enemies.
- 📺 **8-Mode Smart Idle TV Screensaver**:
  - **🕒 Cyber Digital Clock**, **🕰️ Analog Clock**, **🔢 Retro Flip Clock**, **🌡️ Multi-City Weather**, **🕒+🌡️ Clock + Weather Live Station Combo**, **🖼️ Photo Frame (`.rgb`)**, **🎬 Video / Animation Player (`.mvid`)**, and **☕ Buy Me a Coffee (PromptPay QR)**.
  - **Smart Filtering**: Automatically hides clock screens until NTP time is synced, and hides weather screens until valid weather data is fetched.
  - **Instant Web Preview (`👁️`)**: Preview any Idle screen or city directly on the physical ESP32 display with a single click from the Web UI.
- 🌐 **Bilingual (🇹🇭 TH / 🇬🇧 EN) Captive Web Portal (`http://lymini.local`)**:
  - Connect to the open **`LyMini-AP`** Wi-Fi hotspot (no password required) for automatic Captive Portal popup, or access via `http://lymini.local`.
  - **✂️ 128×128 Media Studio**: Supports photos (`.JPG`, `.PNG`, `.WEBP`, `.HEIC`) and iOS/Android videos (`.MOV`, `.MP4`) with touch pan, pinch-to-zoom, live streaming to the ESP32 screen, and LittleFS storage.
  - **🔆 Auto-Update Max Brightness Ceiling**: Real-time slider immediately updates the screen brightness and sets the maximum brightness ceiling.
  - **⚡ Dynamic CPU & Inactivity Timeout Configuration**: Set both Idle (40 MHz / 50% Bri) and Eco (20 MHz / 25% Bri) timeouts directly from the web interface, saved permanently to NVS.
  - **🔄 Smart Screen & Button Rotation**: Default 270° landscape orientation (`0° / 90° / 180° / 270°` selectable) with automatic Left/Right button direction flipping.
  - **🌙 Night Mode Screen-Off Schedule**: Automatically turns off the backlight (`0%`) during scheduled night hours and wakes on any button press.
- ⚡ **Dynamic CPU Scaling & Multi-Stage Power Saving (v0.2.5)**:
  - **Active Gameplay / Menu / Wi-Fi**: Boosts CPU to **160 MHz** with full 60 FPS pacing and user-set brightness.
  - **Idle Display (Smart TV)**: At 30s inactivity (customizable), scales CPU down to **40 MHz** (direct crystal clock with PLL off) and **50% brightness** (24 FPS).
  - **Eco Battery Saver**: At 5 min inactivity (customizable), scales CPU down to **20 MHz** and **25% brightness** (15 FPS) for extreme battery preservation.
  - **Instant Boost**: Touching any button instantly revs the CPU back to **160 MHz** and restores full brightness.
- ℹ️ **Launcher About & Support Screen (11th Menu Card)**:
  - Convenient menu card displaying current firmware version (**v0.2.6**) and platform hardware.
  - Pressing **BOOT (OK)** opens a full-screen, pixel-perfect 112×112 PromptPay QR code for developer donations, with instant one-click return to menu.

---

## 🛠️ Supported Hardware

- **Spotpear ESP32-C3 1.44" ST7735 LCD (128×128)** Mini TV / Arcade Board
  - **Display (ST7735S Greentab 128×128)**: `MOSI=GPIO4`, `SCLK=GPIO3`, `CS=GPIO2`, `DC=GPIO0`, `RST=GPIO5`, `BL=GPIO7`
  - **Controls**: Button A / Left (`GPIO10`), Button B / Right (`GPIO8`), BOOT / OK (`GPIO9`, hold 4s in-game to exit to menu)

### 🛒 Where to Buy Hardware (Shopee)

- 📟 **ESP32-C3 1.44" LCD Mini Arcade / TV Board**:
  - Shopee: https://s.shopee.co.th/40hut3UXdh

---

## 📜 License Terms

Software copyright is owned by **© 2026 Zotix. All Rights Reserved.**

### 1. Personal / Non-Commercial Use — 100% Free
Individuals are free to download, modify, compile, and flash this firmware onto their own hardware for personal, non-commercial, and educational purposes.

### 2. Commercial Use — ⚠️ License Required
**Commercial use, including pre-flashing onto boards for commercial sale (e.g. Shopee, Lazada, TikTok Shop, Amazon, retail stores) or any monetization, is strictly prohibited without written consent.**

> **Interested in commercial distribution?**  
> To obtain commercial licensing rights for distributing pre-flashed boards, **contact the developer (Zotix)** via GitHub Issues:  
> [https://github.com/zotix/LyMini/issues](https://github.com/zotix/LyMini/issues)

See [LICENSE](LICENSE) for full details.

---

## ☕ Support the Developer (Buy Me a Coffee)

If you enjoy **LyMini** and would like to support future development, you can scan the PromptPay QR code below:

<div align="center">
  <img src="docs/images/promptpay_qr.png" width="220" alt="PromptPay QR Code">
  <br>
  <em>Scan with any Thai mobile banking app to support the developer (Zotix) ☕</em>
</div>

- You can also view and lock the PromptPay QR code directly on the **LyMini** screen via Idle mode or the Web Portal (`☕ About & Coffee` tab).
