<div align="center">

# LyMini 🕹️📺

**18-in-1 Retro Arcade & Smart Idle TV Firmware for ESP32-C3 1.44" ST7735 (128×128)**

[ 🇬🇧 English ](README.md) | [ 🇹🇭 ภาษาไทย ](README_TH.md)

[![License: Non-Commercial](https://img.shields.io/badge/License-Non--Commercial-red.svg)](LICENSE)
[![Platform: ESP32-C3](https://img.shields.io/badge/Platform-ESP32--C3-yellow.svg)](https://www.espressif.com/en/products/socs/esp32-c3)
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-blue.svg)](https://www.arduino.cc/)
[![Version: v0.3.4](https://img.shields.io/badge/Version-v0.3.4-green.svg)](https://github.com/zotix/LyMini/releases)

</div>

---

## 📖 Overview

**LyMini** is a high-performance, power-optimized **18-in-1 Retro Arcade & Smart Idle TV** firmware designed for the **16 MB ESP32-C3 1.44" ST7735 (128×128)** mini display board.

Built with a custom zero-flicker 16-bit RGB565 framebuffer engine (ST7735S Gamma Correction + 20 kHz PWM + Dirty-Frame Skip), **LyMini** packs **18 classic arcade games** (with customizable visibility & order), an intelligent multi-mode Idle TV screensaver, **12 MB LittleFS** media storage, a Gzip-compressed mobile-first Captive Web Portal (`http://lymini.local`) with live thumbnail/video previews, **GitHub OTA Updates**, and **ESP32-C3 Light Sleep (`<1 mA`)**.

---

## ✨ Key Features

- 🎮 **18 Built-in Retro Arcade Games (Show/Hide & Reorderable via Web Portal)**:
  1. **Chrome Dino** — Endless desert runner with day/night speed scaling.
  2. **Cyber Pong** — Classic neon paddle battle against adaptive AI.
  3. **Space Runner** — Dodge and blast incoming asteroids in deep space.
  4. **Car Dodge** — 3-lane high-speed highway racer.
  5. **Retro Snake** — 2-button relative steering snake in a garden arena.
  6. **Timberman** — Fast-paced tree chopping while dodging branches.
  7. **Cyber Tetris** — Full block-stacking puzzle with line clears.
  8. **Neon Breakout** — Brick-breaking arcade action.
  9. **Cyber Bomber** — Grid-based Bomberman maze with destructible bricks & bombs.
  10. **Flappy Bird** — Tap to flap through pipes.
  11. **2048 Puzzle** — Merge tiles with 3-button axis toggle.
  12. **Space Invader** — Defend Earth from waves of pixel aliens.
  13. **Pac-Mini** — 13×13 maze dot-chomper with roaming ghosts.
  14. **Simon Says** — 3-color memory sequence challenge.
  15. **Whack-A-Mole** — Fast 3-hole mole & bomb reflex game.
  16. **Tower Stack** — Precision block-stacking tower builder.
  17. **Frogger** — Cross busy roads and river logs.
  18. **Minesweeper** — 6×6 logic minefield with flag & dig controls.
- 📺 **8-Mode Smart Idle TV Screensaver**:
  - **🕒 Cyber Digital Clock**, **🕰️ Analog Clock**, **🔢 Retro Flip Clock**, **🌡️ Multi-City Weather**, **🕒+🌡️ Clock + Weather Live Station Combo**, **🖼️ Photo Frame (`.rgb`)**, **🎬 Video / Animation Player (`.mvid`, toggleable in Eco Mode)**, and **☕ Buy Me a Coffee (PromptPay QR)**.
  - **Smart Filtering & Instant Web Preview (`👁️`)**: Preview any Idle screen or city directly on the physical ESP32 display with a single click.
- 🌐 **Fast Mobile-First Web Portal (`http://lymini.local`)**:
  - **Gzip + `ETag` (`304`) Cache + Add to Home Screen**: ~78% smaller payload, instant repeat loads, and standalone full-screen icon support on iOS & Android.
  - **Mobile Bottom Navigation Bar & Auto-Save**: One-handed bottom tabs on mobile and instant auto-saving without manual save buttons.
  - **✂️ 128×128 Media Studio & Live Gallery Preview**: Crop/zoom photos & videos, view live `.rgb`/`.mvid` thumbnails (via `HTTP Range`) and full animated previews right in the browser, stored on **12 MB LittleFS**.
  - **🎮 Game Manager**: Enable/disable games and reorder them on the device Launcher.
- 🚀 **Dual OTA Firmware Update (16 MB Dual-App Partitions)**:
  - **Web OTA**: Check/update from GitHub Releases or upload a `.bin` file in Tab 5.
  - **On-Device OTA (`BOOT` Hold 10s)**: Auto-checks GitHub and displays `Current` vs `GitHub` version before confirming (`A`: Update, **Hold `A` 5s**: Force Update, `B`/`BOOT`: Cancel).
- ⚡ **Multi-Stage Power Saving & Light Sleep**:
  - **Brightness**: Default **75% (Active) / 45% (Idle) / 15% (Eco)** in 5% steps.
  - **CPU Scaling**: **160 MHz** (Active) → **40 MHz** (Idle) → **20 MHz** (Eco) → **Light Sleep `<1 mA`** (Sleep, wakes on any button).
  - **Battery Protection (Unplugged)**: Auto-enters **Eco Mode** below **50%** and **Sleep Mode** below **20%**.
- ℹ️ **Launcher About & Support Screen**:
  - Displays current firmware version (**v0.3.4**) and opens the 112×112 PromptPay QR code on **BOOT (OK)**.

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
